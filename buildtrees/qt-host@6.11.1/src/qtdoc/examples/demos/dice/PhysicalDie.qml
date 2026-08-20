// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
import QtQuick3D.Physics
import QtMultimedia
import QtQml

DynamicRigidBody {
    id: root
    property real diceWidth: 1.9 // cm
    property alias atRest: motionTimeout.running
    property bool isClose: position.length() < 100
    receiveContactReports: true
    sendContactReports: true
    onBodyContact: (body, positions, impulses, normals) => {
        motionTimeout.restart()
        const volume = impulses.reduce((acc, vector) => acc + vector.length(), 0)
        diceSound.volume = volume / 2000
        if (!diceSound.playing)
            diceSound.play()
    }
    Timer {
        id: motionTimeout
        interval: 500
        running: false
        repeat: false
    }

    massMode: DynamicRigidBody.CustomDensity
    density: 1.13
    collisionShapes: BoxShape {
        id: box
        extents: Qt.vector3d(1, 1, 1).times(root.diceWidth)
    }
    Dice_low {
        receivesShadows: root.isClose
        scale: Qt.vector3d(2.65, 2.65, 2.65).times(root.diceWidth)
    }
    SoundEffect {
        id: diceSound
        loops: 0
        source: "sounds/onedice.wav"
    }
}
