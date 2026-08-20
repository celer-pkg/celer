// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import QtQuick3D
import QtQuick3D.Helpers
import QtQuick3D.Xr

XrView {
    id: xrView
    referenceSpace: XrView.ReferenceSpaceLocalFloor

    depthSubmissionEnabled: true

    xrOrigin: theOrigin

    environment: SceneEnvironment {
        id: sceneEnvironment
        lightProbe: Texture {
            textureData: ProceduralSkyTextureData {
            }
        }
        antialiasingMode: SceneEnvironment.MSAA
        antialiasingQuality: SceneEnvironment.High
        backgroundMode: SceneEnvironment.Color
        clearColor: "skyblue"
        probeHorizon: 0.5
    }

    DirectionalLight {
        eulerRotation.x: -30
        eulerRotation.y: -70
    }

    //! [haptics]
    XrHapticFeedback {
        controller: XrHapticFeedback.RightController
        condition: XrHapticFeedback.RisingEdge
        trigger: pickRay.hit
        hapticEffect: XrSimpleHapticEffect {
            amplitude: 0.5
            duration: 10
            frequency: 200
        }
    }
    //! [haptics]

    XrOrigin {
        id: theOrigin
        z: 100
        //! [picking]
        XrController {
            id: rightController
            controller: XrController.RightController
            poseSpace: XrController.AimPose

            property QtObject hitObject

            onRotationChanged: {
                const pickResult = xrView.rayPick(scenePosition, forward)
                if (pickResult.hitType !== PickResult.Null) {
                    pickRay.hit = true
                    pickRay.length = pickResult.distance
                    hitObject = pickResult.objectHit
                    cursorSphere.position = pickResult.scenePosition
                    cursorSphere.visible = true
                } else {
                    pickRay.hit = false
                    pickRay.length = 50
                    hitObject = null
                    cursorSphere.visible = false
                }
            }

            Node {
                id: pickRay
                property real length: 50
                property bool hit: false

                Model {
                    eulerRotation.x: -90
                    scale: Qt.vector3d(0.005, pickRay.length/100, 0.005)
                    source: "#Cone"
                    materials: PrincipledMaterial {
                        baseColor: rightTrigger.pressed ? "#99aaff" : "#cccccc"
                        lighting: PrincipledMaterial.NoLighting
                    }
                    opacity: 0.8
                }
            }

            Node {
                z: 5
                Model {
                    eulerRotation.x: 90
                    scale: Qt.vector3d(0.05, 0.10, 0.05)
                    source: "#Cylinder"
                    materials: PrincipledMaterial {
                        baseColor: "black"
                        roughness: 0.2
                    }
                }
            }
        }
        //! [picking]
    }

    Model {
        id: cursorSphere
        source: "#Sphere"
        materials: PrincipledMaterial {
            baseColor: "#777777"
            lighting: PrincipledMaterial.NoLighting
        }
        opacity: 0.4
        scale: Qt.vector3d(0.05, 0.05, 0.05)
        visible: false
    }

    //! [trigger input]
    XrInputAction {
        id: rightTrigger
        controller: XrInputAction.RightController
        actionId: [XrInputAction.TriggerPressed, XrInputAction.TriggerValue, XrInputAction.IndexFingerPinch]
        onTriggered: {
            const button = rightController.hitObject as ExampleButton
            if (button && button !== panel.activeButton) {
                panel.activeButton = button
            }
        }
    }
    //! [trigger input]

    //! [mouse input]
    XrInputAction {
        id: rightThumbstickX
        controller: XrInputAction.RightController
        actionId: [XrInputAction.ThumbstickX]
    }
    XrInputAction {
        id: rightThumbstickY
        controller: XrInputAction.RightController
        actionId: [XrInputAction.ThumbstickY]
    }

    XrVirtualMouse {
        view: xrView
        source: rightController
        leftMouseButton: rightTrigger.pressed
        scrollWheelX: rightThumbstickX.value
        scrollWheelY: rightThumbstickY.value
    }
    //! [mouse input]

    Model {
        id: floor
        source: "#Rectangle"
        eulerRotation.x: -90
        scale: Qt.vector3d(5,5,5)
        materials: [ PrincipledMaterial {
                baseColor: "green"
            }
        ]
    }

    Model {
        id: table
        property real height: 70
        position: Qt.vector3d(0, height / 2, 0)
        source: "#Cube"
        scale: Qt.vector3d(3, height / 100, 1)
        materials: PrincipledMaterial {
            baseColor: "#554433"
        }
    }

    Model {
        y: table.height + 2.5
        source: "#Cylinder"
        scale: Qt.vector3d(0.5, 0.05, 0.5)
        materials: PrincipledMaterial {
            baseColor: "black"
            roughness: 0.7
        }
    }

    Model {
        id: teapot
        y: table.height + 5
        source: "meshes/teapot.mesh"
        scale: Qt.vector3d(10, 10, 10)
        property color color: "#cdad52"
        materials: [
            PrincipledMaterial {
                baseColor: teapot.color
                roughness: 0.1
                clearcoatAmount: clearcoatSlider.value
                clearcoatRoughnessAmount: 0.1
                metalness: metalnessCheckBox.checked ? 1.0 : 0.0
            }
        ]
        property real speedFactor: speedSlider.value
        FrameAnimation {
            running: true
            onTriggered: {
                const speed = panel.activeButton?.rotationSpeed * teapot.speedFactor
                teapot.eulerRotation.y += speed * frameTime
            }
        }
    }

    Teacup {
        x: 50
        y: table.height
        scale: Qt.vector3d(1.5, 1.5, 1.5)
        color: colorView.selectedColor
    }

    Node {
        id: panel
        y: table.height
        z: 50
        eulerRotation.x: -45
        Model {
            pickable: true
            scale: Qt.vector3d(1, 0.2, 0.02)
            source: "#Cube"
            materials: PrincipledMaterial {
                baseColor: "lightgray"
            }
        }
        property ExampleButton activeButton: midButton
        ExampleButton {
            id: leftButton
            x: -35
            on: panel.activeButton === this
            objectName: "Button 1"
            rotationSpeed: -45
        }
        ExampleButton {
            id: midButton
            x: 0
            on: panel.activeButton === this
            objectName: "Button 2"
            rotationSpeed: 0
        }
        ExampleButton {
            id: rightButton
            x: 35
            on: panel.activeButton === this
            objectName: "Button 3"
            rotationSpeed: 45
        }
    }

    //! [xritem start]
    XrItem {
        width: 75
        height: 100
        x: -100
        y: height + table.height + 5
        z: 40
        eulerRotation.y: 45
        color: "transparent"
        contentItem: Rectangle {
            color: Qt.rgba(1, 1, 1, 0.5)
            border.width: 5
            border.color: "lightblue"
            height: 400
            width: 300
            radius: 25
    //! [xritem start]

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 10
                Button {
                    text: "Red"
                    onClicked: teapot.color = "#dd3311"
                    Layout.alignment: Qt.AlignHCenter
                    Layout.preferredWidth: parent.width * 2 / 3
                }
                Button {
                    text: "Green"
                    onClicked: teapot.color = "#41cd52"
                    Layout.alignment: Qt.AlignHCenter
                    Layout.preferredWidth: parent.width * 2 / 3
                }
                Button {
                    text: "Blue"
                    onClicked: teapot.color = "#103f8b"
                    Layout.alignment: Qt.AlignHCenter
                    Layout.preferredWidth: parent.width * 2 / 3
                }
                Slider {
                    id: speedSlider
                    from: 0.5
                    value: 1
                    to: 5
                    wheelEnabled: true
                    Layout.fillWidth: true
                }
                Text {
                    text: "speed: " + speedSlider.value.toFixed(1)
                }
                Slider {
                    id: clearcoatSlider
                    from: 0.0
                    value: 0.0
                    to: 1.0
                    wheelEnabled: true
                    Layout.fillWidth: true
                }
                Text {
                    text: "clearcoat: " + clearcoatSlider.value.toFixed(1)
                }
                CheckBox {
                    id: metalnessCheckBox
                    text: "Metallic"
                    checked: false
                }
    //! [xritem end]
            }
        }
    //! [xritem end]
    }

    Node {
        x: 150
        y: listviewItem.height + table.height + 5
        z: 40
        eulerRotation.y: -45
        XrItem {
            id: listviewItem
            width: 75
            height: 100
            x: -width
            color: "transparent"

            contentItem: Rectangle {
                color: Qt.rgba(1, 1, 1, 0.5)
                border.width: 5
                border.color: "lightblue"
                height: 400
                width: 300
                radius: 25

                ColorView {
                    id: colorView
                    anchors.fill: parent
                    anchors.margins: 25
                }
            }
        }
    }
}
