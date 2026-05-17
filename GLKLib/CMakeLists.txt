add_library(GLKLib STATIC
    GLKGeometry.cpp
    GLKGraph.cpp
    GLKHeap.cpp
    GLKLib.cpp
    GLKMatrixLib.cpp
    GLKNearestNeighbor.cpp
    GLKObList.cpp
    GLKRational.cpp
    GLKSparseMatrix.cpp
    GLKSparseMatrixDirectSolver.cpp
    GLKSparseMatrixIterativeSolver.cpp
    InteractiveTool.cpp
)

target_include_directories(GLKLib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}
)

target_link_libraries(GLKLib PUBLIC
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    Qt5::OpenGL
    OpenGL::GL
    Eigen3::Eigen
)

if(APPLE)
    target_link_libraries(GLKLib PUBLIC "-framework OpenGL")
elseif(UNIX)
    find_package(OpenGL REQUIRED COMPONENTS GLU)
    target_link_libraries(GLKLib PUBLIC OpenGL::GLU)
endif()

if(OpenMP_CXX_FOUND)
    target_link_libraries(GLKLib PUBLIC OpenMP::OpenMP_CXX)
endif()
