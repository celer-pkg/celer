// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick3D
import QtQuick3D.Helpers

Window {
    width: 1280
    height: 720
    visible: true

    View3D {
        id: view3d
        anchors.fill: parent

        environment: SceneEnvironment {
            backgroundMode: SceneEnvironment.Color
            clearColor: "black"
        }

        PerspectiveCamera {
            id: camera
            z: 300
            y: 100
        }

        property int lightBakeMode: settings.lmEnabled ? Light.BakeModeAll : Light.BakeModeDisabled

        PointLight {
            bakeMode: view3d.lightBakeMode
            y: 190
            castsShadow: true
            shadowFactor: 75
            shadowBias: 20
        }

        PointLight {
            bakeMode: view3d.lightBakeMode
            y: 190
            x: 300
            castsShadow: true
            shadowFactor: 75
            shadowBias: 20
            color: "pink"
        }

        PointLight {
            bakeMode: view3d.lightBakeMode
            y: 30
            x: -300
            castsShadow: true
            shadowFactor: 75
            shadowBias: 20
            color: "cyan"
        }

        Model {
            property string objectName: "Model 0"
            source: settings.shape0Source
            usedInBakedLighting: true
            bakedLightmap: BakedLightmap {
                enabled: settings.lmEnabled
                key: settings.shape0Key
            }
            texelsPerUnit: settings.shape0TPU
            materials: PrincipledMaterial {
                baseColor: "red"
            }
            scale: Qt.vector3d(3, 0.1, 3)
            pickable: true
        }

        Model {
            property string objectName: "Model 1"
            source: settings.shape1Source
            usedInBakedLighting: true
            bakedLightmap: BakedLightmap {
                enabled: settings.lmEnabled
                key: settings.shape1Key
            }
            texelsPerUnit: settings.shape1TPU
            materials: PrincipledMaterial {
                baseColor: "blue"
            }
            position: Qt.vector3d(0, 100, 0)
            scale: Qt.vector3d(0.5, 0.5, 0.5)
            eulerRotation: Qt.vector3d(45, 0, 0)
            pickable: true
        }

        Model {
            property string objectName: "Model 2"
            source: settings.shape2Source
            usedInBakedLighting: true
            bakedLightmap: BakedLightmap {
                enabled: settings.lmEnabled
                key: settings.shape2Key
            }
            texelsPerUnit: settings.shape2TPU
            materials: PrincipledMaterial {
                baseColor: "orange"
            }
            position: Qt.vector3d(100, 100, 0)
            scale: Qt.vector3d(0.5, 0.5, 0.5)
            eulerRotation: Qt.vector3d(45, 0, 0)
            pickable: true
        }

        Model {
            property string objectName: "Model 3"
            source: settings.shape3Source
            usedInBakedLighting: true
            bakedLightmap: BakedLightmap {
                enabled: settings.lmEnabled
                key: settings.shape3Key
            }
            texelsPerUnit: settings.shape3TPU
            materials: PrincipledMaterial {
                baseColor: "cyan"
            }
            position: Qt.vector3d(-100, 100, 100)
            scale: Qt.vector3d(0.5, 0.5, 0.5)
            eulerRotation: Qt.vector3d(45, 0, 0)
            pickable: true
        }
    }

    Label {
        anchors.top: parent.top
        anchors.right: parent.right
        id: pickLabel
        color: "white"
        text: "None"
        font.pointSize: 14
    }

    WasdController {
        controlledObject: camera
        speed: 0.4
        shiftSpeed: 0.8
    }

    MouseArea {
        anchors.fill: view3d

        onClicked: (mouse) => {
            var result = view3d.pick(mouse.x, mouse.y);
            if (result.objectHit) {
                var pickedObject = result.objectHit;
                pickLabel.text = pickedObject.objectName;

            } else {
                pickLabel.text = "None";
            }
        }
    }

    SettingsPane {
        id: settings
        viewport: view3d
    }
}
