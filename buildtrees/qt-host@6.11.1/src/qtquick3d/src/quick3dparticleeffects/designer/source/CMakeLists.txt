# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_copy_or_install(
    FILES
        particleeffect_clouds.qml
        particleeffect_dust.qml
        particleeffect_exhaust.qml
        particleeffect_fire.qml
        particleeffect_heavyrain.qml
        particleeffect_heavyrain_tirespray.qml
        particleeffect_lightrain.qml
        particleeffect_lightrain_tirespray.qml
        particleeffect_rainmist.qml
        particleeffect_steam.qml
        particleeffect_snow.qml
    DESTINATION "${INSTALL_QMLDIR}/QtQuick3D/ParticleEffects/designer/source"
)
