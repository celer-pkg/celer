# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

cmake_minimum_required(VERSION 3.14)

project(XRPhysicsBaseTeleportationDemo LANGUAGES CXX)


find_package(Qt6 REQUIRED COMPONENTS Core Gui Quick Quick3D Quick3DPhysics Quick3DXr Multimedia)

qt_standard_project_setup(REQUIRES 6.8)

qt_add_executable(${PROJECT_NAME}
    main.cpp
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

target_link_libraries(${PROJECT_NAME} PUBLIC
    Qt::Core
    Qt::Gui
    Qt::Quick
    Qt::Quick3D
    Qt::Quick3DPhysics
    Qt::Quick3DXr
    Qt::Multimedia
)

set(qml_singletons
    FireResources.qml
    ParticleResources.qml
    CampfireMaterial.qml
    InvisibleMaterial.qml
    CommonResources.qml
)

set_source_files_properties(${qml_singletons}
    PROPERTIES QT_QML_SINGLETON_TYPE TRUE)

if(ANDROID)
    set_target_properties(${PROJECT_NAME}  PROPERTIES
        QT_ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android"
        QT_ANDROID_PACKAGE_NAME "io.qt.demo.${PROJECT_NAME}"
        QT_ANDROID_TARGET_SDK_VERSION 34
        QT_ANDROID_VERSION_CODE 10
        QT_ANDROID_VERSION_NAME "1.0"
    )
endif()

# Resources:
qt_add_qml_module(${PROJECT_NAME}
    URI EXAMPLE
    QML_FILES
        ${qml_singletons}
        Main.qml
        ValueFader.qml
        BeamModel.qml
        TargetIndicator.qml
        PhysicsbaseTeleporter.qml
        Grass.qml
        House.qml
        Fire.qml
        Firewood.qml
        Torch.qml
        TorchGripper.qml
        Fence.qml
        Table.qml
        Chair.qml
        Campfire.qml
        Stands.qml
        Smoke.qml
        Ground.qml
        Prompter.qml
    RESOURCES
        "media/meshes/fence_instances.xml.bin"
        "media/meshes/block_instances.xml.bin"
        "media/meshes/firewood_instances.xml.bin"
        "media/shaders/invisible.vert"
        "media/shaders/invisible.frag"
        "media/shaders/target_indicator.vert"
        "media/shaders/target_indicator.frag"
        "media/shaders/grass.vert"
        "media/shaders/grass.frag"
        "media/shaders/fire.vert"
        "media/shaders/fire.frag"
        "media/textures/OpenfootageNET_lowerAustria01-1024.hdr"

        "media/meshes/cylinder_invert.mesh"

        "media/meshes/house/cube_004_mesh.mesh"
        "media/meshes/house/cube_006_mesh.mesh"
        "media/meshes/house/cube_007_mesh.mesh"
        "media/meshes/house/cube_010_mesh.mesh"
        "media/meshes/house/cube_012_mesh.mesh"
        "media/meshes/house/cube_014_mesh.mesh"
        "media/meshes/house/cube_016_mesh.mesh"
        "media/meshes/house/cube_019_mesh.mesh"
        "media/meshes/house/cube_020_mesh.mesh"
        "media/textures/house/wood_bc.jpg"
        "media/textures/house/wood_n.jpg"
        "media/textures/house/wood_r.jpg"
        "media/textures/house/rock_bc.jpg"
        "media/textures/house/rock_n.jpg"
        "media/textures/house/rock_r.jpg"

        "media/meshes/torch/object_1_mesh.mesh"
        "media/textures/torch/torch_bc.jpg"
        "media/textures/torch/torch_n.jpg"
        "media/textures/torch/torch_r.jpg"

        "media/meshes/table/defaultMaterial_mesh7.mesh"
        "media/meshes/table/defaultMaterial_mesh17.mesh"
        "media/meshes/table/defaultMaterial_mesh20.mesh"
        "media/meshes/table/defaultMaterial_mesh23.mesh"
        "media/meshes/table/defaultMaterial_mesh26.mesh"
        "media/meshes/table/defaultMaterial_mesh27.mesh"
        "media/textures/table/table_bc.jpg"
        "media/textures/table/table_n.jpg"
        "media/textures/table/table_mr.jpg"

        "media/meshes/campfire/cube_003_low_campfire_0_mesh.mesh"
        "media/meshes/campfire/cube_004_low_campfire_0_mesh.mesh"
        "media/meshes/campfire/cube_005_low_campfire_0_mesh.mesh"
        "media/meshes/campfire/cube_006_low_campfire_0_mesh.mesh"
        "media/meshes/campfire/cube_007_low_campfire_0_mesh.mesh"
        "media/meshes/campfire/cube_0012_low_campfire_0_mesh.mesh"
        "media/meshes/campfire/cube_0022_low_campfire_0_mesh.mesh"
        "media/meshes/campfire/cube_0032_low_campfire_0_mesh.mesh"
        "media/meshes/campfire/cube_low_campfire_0_mesh.mesh"
        "media/meshes/campfire/cube21_low_campfire_0_mesh.mesh"
        "media/meshes/campfire/cylinder_002_low_campfire_0_mesh.mesh"
        "media/meshes/campfire/cylinder_low_campfire_0_mesh.mesh"
        "media/textures/campfire/campfire_bc.jpg"
        "media/textures/campfire/campfire_n.jpg"
        "media/textures/campfire/campfire_mr.jpg"

        "media/meshes/chair/defaultMaterial_mesh5.mesh"
        "media/meshes/chair/defaultMaterial_mesh13.mesh"
        "media/meshes/chair/defaultMaterial_mesh16.mesh"
        "media/meshes/chair/defaultMaterial_mesh19.mesh"
        "media/textures/chair/chair_bc.jpg"
        "media/textures/chair/chair_n.jpg"
        "media/textures/chair/chair_mr.jpg"

        "media/meshes/firewood/plane_mesh.mesh"
        "media/textures/firewood/firewood_bc.jpg"
        "media/textures/firewood/firewood_n.jpg"
        "media/textures/firewood/firewood_r.jpg"
        "media/textures/firewood/firewood_a.jpg"

        "media/meshes/block/defaultMaterial_mesh.mesh"
        "media/textures/block/block_bc.jpg"
        "media/textures/block/block_n.jpg"
        "media/textures/block/block_mr.jpg"

        "media/textures/ground_bc.jpg"
        "media/textures/ground_n.jpg"
        "media/textures/ground_r.jpg"

        "media/textures/bark_bc.jpg"
        "media/textures/bark_n.jpg"

        "media/textures/turbulence.png"
        "media/textures/perlin.jpg"
        "media/textures/grass_bc.jpg"
        "media/textures/fire.mp4"
        "media/textures/smoke_sprite.png"
    NO_RESOURCE_TARGET_PATH
)


install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_BINDIR}
)

qt_generate_deploy_qml_app_script(
    TARGET ${PROJECT_NAME}
    OUTPUT_SCRIPT deploy_script
    MACOS_BUNDLE_POST_BUILD
    NO_UNSUPPORTED_PLATFORM_ERROR
    DEPLOY_USER_QML_MODULES_ON_UNSUPPORTED_PLATFORM
)
install(SCRIPT ${deploy_script})


