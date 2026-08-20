// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
import QtQuick
import QtQuick3D
import QtQuick3D.Physics
import QtQuick3D.Xr

Node {
    id: gripper
    property alias hand: action.hand
    property Torch torchBody: null

    XrInputAction {
        id: action
        actionId: [XrInputAction.SqueezeValue, XrInputAction.SqueezePressed]
        onValueChanged: {
            if (gripper.torchBody) {
                if (value > 0.5) {
                    gripper.torchBody.attachedTo = gripper
                }else if (gripper.torchBody.attachedTo) {
                    gripper.torchBody.attachedTo = null
                    gripper.torchBody = null
                }
            }
        }
    }

    TriggerBody {
        scale: Qt.vector3d(0.03, 0.03, 0.03)
        collisionShapes: [SphereShape{}]

        onBodyEntered: (body)=>{
                           if (body.objectName === "Torch" &&
                               !gripper.torchBody) {
                               gripper.torchBody = body
                           }
                       }
        onBodyExited: (body)=>{
                          if (body.objectName === "Torch" &&
                              gripper.torchBody &&
                              !gripper.torchBody.attachedTo) {
                              gripper.torchBody = null
                          }
                      }
        Model {
            source: "#Sphere"
            materials: PrincipledMaterial {
                baseColor: gripper.torchBody ? "green" : "white"
            }
        }
    }
}

