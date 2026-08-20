# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_internal_add_manual_test(manual_test_specularglossymaterial
    GUI
    SOURCES
        main.cpp
    LIBRARIES
        Qt::Gui
        Qt::Quick
        Qt::Quick3DPrivate
)

# Resources:
set(qml_resource_files
    "main.qml"
)

qt_internal_add_resource(manual_test_specularglossymaterial "qml"
    PREFIX
        "/"
    FILES
        ${qml_resource_files}
)
set(materials_resource_files
    "maps/WaterBottle_baseColor.jpg"
    "maps/WaterBottle_emissive.png"
    "maps/WaterBottle_normal.png"
    "maps/WaterBottle_occlusion.png"
    "maps/WaterBottle_roughnessMetallic.jpg"
    "maps/WaterBottle_specularGlossiness.png"
    "maps/WaterBottle_diffuse.jpg"
    "meshes/waterBottle_MR.mesh"
    "meshes/waterBottle_SpecGloss.mesh"
)

qt_internal_add_resource(manual_test_specularglossymaterial "assets"
    PREFIX
        "/"
    FILES
        ${materials_resource_files}
)
