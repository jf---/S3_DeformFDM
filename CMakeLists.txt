cmake_minimum_required(VERSION 3.24)
project(S3_DeformFDM CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/ShapeLab")

# Disable the per-user CMake package registry; other CADCAM projects export
# their build-tree Eigen3 into ~/.cmake/packages/Eigen3/ and we would pick
# up THEIR Eigen (with MKL Pardiso enabled) instead of the pixi-local one.
set(CMAKE_FIND_USE_PACKAGE_REGISTRY OFF)
set(CMAKE_FIND_USE_SYSTEM_PACKAGE_REGISTRY OFF)

find_package(Qt5 5.12 COMPONENTS Core Gui Widgets OpenGL REQUIRED)
find_package(OpenGL REQUIRED)
find_package(OpenMP REQUIRED)
find_package(Eigen3 3.3 NO_MODULE HINTS "$ENV{CONDA_PREFIX}")

if(NOT TARGET Eigen3::Eigen)
    add_library(Eigen3::Eigen INTERFACE IMPORTED)
    set_target_properties(Eigen3::Eigen PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}/ThirdPartyDependence/eigen3")
endif()

if(WIN32)
    add_compile_definitions(_WINDOWS _UNICODE _ENABLE_EXTENDED_ALIGNED_STORAGE WIN64)
endif()

if(APPLE)
    # Silence the wall of OpenGL deprecation warnings — Apple deprecated GL in 10.14
    # but the legacy fixed-function pipeline still works; modernizing is out of scope.
    add_compile_definitions(GL_SILENCE_DEPRECATION)
endif()

add_subdirectory(GLKLib)
add_subdirectory(QMeshLib)
add_subdirectory(ShapeLab)
