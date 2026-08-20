# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_path_join(destination ${QT_INSTALL_DIR} "${INSTALL_QMLDIR}/QtQuick3D/Helpers/designer/images")
qt_copy_or_install(
    FILES
        axishelper.png
        axishelper16.png
        axishelper@2x.png
        debugview.png
        debugview16.png
        debugview@2x.png
        dummy.png
        dummy16.png
        dummy@2x.png
        extendedsceneenvironment.png
        extendedsceneenvironment16.png
        extendedsceneenvironment@2x.png
        gridgeometry.png
        gridgeometry16.png
        gridgeometry@2x.png
        heightfieldgeometry.png
        heightfieldgeometry16.png
        heightfieldgeometry@2x.png
        infinitegrid.png
        infinitegrid16.png
        infinitegrid@2x.png
        instancemodel.png
        instancemodel16.png
        instancemodel@2x.png
        lookatnode.png
        lookatnode16.png
        lookatnode@2x.png
        instancerepeater.png
        instancerepeater16.png
        instancerepeater@2x.png
        wasdcontroller.png
        wasdcontroller16.png
        wasdcontroller@2x.png
        orbitcameracontroller.png
        orbitcameracontroller16.png
        orbitcameracontroller@2x.png
        proceduralskytexturedata.png
        proceduralskytexturedata16.png
        proceduralskytexturedata@2x.png
        lodmanager.png
        lodmanager16.png
        lodmanager@2x.png
    DESTINATION "${destination}"
)
