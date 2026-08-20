// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
import QtQuick
import QtQuick3D
import QtQuick3D.Xr

Node {
    id: root
    property color color: "#ddaa88"
    required property int touchId
    required property int hand
    required property XrView view

    property alias touchPosition: handController.scenePos
    property bool touchActive: false

    XrController {
        id: handController
        controller: root.hand

        property vector3d scenePos: view.xrOrigin.mapPositionToScene(pokePosition)

        onScenePosChanged: {
            const touchOffset = view.processTouch(scenePos, root.touchId)
            handModel.position = touchOffset

            //! [touch handling]
            const touchState = view.touchpointState(root.touchId)
            const gadget = touchState.model as XrGadget
            if (gadget) {
                gadget.handleTouch(touchState.uvPosition, root.touchId, touchState.pressed)
            }
            //! [touch handling]
        }
    }

    XrHandModel {
        id: handModel
        hand: root.hand
        materials: PrincipledMaterial {
            baseColor: root.color
            roughness: 0.5
        }
    }
}
