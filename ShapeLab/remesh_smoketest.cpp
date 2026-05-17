// Tiny CLI to smoke-test the in-process remesher.
// Usage: remesh_smoketest <input.obj> [output.obj]
#include "Remesher.h"
#include <iostream>
#include <string>

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "usage: " << argv[0] << " <input.obj> [output.obj]\n";
        return 2;
    }
    std::string in  = argv[1];
    std::string out = (argc >= 3) ? argv[2] : in + ".remeshed.obj";
    std::cout << "[smoketest] " << in << " -> " << out << "\n";
    return Remesher::remesh_file(in, out) ? 0 : 1;
}
