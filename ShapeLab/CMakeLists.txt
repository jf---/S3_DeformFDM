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
    Paths.cpp
    Remesher.cpp
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
    vcglib::vcglib
)

if(APPLE)
    target_link_libraries(ShapeLab PRIVATE "-framework OpenGL")
endif()

if(OpenMP_CXX_FOUND)
    target_link_libraries(ShapeLab PRIVATE OpenMP::OpenMP_CXX)
endif()

if(USE_MKL)
    target_link_libraries(ShapeLab PRIVATE MKL::MKL)
endif()

set_target_properties(ShapeLab PROPERTIES
    MACOSX_BUNDLE FALSE
    WIN32_EXECUTABLE FALSE
)

add_executable(remesh_smoketest remesh_smoketest.cpp Remesher.cpp Paths.cpp)
target_link_libraries(remesh_smoketest PRIVATE vcglib::vcglib Eigen3::Eigen)
target_include_directories(remesh_smoketest PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
set_target_properties(remesh_smoketest PROPERTIES
    MACOSX_BUNDLE FALSE
    WIN32_EXECUTABLE FALSE
)
