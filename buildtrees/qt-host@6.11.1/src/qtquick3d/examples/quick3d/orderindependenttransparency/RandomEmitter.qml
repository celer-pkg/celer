// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick3D.Particles3D

ParticleEmitter3D {
    id: root
    property real timeVariance: 0.0
    property real minTime: 0.0
    property real amountVariance: 0
    property alias running : fa.running
    property vector3d positionVariance: Qt.vector3d(0, 0, 0)

    onTimeVarianceChanged: initRandomEmitter()
    onMinTimeChanged: initRandomEmitter()

    QtObject {
        id: vars
        property int curTicks: 0
        property int burstTicks: 0
        property int amount: 0
        property vector3d position
    }
    function initRandomEmitter()
    {
        var burstTime = Math.random() * timeVariance + minTime
        vars.burstTicks = burstTime / 16
        vars.curTicks = 0
        vars.amount = 1 + Math.random() * amountVariance
        vars.position = Qt.vector3d(Math.random() * positionVariance.x, Math.random() * positionVariance.y, Math.random() * positionVariance.z)
    }

    FrameAnimation {
        id: fa
        onTriggered: {
            vars.curTicks++
            if (vars.curTicks > vars.burstTicks) {
                root.burst(vars.amount, 0, vars.position)
                root.initRandomEmitter()
            }
        }
    }
}
