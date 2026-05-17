#include "Remesher.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <vector>
#include <dirent.h>
#include <sys/stat.h>
#include <cstring>

#include <vcg/complex/complex.h>
#include <vcg/complex/algorithms/update/topology.h>
#include <vcg/complex/algorithms/update/normal.h>
#include <vcg/complex/algorithms/update/flag.h>
#include <vcg/complex/algorithms/clean.h>
#include <vcg/complex/algorithms/isotropic_remeshing.h>
#include <wrap/io_trimesh/import_obj.h>
#include <wrap/io_trimesh/export_obj.h>

namespace {

class RVertex;
class RFace;
class REdge;

struct RUsedTypes : public vcg::UsedTypes<
    vcg::Use<RVertex>::AsVertexType,
    vcg::Use<REdge>::AsEdgeType,
    vcg::Use<RFace>::AsFaceType> {};

class RVertex : public vcg::Vertex<RUsedTypes,
    vcg::vertex::Coord3f, vcg::vertex::Normal3f,
    vcg::vertex::Qualityf, vcg::vertex::Mark,
    vcg::vertex::VFAdj, vcg::vertex::BitFlags> {};
class REdge : public vcg::Edge<RUsedTypes> {};
class RFace : public vcg::Face<RUsedTypes, vcg::face::VertexRef,
    vcg::face::Normal3f, vcg::face::Qualityf, vcg::face::Mark,
    vcg::face::VFAdj, vcg::face::FFAdj, vcg::face::BitFlags> {};

class RMesh : public vcg::tri::TriMesh<std::vector<RVertex>, std::vector<RFace>> {};

bool ends_with(const std::string& s, const std::string& suffix) {
    return s.size() >= suffix.size() &&
           s.compare(s.size() - suffix.size(), suffix.size(), suffix) == 0;
}

bool ensure_dir(const std::string& path) {
    struct stat st;
    if (stat(path.c_str(), &st) == 0) return S_ISDIR(st.st_mode);
    return mkdir(path.c_str(), 0755) == 0;
}

// Delete every .obj under `dir` (non-recursive, top-level only). Returns count.
// Used to keep layers_remeshed/ free of stale files from prior runs that would
// otherwise pollute the combined output.
int wipe_objs(const std::string& dir) {
    DIR* d = opendir(dir.c_str());
    if (!d) return 0;
    int n = 0;
    struct dirent* e;
    while ((e = readdir(d)) != nullptr) {
        std::string name = e->d_name;
        if (ends_with(name, ".obj") || ends_with(name, ".OBJ")) {
            std::string p = dir + "/" + name;
            if (std::remove(p.c_str()) == 0) ++n;
        }
    }
    closedir(d);
    return n;
}

} // namespace

bool Remesher::remesh_file(const std::string& input_obj,
                           const std::string& output_obj,
                           const RemeshParams& p) {
    RMesh m;

    int mask = 0;
    int load = vcg::tri::io::ImporterOBJ<RMesh>::Open(m, input_obj.c_str(), mask);
    if (load != vcg::tri::io::ImporterOBJ<RMesh>::E_NOERROR &&
        load != vcg::tri::io::ImporterOBJ<RMesh>::E_NON_CRITICAL_ERROR) {
        std::cerr << "[Remesher] failed to load " << input_obj
                  << " (code " << load << ")\n";
        return false;
    }

    vcg::tri::Clean<RMesh>::RemoveDuplicateVertex(m);
    vcg::tri::Clean<RMesh>::RemoveUnreferencedVertex(m);
    vcg::tri::Allocator<RMesh>::CompactEveryVector(m);
    vcg::tri::UpdateTopology<RMesh>::FaceFace(m);
    vcg::tri::UpdateTopology<RMesh>::VertexFace(m);
    vcg::tri::UpdateNormal<RMesh>::PerVertexNormalizedPerFace(m);
    vcg::tri::UpdateFlags<RMesh>::FaceBorderFromFF(m);
    vcg::tri::UpdateFlags<RMesh>::VertexBorderFromFaceBorder(m);

    typename vcg::tri::IsotropicRemeshing<RMesh>::Params params;
    params.SetTargetLen(p.target_edge_len);
    params.SetFeatureAngleDeg(p.feature_angle_deg);
    params.iter         = p.iterations;
    params.adapt        = p.adaptive;
    params.splitFlag    = p.split;
    params.collapseFlag = p.collapse;
    params.swapFlag     = p.swap;
    params.smoothFlag   = p.smooth;
    params.projectFlag  = p.reproject;
    // vcglib defaults surfDistCheck=true with maxSurfDist uninitialized — every
    // op fails the distance test against garbage. The MeshLab .mlx explicitly
    // disables CheckSurfDist; mirror that here.
    params.surfDistCheck = false;

    const size_t v_before = m.VN(), f_before = m.FN();
    vcg::tri::IsotropicRemeshing<RMesh>::Do(m, params);
    std::cerr << "[Remesher] v " << v_before << "->" << m.VN()
              << "  f " << f_before << "->" << m.FN()
              << "  splits=" << params.stat.splitNum
              << "  collapses=" << params.stat.collapseNum
              << "  flips=" << params.stat.flipNum
              << "  bbox_diag=" << m.bbox.Diag()
              << "  target_len=" << p.target_edge_len << "\n";

    int save = vcg::tri::io::ExporterOBJ<RMesh>::Save(m, output_obj.c_str(), vcg::tri::io::Mask::IOM_NONE);
    if (save != 0) {
        std::cerr << "[Remesher] failed to save " << output_obj
                  << " (code " << save << ")\n";
        return false;
    }
    return true;
}

namespace {

// Offset OBJ face/line vertex indices. Token may be `v`, `v/vt`, `v//vn`, or
// `v/vt/vn` (1-indexed, possibly negative). Only the v / vt / vn components
// get offset; we never see negative indices from our writers so don't worry
// about them.
std::string offset_obj_token(const std::string& tok, int dv, int dvt, int dvn) {
    std::ostringstream out;
    std::istringstream iss(tok);
    std::string part;
    int slot = 0;
    bool first = true;
    while (std::getline(iss, part, '/')) {
        if (!first) out << "/";
        first = false;
        if (!part.empty()) {
            int idx = std::stoi(part);
            int delta = (slot == 0) ? dv : (slot == 1) ? dvt : dvn;
            out << (idx + delta);
        }
        ++slot;
    }
    return out.str();
}

} // namespace

int Remesher::combine_directory(const std::string& input_dir,
                                const std::string& output_obj) {
    DIR* dir = opendir(input_dir.c_str());
    if (!dir) {
        std::cerr << "[Combine] cannot open input dir: " << input_dir << "\n";
        return 0;
    }

    std::vector<std::string> files;
    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        std::string name = entry->d_name;
        if (ends_with(name, ".obj") || ends_with(name, ".OBJ")) files.push_back(name);
    }
    closedir(dir);
    std::sort(files.begin(), files.end());

    std::ofstream out(output_obj);
    if (!out.is_open()) {
        std::cerr << "[Combine] cannot write: " << output_obj << "\n";
        return 0;
    }
    out << "# combined from " << files.size() << " OBJ file(s) in " << input_dir << "\n";

    int v_off = 0, vt_off = 0, vn_off = 0;
    int total_v = 0, total_f = 0;

    for (const auto& name : files) {
        std::ifstream in(input_dir + "/" + name);
        if (!in.is_open()) continue;

        out << "o " << name.substr(0, name.find_last_of('.')) << "\n";

        int v_n = 0, vt_n = 0, vn_n = 0, f_n = 0;
        std::string line;
        while (std::getline(in, line)) {
            if (line.empty() || line[0] == '#') continue;

            if (line.rfind("v ",  0) == 0) { out << line << "\n"; ++v_n;  }
            else if (line.rfind("vt ", 0) == 0) { out << line << "\n"; ++vt_n; }
            else if (line.rfind("vn ", 0) == 0) { out << line << "\n"; ++vn_n; }
            else if (line.rfind("f ", 0) == 0 || line.rfind("l ", 0) == 0) {
                out << line[0];
                std::istringstream iss(line.substr(2));
                std::string token;
                while (iss >> token) out << " " << offset_obj_token(token, v_off, vt_off, vn_off);
                out << "\n";
                if (line[0] == 'f') ++f_n;
            }
            // skip mtllib/usemtl/g/s/o — combining doesn't need them
        }

        v_off  += v_n;
        vt_off += vt_n;
        vn_off += vn_n;
        total_v += v_n;
        total_f += f_n;
    }

    std::cout << "[Combine] " << files.size() << " file(s) -> " << output_obj
              << "  (verts=" << total_v << " faces=" << total_f << ")\n";
    return (int)files.size();
}

int Remesher::remesh_directory(const std::string& input_dir,
                               const std::string& output_dir,
                               const RemeshParams& p) {
    if (!ensure_dir(output_dir)) {
        std::cerr << "[Remesher] cannot create output dir: " << output_dir << "\n";
        return 0;
    }
    // Clear stale .obj from prior runs — otherwise the combine pass mixes
    // current output with leftovers (mix of planar + s3 layers in the same dir).
    int wiped = wipe_objs(output_dir);
    if (wiped > 0) std::cout << "[Remesher] wiped " << wiped << " stale .obj(s) from " << output_dir << "\n";

    DIR* dir = opendir(input_dir.c_str());
    if (!dir) {
        std::cerr << "[Remesher] cannot open input dir: " << input_dir << "\n";
        return 0;
    }

    int processed = 0;
    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        std::string name = entry->d_name;
        if (!ends_with(name, ".obj") && !ends_with(name, ".OBJ")) continue;

        std::string in_path  = input_dir  + "/" + name;
        std::string out_path = output_dir + "/" + name;
        std::cout << "[Remesher] " << name << " ... " << std::flush;
        bool ok = remesh_file(in_path, out_path, p);
        std::cout << (ok ? "ok" : "FAIL") << "\n";
        if (ok) ++processed;
    }
    closedir(dir);
    std::cout << "[Remesher] " << processed << " file(s) remeshed -> " << output_dir << "\n";
    return processed;
}
