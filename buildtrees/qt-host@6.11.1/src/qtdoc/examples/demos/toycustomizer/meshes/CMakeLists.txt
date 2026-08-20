# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

set(assets_meshes
    mesh_022_mesh.mesh
    mesh_025_mesh.mesh
    mesh_027_mesh.mesh
    mesh_029_mesh.mesh
    mesh_035_mesh.mesh
    mesh_063_mesh.mesh
    mesh_076_mesh.mesh
    mesh_079_mesh.mesh
    mesh_081_mesh.mesh
    mesh_094_mesh.mesh
    mesh_095_mesh.mesh
    mesh_096_mesh.mesh
    mesh_097_mesh.mesh
    mesh_098_mesh.mesh
    mesh_099_mesh.mesh
    mesh_100_mesh.mesh
    mesh_101_mesh.mesh
    mesh_102_mesh.mesh
    mesh_103_mesh.mesh
    mesh_104_mesh.mesh
    mesh_105_mesh.mesh
    mesh_106_mesh.mesh
    mesh_107_mesh.mesh
    mesh_108_mesh.mesh
    mesh_109_mesh.mesh
    mesh_110_mesh.mesh
    mesh_111_mesh.mesh
    mesh_112_mesh.mesh
    mesh_002_mesh.mesh
    mesh_124_mesh.mesh
    mesh_mesh.mesh
    plane_016_mesh.mesh
    skin_bear.mesh
    skin_deer.mesh
    skin_cat.mesh
    skin_sheep.mesh
    skin_lion.mesh
    skin_raccoon.mesh
    skin_squirrel.mesh
    skin_monkey.mesh
    skin_panda.mesh
    skin_koala.mesh
    skin_sloth.mesh
    skin_pig.mesh
    skin_rabbit.mesh
    skin_tiger.mesh
    mesh_003_mesh.mesh
    mesh_013_mesh.mesh
    torus_001_mesh.mesh
    cylinder_009_mesh.mesh
    plane_018_mesh.mesh
    empty_003_mesh.mesh
    empty_mesh.mesh
    mesh_019_mesh.mesh
    mesh_011_mesh.mesh
    cylinder_004_mesh.mesh
    nvgoggles_mesh.mesh
    plane_014_mesh.mesh
    plane_015_mesh.mesh
    cap_mesh.mesh
    cylinder_002_mesh.mesh
    mesh_005_mesh.mesh
    mesh_006_mesh.mesh
    mesh_009_mesh.mesh
    mesh_007_mesh.mesh
    mesh_010_mesh.mesh
    mesh_015_mesh.mesh
    mesh_014_mesh.mesh
    mesh_016_mesh.mesh
    empty_001_mesh.mesh
    mesh_004_mesh.mesh
    plane_017_mesh.mesh
    cylinder_001_mesh.mesh
    cylinder_005_mesh.mesh
    mesh_012_mesh.mesh
    cone_mesh.mesh
    cube_001_mesh.mesh
    torus_002_mesh.mesh
    plane_001_mesh.mesh
    mesh_008_mesh.mesh
)

qt_target_qml_sources(${TARGET_NAME}
    PREFIX "/qt/qml/ToyCustomizer/qml/meshes/"
    RESOURCES "${assets_meshes}"
)

set(clean_meshes "${assets_meshes}")
list(TRANSFORM clean_meshes PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/")
set_property(DIRECTORY APPEND PROPERTY ADDITIONAL_CLEAN_FILES "${clean_meshes}")
