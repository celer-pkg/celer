// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick3D
import QtQuick3D.Helpers
import QtQuick3D.Xr

import xr_shared

pragma ComponentBehavior: Bound

XrView {
    id: xrView
    referenceSpace: XrView.ReferenceSpaceLocalFloor

    depthSubmissionEnabled: true

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
        id: hapticFeedback
        controller: XrHapticFeedback.RightController
        hapticEffect: XrSimpleHapticEffect {
            amplitude: 0.5
            duration: 10
            frequency: 200
        }
        property Model prevObj: null
        function handleHover(obj: Model) {
            if (obj && obj !== prevObj)
                start()
            prevObj = obj
        }
    }
    //! [haptics]

    xrOrigin: XrOrigin {
        id: theOrigin

        //! [connections]
        AimController {
            id: rightAim
            controller: XrController.RightController
            view: xrView
            enableVirtualMouse: true
            enableThumbstickMove: thumbCheckBox.checked

            onObjectPressed: (obj, pos, dir) => {
                gadgetBox.handlePress(obj, pos, dir)
            }
            onHoveredObjectChanged: {
                gadgetBox.handleHover(hoveredObject)
                hapticFeedback.handleHover(hoveredObject)
            }
            onMoved: (pos, dir) => {
                gadgetBox.handleMove(pos, dir)
            }
            onReleased: {
                gadgetBox.handleRelease()
            }
            onObjectGrabbed: (obj) => {
                if (!grabCheckBox.checked)
                    return
                const gadget = obj as XrGadget
                if (!gadget)
                    startGrab(obj)
                else if (gadget.grabbable)
                    startGrab(gadget.controlledObject)
            }
            Model {
                source: "#Cylinder"
                scale: "0.05, 0.1, 0.05"
                z: 5
                eulerRotation.x: 90
                materials: PrincipledMaterial {
                    baseColor: "black"
                }
            }
            xrCursor: cursor
        }
        //! [connections]

        XrCamera {
            id: camera
        }

        XrCursor {
            id: cursor
            cameraNode: camera
            size: cursorSlider.value
            function style() : int {
                if (rightAim.pickStatus === PickResult.Item)
                    return XrCursor.CursorStyle.Flat
                if (rightAim.pickStatus === PickResult.Null)
                    return XrCursor.CursorStyle.Hidden
                const gadget = rightAim.hoveredObject as XrGadget
                if (gadget)
                    return gadget.cursorStyle
                return sphereButton.checked ? XrCursor.CursorStyle.Sphere : XrCursor.CursorStyle.Flat
            }
            cursorStyle: style()
        }
    }

    GadgetBox {
        id: gadgetBox
    }

    Scene {
        z: -100
    }

    XrItem {
        y: height + 75
        x: -125
        z: -75
        width: 60
        height: 80
        eulerRotation.y: 30
        color: "transparent"
        contentItem: Rectangle {
            width: 300
            height: 400
            opacity: 0.99
            color: "#aafff7f0"
            radius: 20
            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 20
                CheckBox {
                    id: grabCheckBox
                    text: "Enable grab"
                    checked: true
                }
                CheckBox {
                    id: thumbCheckBox
                    text: "Thumbstick move"
                    checked: true
                    enabled: grabCheckBox.checked
                }
                Slider {
                    id: cursorSlider
                    from: 0.5
                    value: 2.0
                    to: 5
                }
                Text {
                    text: "Cursor size: " + cursorSlider.value.toFixed(1)
                }
                Frame {
                    ColumnLayout {
                        anchors.fill: parent
                        Text { text: "Cursor style for 3D objects" }
                        RadioButton { id: sphereButton; text: "Sphere"; checked: true }
                        RadioButton { text: "Flat" }
                    }
                    Layout.alignment: Qt.AlignBottom
                }
            }
        }
        XrItemHandle {
            visible: grabCheckBox.checked
        }
    }
}
