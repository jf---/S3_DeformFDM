#pragma once

#include <string>
#include <vector>

// Defaults match the upstream MeshLab .mlx (Isotropic Explicit Remeshing):
// 5 iterations, target edge length 2.0 (% of bbox diag), 30° crease angle,
// all 5 sub-steps enabled, adaptive off.
struct RemeshParams {
    float  target_edge_len    = 2.0f;
    float  feature_angle_deg  = 30.0f;
    int    iterations         = 5;
    bool   adaptive           = false;
    bool   split              = true;
    bool   collapse           = true;
    bool   swap               = true;
    bool   smooth             = true;
    bool   reproject          = true;
};

// In-process isotropic remesher (vcglib) replacing the upstream
// remesh_slimmedLayer.bat → MeshLab pipeline. Same Botsch-Kobbelt-style
// algorithm MeshLab itself uses (MeshLab is built on vcglib).
class Remesher {
public:
    // Drop-in for the .bat: every OBJ in input_dir is remeshed and written
    // (same filename) into output_dir. Returns number of files processed.
    static int remesh_directory(const std::string& input_dir,
                                const std::string& output_dir,
                                const RemeshParams& p = {});

    // Single-file. Returns true on success.
    static bool remesh_file(const std::string& input_obj,
                            const std::string& output_obj,
                            const RemeshParams& p = {});
};
