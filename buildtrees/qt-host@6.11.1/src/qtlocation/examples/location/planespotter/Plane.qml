// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtLocation
import QtQuick.Shapes


//! [PlaneMapQuick1]
// Plane.qml
MapQuickItem {
    id: plane
    property string pilotName;
    property int bearing: 0;

    anchorPoint.x: image.width/2
    anchorPoint.y: image.height/2

    sourceItem: Item {
        //...
//! [PlaneMapQuick1]
        width: childrenRect.width
        height: childrenRect.height
        Image {
            id: image
            rotation: bearing
            source: "airplane.svg"
        }
        Shape {
            id: bubble
            anchors.bottom: image.top
            anchors.margins: 3
            opacity: 0.8
            preferredRendererType: Shape.CurveRenderer
            ShapePath {
                strokeWidth: 0
                strokeColor: "#00414A"
                fillGradient: LinearGradient {
                    x1: 0; y1: 0
                    x2: 10 + text.width; y2: 10 + text.height
                    GradientStop { position: 0; color: "#00414A" }
                    GradientStop { position: 1; color: "#0C1C1F" }
                }
                startX: 5; startY: 0
                PathLine { x: 5 + text.width + 6; y: 0 }
                PathArc { x: 10 + text.width + 6; y: 5; radiusX: 5; radiusY: 5}
                PathLine { x: 10 + text.width + 6; y: 5 + text.height + 6 }
                PathArc { x: 5 + text.width + 6; y: 10 + text.height + 6 ; radiusX: 5; radiusY: 5}
                // arrow down
                PathLine { x: 10 + text.width / 2 + 3; y: 10 + text.height + 6 }
                PathLine { x: 5 + text.width / 2 + 3; y: 15 + text.height + 6 }
                PathLine { x: 0 + text.width / 2 + 3; y: 10 + text.height + 6 }
                // end arrow down
                PathLine { x: 5; y: 10 + text.height + 6 }
                PathArc { x: 0; y: 5 + text.height + 6 ; radiusX: 5; radiusY: 5}
                PathLine { x: 0; y: 5 }
                PathArc { x: 5; y: 0 ; radiusX: 5; radiusY: 5}
            }
            Text {
                id: text
                color: "white"
                anchors.top: parent.top
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.margins: 8
                text: pilotName
                font.bold: true
            }
        }

        Rectangle {
            id: message
            anchors.left: image.right
            anchors.margins: 3
            opacity: 0.0
            radius: 5
            border.width: 0
            color: "#E6E6E6"
            width: banner.width + 30
            height: banner.height + 20
            Text {
                id: banner
                color: "#00414A"
                anchors.centerIn: parent
                font.bold: true
            }

            SequentialAnimation {
                id: playMessage
                running: false
                NumberAnimation { target: message;
                    property: "opacity";
                    to: 1;
                    duration: 200
                    easing.type: Easing.InOutQuad
                }
                PauseAnimation  { duration: 3000 }
                NumberAnimation { target: message;
                    property: "opacity";
                    to: 0.0;
                    duration: 200}
            }
        }
    }
    function showMessage(message) {
        banner.text = message
        playMessage.start()
//! [PlaneMapQuick2]
    }
}
//! [PlaneMapQuick2]
