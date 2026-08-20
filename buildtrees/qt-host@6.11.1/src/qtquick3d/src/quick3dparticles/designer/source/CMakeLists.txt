# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_path_join(destination ${QT_INSTALL_DIR} "${INSTALL_QMLDIR}/QtQuick3D/Particles3D/designer/source")
qt_copy_or_install(
    FILES
        particlesystem_animatedsprite_template.qml
        particlesystem_attractor_template.qml
        particlesystem_burst_template.qml
        particlesystem_modelblend_template.qml
        particlesystem_modelshape_template.qml
        particlesystem_particletrail_template.qml
        particlesystem_sprite_template.qml
        particlesystem_template.qml
        particlesystem_wander_template.qml
    DESTINATION "${destination}"
)
