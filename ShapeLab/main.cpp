#include "MainWindow.h"
#include <QApplication>
#include <QString>
#include <QByteArray>
#include <QFileInfo>
#include <iostream>
#include <cstring>
#include <unistd.h>

static void print_usage(const char* exe) {
    std::cerr <<
        "usage: " << exe << " [--headless <model.tet>] [--case SL_SR_SQ]\n"
        "\n"
        "  no args            launch GUI (default)\n"
        "  --headless PATH    load PATH (a .tet file), run deformation + inverse\n"
        "                     + layer generation + remesh, exit\n"
        "  --case NAME        deformation case (default: SL_SR_SQ)\n";
}

int main(int argc, char *argv[])
{
    QString model_path;
    QString case_name = "SL_SR_SQ";
    bool headless = false;

    for (int i = 1; i < argc; ++i) {
        const char* a = argv[i];
        if (std::strcmp(a, "--headless") == 0 && i + 1 < argc) {
            headless = true;
            model_path = QString::fromLocal8Bit(argv[++i]);
        } else if (std::strcmp(a, "--case") == 0 && i + 1 < argc) {
            case_name = QString::fromLocal8Bit(argv[++i]);
        } else if (std::strcmp(a, "-h") == 0 || std::strcmp(a, "--help") == 0) {
            print_usage(argv[0]);
            return 0;
        }
    }

    if (headless) {
        // Resolve model_path to absolute BEFORE chdir.
        QFileInfo fi(model_path);
        if (fi.isRelative()) model_path = fi.absoluteFilePath();
        // The slot handlers use hardcoded "../DataSet/..." paths which only
        // resolve correctly when cwd is a direct subdirectory of the project
        // root. Binary lives at build/ShapeLab/ShapeLab, so chdir up one
        // level to build/ — then `..` is the project root and DataSet is
        // beside it.
        QString exe_dir = QFileInfo(QString::fromLocal8Bit(argv[0])).absolutePath();
        QString cwd_target = QFileInfo(exe_dir).absolutePath();  // parent of exe_dir
        if (chdir(cwd_target.toLocal8Bit().constData()) != 0) {
            std::cerr << "[headless] warning: chdir to " << cwd_target.toStdString()
                      << " failed\n";
        }
    }

    QApplication a(argc, argv);
    MainWindow w;
    // The UI window flashes up in headless mode too — Qt's offscreen QPA
    // doesn't support QOpenGLWidget on macOS, so we just use the real
    // cocoa platform and exit after the pipeline.
    w.show();

    if (headless) {
        w.runHeadlessPipeline(model_path, case_name);
        return 0;
    }

    return a.exec();
}
