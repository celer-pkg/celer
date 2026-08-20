// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick3D

Window {
    id: window
    width: 1280
    height: 720
    visible: true
    color: "black"

    View3D {
        id: view
        anchors.fill:parent
        renderMode: View3D.Underlay

        environment: SceneEnvironment {
            backgroundMode: SceneEnvironment.Color
            clearColor: "black"
        }

        Doors {
            id: door
            targetItem: object2d
        }

        //! [picking]
        TapHandler {
            gesturePolicy: TapHandler.WithinBounds
            // qmllint disable signal-handler-parameters
            onTapped: {
                var result = view.pick(point.position.x, point.position.y);
                if (result.objectHit) {
                    console.log("pick dist", result.distance, "hit", result.objectHit,
                                "scene pos", result.scenePosition, "uv", result.uvPosition);
                    var pickedDoor = result.objectHit;
                    if (pickedDoor.state === "")
                        pickedDoor.state = "opened";
                    else
                        pickedDoor.state = "";

                }
            }
            // qmllint enable signal-handler-parameters
        }
        //! [picking]
    }

    //! [2d layer]
    Rectangle {
        id: object2d
        width: 500
        height: 700
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.margins: 10

        CorkBoards { }

        clip: true
        layer.enabled: true
    }
    //! [2d layer]
}
