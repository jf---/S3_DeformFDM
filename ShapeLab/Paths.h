#pragma once

#include <string>

// Project-root anchored path resolution. Auto-detects the root by walking up
// from argv[0] until it finds a sibling `DataSet/` directory. Lets the binary
// run from anywhere (no cwd dependency, no chdir hacks).
namespace Paths {
    // Walks up from argv0 to find the project root (the first ancestor with
    // a `DataSet/` subdirectory). Idempotent. Falls back to current working
    // directory if no DataSet/ found anywhere (shouldn't happen for us).
    void init(const std::string& argv0);

    // Returns absolute path to the discovered project root.
    const std::string& root();

    // Returns absolute path to <root>/DataSet/[subpath]. Pass "" for the
    // DataSet/ dir itself. subpath should NOT start with a slash.
    std::string dataset(const std::string& subpath = "");
}
