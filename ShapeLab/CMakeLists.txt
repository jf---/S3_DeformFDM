add_executable(ShapeLab
    main.cpp
    MainWindow.cpp
    DeformTet.cpp
    Fabrication.cpp
    FileIO.cpp
    SupportGeneration.cpp
    heatMethod.cpp
    heatmethodfield.cpp
    IsoLayerGeneration.cpp
    PrincipleStressField.cpp
    toolpathGeneration.cpp
    GcodeGeneration.cpp
    Graph.cpp
    MainWindow.ui
    ShapeLab.qrc
)

target_include_directories(ShapeLab PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/ThirdPartyDependence
)

target_link_libraries(ShapeLab PRIVATE
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    Qt5::OpenGL
    OpenGL::GL
    Eigen3::Eigen
    QMeshLib
    GLKLib
)

if(APPLE)
    target_link_libraries(ShapeLab PRIVATE "-framework OpenGL")
endif()

if(OpenMP_CXX_FOUND)
    target_link_libraries(ShapeLab PRIVATE OpenMP::OpenMP_CXX)
endif()

set_target_properties(ShapeLab PROPERTIES
    MACOSX_BUNDLE FALSE
    WIN32_EXECUTABLE FALSE
)
