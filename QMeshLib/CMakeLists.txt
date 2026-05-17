set(PQP_DIR ${CMAKE_SOURCE_DIR}/ThirdPartyDependence/PQPLib)
set(QHULL_DIR ${CMAKE_SOURCE_DIR}/ThirdPartyDependence/QHullLib)

add_library(QMeshLib STATIC
    PolygenMesh.cpp
    QMeshCluster.cpp
    QMeshEdge.cpp
    QMeshFace.cpp
    QMeshNode.cpp
    QMeshPatch.cpp
    QMeshTetra.cpp
    ${PQP_DIR}/BV.cpp
    ${PQP_DIR}/Build.cpp
    ${PQP_DIR}/PQP.cpp
    ${PQP_DIR}/TriDist.cpp
    ${QHULL_DIR}/geom.cpp
    ${QHULL_DIR}/geom2.cpp
    ${QHULL_DIR}/global.cpp
    ${QHULL_DIR}/io.cpp
    ${QHULL_DIR}/men.cpp
    ${QHULL_DIR}/merge.cpp
    ${QHULL_DIR}/poly.cpp
    ${QHULL_DIR}/poly2.cpp
    ${QHULL_DIR}/qconvex.cpp
    ${QHULL_DIR}/qhull.cpp
    ${QHULL_DIR}/qset.cpp
    ${QHULL_DIR}/rbox.cpp
    ${QHULL_DIR}/stat.cpp
    ${QHULL_DIR}/user.cpp
)

target_include_directories(QMeshLib
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}
    PRIVATE
        ${PQP_DIR}
        ${QHULL_DIR}
)

target_link_libraries(QMeshLib PUBLIC
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    Qt5::OpenGL
    Eigen3::Eigen
    GLKLib
)

# PQP and QHull are 1990s-vintage C++ — use of removed `register` keyword,
# C-style sloppy casts, etc. Silence rather than rewrite upstream code.
set(_legacy_third_party
    ${PQP_DIR}/BV.cpp
    ${PQP_DIR}/Build.cpp
    ${PQP_DIR}/PQP.cpp
    ${PQP_DIR}/TriDist.cpp
    ${QHULL_DIR}/geom.cpp
    ${QHULL_DIR}/geom2.cpp
    ${QHULL_DIR}/global.cpp
    ${QHULL_DIR}/io.cpp
    ${QHULL_DIR}/men.cpp
    ${QHULL_DIR}/merge.cpp
    ${QHULL_DIR}/poly.cpp
    ${QHULL_DIR}/poly2.cpp
    ${QHULL_DIR}/qconvex.cpp
    ${QHULL_DIR}/qhull.cpp
    ${QHULL_DIR}/qset.cpp
    ${QHULL_DIR}/rbox.cpp
    ${QHULL_DIR}/stat.cpp
    ${QHULL_DIR}/user.cpp
)
set_source_files_properties(${_legacy_third_party} PROPERTIES
    COMPILE_OPTIONS "-Wno-register;-Wno-deprecated-register;-Wno-deprecated-declarations;-Wno-writable-strings"
)
# PQP.cpp pulls in OBB_Disjoint.h transitively
set_source_files_properties(${PQP_DIR}/Build.cpp ${PQP_DIR}/PQP.cpp PROPERTIES
    COMPILE_OPTIONS "-Wno-register;-Wno-deprecated-register"
)
