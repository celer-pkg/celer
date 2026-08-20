# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_path_join(destination ${QT_INSTALL_DIR} "${INSTALL_QMLDIR}/QtQuick3D/Helpers/designer")
qt_copy_or_install(
    FILES
        AxisHelperSection.qml
        AxisHelperSpecifics.qml
        DebugViewSection.qml
        DebugViewSpecifics.qml
        ExtendedSceneEnvironmentSection.qml
        ExtendedSceneEnvironmentSpecifics.qml
        GridGeometrySection.qml
        GridGeometrySpecifics.qml
        HeightFieldGeometrySection.qml
        HeightFieldGeometrySpecifics.qml
        InfiniteGridSection.qml
        InfiniteGridSpecifics.qml
        InstanceModelSection.qml
        InstanceModelSpecifics.qml
        InstanceRepeaterSection.qml
        InstanceRepeaterSpecifics.qml
        LodManagerSection.qml
        LodManagerSpecifics.qml
        LookAtNodeSection.qml
        LookAtNodeSpecifics.qml
        NodeSection.qml
        OrbitCameraControllerSection.qml
        OrbitCameraControllerSpecifics.qml
        ProceduralSkyTextureDataSection.qml
        ProceduralSkyTextureDataSpecifics.qml
        Repeater3DSection.qml
        WasdControllerSection.qml
        WasdControllerSpecifics.qml
        helpers.metainfo
    DESTINATION "${destination}"
)

add_subdirectory(images)
