#include "MainWindow.h"
#include "Paths.h"
#include <QApplication>
#include <QString>
#include <QByteArray>
#include <QFileInfo>
#include <iostream>
#include <cstring>

static void print_usage(const char* exe) {
    std::cerr <<
        "usage: " << exe << " [--headless <model.tet>] [--case NAME] [--adaptive]\n"
        "\n"
        "  no args            launch GUI (default)\n"
        "  --headless PATH    load PATH (a .tet file), run deformation + inverse\n"
        "                     + layer generation + remesh, exit\n"
        "  --case NAME        deformation: SF | SR | SQ | SL_SQ | SR_SQ | SL_SR | SL_SR_SQ\n"
        "                     (default SL_SR_SQ — README §1.7)\n"
        "  --adaptive         use adaptive-height slicing (README §3.1, needed for\n"
        "                     AnkleBaseV1) instead of fixed layer count\n";
}

int main(int argc, char *argv[])
{
    QString model_path;
    QString case_name = "SL_SR_SQ";
    bool headless = false;
    bool adaptive = false;

    for (int i = 1; i < argc; ++i) {
        const char* a = argv[i];
        if (std::strcmp(a, "--headless") == 0 && i + 1 < argc) {
            headless = true;
            model_path = QString::fromLocal8Bit(argv[++i]);
        } else if (std::strcmp(a, "--case") == 0 && i + 1 < argc) {
            case_name = QString::fromLocal8Bit(argv[++i]);
        } else if (std::strcmp(a, "--adaptive") == 0) {
            adaptive = true;
        } else if (std::strcmp(a, "-h") == 0 || std::strcmp(a, "--help") == 0) {
            print_usage(argv[0]);
            return 0;
        }
    }

    // Anchor every DataSet/ lookup to the project root, discovered by
    // walking up from argv[0] until a sibling DataSet/ appears. This
    // replaces the previous hack of chdir'ing to build/ so the hardcoded
    // "../DataSet/..." strings happened to resolve.
    Paths::init(argv[0]);
    std::cerr << "[Paths] project root: " << Paths::root() << "\n";

    if (headless) {
        QFileInfo fi(model_path);
        if (fi.isRelative()) model_path = fi.absoluteFilePath();
    }

    QApplication a(argc, argv);
    MainWindow w;
    // The UI window flashes up in headless mode too — Qt's offscreen QPA
    // doesn't support QOpenGLWidget on macOS, so we just use the real
    // cocoa platform and exit after the pipeline.
    w.show();

    if (headless) {
        w.runHeadlessPipeline(model_path, case_name, adaptive);
        return 0;
    }

    return a.exec();
}
