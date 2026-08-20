// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Controls
import QtQuick3D
import QtQuick3D.Helpers

ApplicationWindow {
    id: window
    visible: true
    width: 1200
    height: 720
    title: qsTr("Cascaded Shadow Maps Example")

    //! [scene]
    View3D {
        id: view
        x: settings.viewX
        y: 0
        width: parent.width - settings.viewX
        height: parent.height
        camera: camera
        environment: SceneEnvironment {
            clearColor: "lightblue"
            backgroundMode: SceneEnvironment.Color
            antialiasingMode: SceneEnvironment.MSAA
            antialiasingQuality: SceneEnvironment.High
        }

        PerspectiveCamera {
            id: camera
            position: Qt.vector3d(458, 300, 515)
            eulerRotation: Qt.vector3d(-14, 19, 0)
            clipFar: settings.clipFar
        }

        DirectionalLight {
            visible: true
            castsShadow: true
            shadowFactor: settings.shadowFactor
            eulerRotation: settings.eulerRotation
            csmSplit1: settings.csmSplit1
            csmSplit2: settings.csmSplit2
            csmSplit3: settings.csmSplit3
            csmNumSplits: settings.csmNumSplits
            shadowMapQuality: settings.shadowMapQuality
            csmBlendRatio: settings.csmBlendRatio
            shadowBias: settings.shadowBias
            pcfFactor: settings.pcfFactor
            softShadowQuality: settings.softShadowQuality
            shadowMapFar: settings.shadowMapFar
            lockShadowmapTexels: settings.lockShadowmapTexels
        }

        Model {
            id: ground
            source: "#Cube"
            scale: Qt.vector3d(25, 0.01, 135)
            z: -5500
            materials: PrincipledMaterial {
                baseColor: "gray"
            }
            castsShadows: false
        }

        Node {
            id: shapeSpawner
            Component.onCompleted: {
                var conesAndCylinderTrio = Qt.createComponent("ConesAndCylinderTrio.qml")
                var z_pos = 0
                for (var i = 0; i < 25; i++) {
                    conesAndCylinderTrio.incubateObject(shapeSpawner, {
                                                        "z_positions": [
                                                                z_pos,
                                                                z_pos - 125,
                                                                z_pos - 250
                                                            ]})
                    z_pos -= 450
                }
            }
        }

        MouseArea {
            anchors.fill: parent
            acceptedButtons: Qt.LeftButton
            propagateComposedEvents: true
            onClicked: {
                settings.drawerVisible = false
                wasd.forceActiveFocus() // focus WASD controller for keyboard input
            }
        }
    }
    //! [scene]

    WasdController {
        id: wasd
        controlledObject: view.camera
        speed: 5
        shiftSpeed: 10
        enabled: !settings.drawerOpen
    }

    SettingsPane {
        id: settings
    }

}
