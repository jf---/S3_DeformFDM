#include "Paths.h"

#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

namespace Paths {
namespace {
    std::string g_root;

    fs::path walk_up_for_dataset(fs::path start) {
        fs::path cur = fs::absolute(start);
        if (fs::is_regular_file(cur)) cur = cur.parent_path();
        for (int i = 0; i < 32; ++i) {  // bounded — never traverse filesystem root
            if (fs::is_directory(cur / "DataSet")) return cur;
            fs::path parent = cur.parent_path();
            if (parent == cur) break;  // hit filesystem root
            cur = parent;
        }
        return {};
    }
}

void init(const std::string& argv0) {
    fs::path found = walk_up_for_dataset(fs::path(argv0));
    if (found.empty()) {
        found = walk_up_for_dataset(fs::current_path());
    }
    if (found.empty()) {
        g_root = fs::current_path().string();
        std::cerr << "[Paths] WARNING: no DataSet/ found walking up from "
                  << argv0 << " or cwd. Falling back to cwd: " << g_root << "\n";
    } else {
        g_root = found.string();
    }
}

const std::string& root() { return g_root; }

std::string dataset(const std::string& subpath) {
    // Plain concat (not filesystem::path / operator) so trailing slashes in
    // `subpath` are preserved — callers like `Paths::dataset("foo/") + name`
    // need that, and the platforms we run on accept either separator anyway.
    return g_root + "/DataSet/" + subpath;
}

} // namespace Paths
