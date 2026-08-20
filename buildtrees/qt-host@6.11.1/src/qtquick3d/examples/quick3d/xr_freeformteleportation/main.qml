// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick3D
import QtQuick3D.Helpers
import QtQuick3D.Xr

import xr_shared

XrView {
    id: xrView
    XrErrorDialog { id: err }
    onInitializeFailed: (errorString) => err.run("XR freeform teleportation", errorString)
    referenceSpace: XrView.ReferenceSpaceLocalFloor

    environment: ExtendedSceneEnvironment {
        id: extendedSceneEnvironment
        clearColor: "black"
        backgroundMode: SceneEnvironment.Color
        antialiasingMode: SceneEnvironment.MSAA
        antialiasingQuality: SceneEnvironment.High
        lightProbe: Texture {
            textureData: ProceduralSkyTextureData {}
        }
        exposure: teleporter.screenVisibility
        specularAAEnabled: true
    }

    xrOrigin: XrOrigin {
        id: xrOrigin

        camera: XrCamera {
            id: trackingCamera
        }

        Component.onCompleted: {
            teleporter.teleportTo(Qt.vector3d(1000, 200, -300))
        }

        XrController {
            id: xrRightController
            controller: XrController.RightController
            poseSpace: XrController.AimPose

            XrInputAction {
                id: thumbstickX
                controller: XrInputAction.RightController
                actionId: [XrInputAction.ThumbstickX]
            }

            XrInputAction {
                id: thumbstickY
                controller: XrInputAction.RightController
                actionId: [XrInputAction.ThumbstickY]
            }

            XrInputAction {
                id: trackpadX
                controller: XrInputAction.RightController
                actionId: [XrInputAction.TrackpadX]
            }

            XrInputAction {
                id: trackpadY
                controller: XrInputAction.RightController
                actionId: [XrInputAction.TrackpadY]
            }

            XrInputAction {
                id: trackpadPressed
                controller: XrInputAction.RightController
                actionId: [XrInputAction.TrackpadPressed]
            }

            XrInputAction {
                id: indexFingerPinch
                property bool active: false
                controller: XrInputAction.RightController
                actionId: [XrInputAction.IndexFingerPinch]
                onTriggered: teleporter.toggleTeleport()
            }

            // Rotate left/right by pinching the middle fingers
            XrInputAction {
                id: leftMiddleFingerPinch
                controller: XrInputAction.LeftController
                actionId: [XrInputAction.MiddleFingerPinch]
                onTriggered: teleporter.rotateLeft()
            }

            XrInputAction {
                id: rightMiddleFingerPinch
                controller: XrInputAction.RightController
                actionId: [XrInputAction.MiddleFingerPinch]
                onTriggered: teleporter.rotateRight()
            }

            property real xValue: trackpadPressed.pressed ? trackpadX.value : thumbstickX.value
            property real yValue: trackpadPressed.pressed ? trackpadY.value : thumbstickY.value


            Model {
                source: "#Cube"
                scale: Qt.vector3d(0.1, 0.1, 0.1)
                materials: PrincipledMaterial {
                    baseColor: "white"
                }
            }
        }
    }

    //! [FreeformTeleporter component]
    FreeformTeleporter {
        id: teleporter

        view: xrView
        originNode: xrOrigin
        cameraNode: xrOrigin.camera
        beamHandle: xrRightController

        rotationTriggerValue: xrRightController.xValue
        teleportTriggerValue: xrRightController.yValue

        onDoTeleportation: (cameraOriginPosition)=> {
                               xrOrigin.position = cameraOriginPosition
                           }
        onDoRotation: (cameraOriginRotation, cameraOriginPosition)=> {
                          xrOrigin.rotation = cameraOriginRotation
                          xrOrigin.position = cameraOriginPosition
                      }
    }
    //! [FreeformTeleporter component]

    Building {}
}


