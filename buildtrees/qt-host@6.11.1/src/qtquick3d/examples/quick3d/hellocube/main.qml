// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick3D

Window {
    id: window
    width: 640
    height: 640
    visible: true
    color: "black"

    Item {
        id: qt_logo
        width: 230
        height: 230
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.margins: 10

        //! [offscreenSurface]
        layer.enabled: true
        //! [offscreenSurface]

        Rectangle {
            anchors.fill: parent
            color: "black"
            //! [2d]
            Image {
                anchors.fill: parent
                source: "qt_logo.png"
            }
            Text {
                anchors.bottom: parent.bottom
                anchors.left: parent.left
                color: "white"
                font.pixelSize: 17
                text: qsTr("The Future is Written with Qt")
            }
            //! [2d]

            //! [2danimation]
            transform: Rotation {
                id: rotation
                origin.x: qt_logo.width / 2
                origin.y: qt_logo.height / 2
                axis { x: 1; y: 0; z: 0 }
            }

            PropertyAnimation {
                id: flip1
                target: rotation
                property: "angle"
                duration: 600
                to: 180
                from: 0
            }
            PropertyAnimation {
                id: flip2
                target: rotation
                property: "angle"
                duration: 600
                to: 360
                from: 180
            }
            //! [2danimation]
        }
    }

    View3D {
        id: view
        anchors.fill: parent

        PerspectiveCamera {
            position: Qt.vector3d(0, 200, 300)
            eulerRotation.x: -30
        }

        DirectionalLight {
            eulerRotation.x: -30
        }

        Model {
            //! [3dcube]
            id: cube
            source: "#Cube"
            materials: PrincipledMaterial {
                baseColorMap: Texture {
                    sourceItem: qt_logo
                }
            }
            eulerRotation.y: 90
            //! [3dcube]

            Vector3dAnimation on eulerRotation {
                loops: Animation.Infinite
                duration: 5000
                from: Qt.vector3d(0, 0, 0)
                to: Qt.vector3d(360, 0, 360)
            }
        }
    }

    MouseArea {
        id: mouseArea
        anchors.fill: qt_logo

        Text {
            id: clickme
            anchors.top: mouseArea.top
            anchors.horizontalCenter: mouseArea.horizontalCenter
            font.pixelSize: 17
            text: "Click me!"

            SequentialAnimation on color {
                loops: Animation.Infinite
                ColorAnimation { duration: 400; from: "white"; to: "black" }
                ColorAnimation { duration: 400; from: "black"; to: "white" }
            }

            states: [
                State {
                    name: "flipped";
                    AnchorChanges {
                        target: clickme
                        anchors.top: undefined
                        // QTBUG-101364
                        anchors.bottom: mouseArea.bottom // qmllint disable incompatible-type
                    }
                }
            ]
        }

        onClicked: {
            // do nothing while animating
            if (flip1.running || flip2.running)
                return;

            if (clickme.state === "flipped") {
                clickme.state = "";
                flip2.start();
            } else {
                clickme.state = "flipped";
                flip1.start();
            }
        }
    }
}
