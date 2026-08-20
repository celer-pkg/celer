// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick3D
import QtQuick3D.Physics
import QtMultimedia

Node {
    id: shapeSpawner

    required property PhysicsMaterial physicsMaterial
    property int count: 0
    property double diceWidth: 3.5
    property vector3d rollForce: Qt.vector3d(0, 0, 0)

    onCountChanged: respawn()

    Repeater3D {
        id: repeater

        model: new Array(shapeSpawner.count).fill({
                                                      "ts": new Date().valueOf()
                                                  })

        delegate: PhysicalDie {
            required property int index
            eulerRotation.x: shapeSpawner.randomInRange(0, 360)
            eulerRotation.y: shapeSpawner.randomInRange(0, 360)
            eulerRotation.z: shapeSpawner.randomInRange(0, 360)
            physicsMaterial: shapeSpawner.physicsMaterial
            diceWidth: shapeSpawner.diceWidth

            Component.onCompleted: {
                position.x = diceWidth * Math.cos(index / (Math.PI / 4))
                position.y = index * shapeSpawner.diceWidth * 1.41
                position.z = diceWidth * Math.sin(index / (Math.PI / 4))
                applyCentralForce(shapeSpawner.rollForce)
            }
        }
    }

    SoundEffect {
        id: rollSound
        loops: 0
        source: "sounds/rolling.wav"
    }

    function respawn() {
        repeater.model = new Array(shapeSpawner.count).fill({
                                                                "ts": new Date().valueOf()
                                                            })
        rollSound.stop()
        rollSound.play()
    }

    function randomInRange(min, max) {
        return Math.random() * (max - min) + min
    }
}
