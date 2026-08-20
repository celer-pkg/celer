# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_path_join(destination ${QT_INSTALL_DIR} "${INSTALL_QMLDIR}/QtQuick3D/Physics/designer/images")
qt_copy_or_install(
    FILES
        physicsworld.png
        physicsworld16.png
        physicsworld@2x.png
        triggerbody.png
        triggerbody16.png
        triggerbody@2x.png
        staticrigidbody.png
        staticrigidbody16.png
        staticrigidbody@2x.png
        dynamicrigidbody.png
        dynamicrigidbody16.png
        dynamicrigidbody@2x.png
        physicsmaterial.png
        physicsmaterial16.png
        physicsmaterial@2x.png
        boxshape.png
        boxshape16.png
        boxshape@2x.png
        capsuleshape.png
        capsuleshape16.png
        capsuleshape@2x.png
        charactercontroller.png
        charactercontroller16.png
        charactercontroller@2x.png
        convexmeshshape.png
        convexmeshshape16.png
        convexmeshshape@2x.png
        heightfieldshape.png
        heightfieldshape16.png
        heightfieldshape@2x.png
        planeshape.png
        planeshape16.png
        planeshape@2x.png
        sphereshape.png
        sphereshape16.png
        sphereshape@2x.png
        trianglemeshshape.png
        trianglemeshshape16.png
        trianglemeshshape@2x.png
    DESTINATION "${destination}"
)
