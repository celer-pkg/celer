// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Window
import QtPositioning
import QtLocation

Window {
    width: 700
    height: 550
    visible: true

    property variant topLeftEurope: QtPositioning.coordinate(61, 0.0)
    property variant bottomRightEurope: QtPositioning.coordinate(51.0, 14.0)
    property variant viewOfEurope:
            QtPositioning.rectangle(topLeftEurope, bottomRightEurope)

    property variant berlin: QtPositioning.coordinate(52.5175, 13.384)
    property variant oslo: QtPositioning.coordinate(59.9154, 10.7425)
    property variant london: QtPositioning.coordinate(51.5, 0.1275)

    Map {
        id: mapOfEurope
        anchors.centerIn: parent;
        anchors.fill: parent
        plugin: Plugin {
            name: "osm" // "maplibregl", "esri", ...
        }

        Plane {
            id: qmlPlane
            pilotName: "QML"
            coordinate: oslo2Berlin.position

            SequentialAnimation {
                id: qmlPlaneAnimation
                property real rotationDirection : 0;
                NumberAnimation {
                    target: qmlPlane; property: "bearing"; duration: 1000
                    easing.type: Easing.InOutQuad
                    to: qmlPlaneAnimation.rotationDirection
                }
                //! [QmlPlane1]
                CoordinateAnimation {
                    id: coordinateAnimation; duration: 5000
                    target: oslo2Berlin; property: "position"
                    easing.type: Easing.InOutQuad
                }
                //! [QmlPlane1]

                onStopped: {
                    if (coordinateAnimation.to === berlin)
                        qmlPlane.showMessage(qsTr("Hello Berlin!"))
                    else if (coordinateAnimation.to === oslo)
                        qmlPlane.showMessage(qsTr("Hello Oslo!"))
                }
                onStarted: {
                    if (coordinateAnimation.from === oslo)
                        qmlPlane.showMessage(qsTr("See you Oslo!"))
                    else if (coordinateAnimation.from === berlin)
                        qmlPlane.showMessage(qsTr("See you Berlin!"))
                }
            }

            //! [QmlPlane2]
            TapHandler {
                onTapped: {
                    if (qmlPlaneAnimation.running) {
                        console.log("Plane still in the air.");
                        return;
                    }

                    if (oslo2Berlin.position === berlin) {
                        coordinateAnimation.from = berlin;
                        coordinateAnimation.to = oslo;
                    } else if (oslo2Berlin.position === oslo) {
                        coordinateAnimation.from = oslo;
                        coordinateAnimation.to = berlin;
                    }

                    qmlPlaneAnimation.rotationDirection = oslo2Berlin.position.azimuthTo(coordinateAnimation.to)
                    qmlPlaneAnimation.start()
                }
            }
            //! [QmlPlane2]
            Component.onCompleted: {
                oslo2Berlin.position = oslo;
            }
        }

        //! [CppPlane1]
        Plane {
            id: cppPlane
            pilotName: "C++"
            coordinate: berlin2London.position

            TapHandler {
                onTapped: {
                    if (cppPlaneAnimation.running || berlin2London.isFlying()) {
                        console.log("Plane still in the air.");
                        return;
                    }

                    berlin2London.swapDestinations();
                    cppPlaneAnimation.rotationDirection = berlin2London.position.azimuthTo(berlin2London.to)
                    cppPlaneAnimation.start();
                    cppPlane.departed();
                }
            }
        //! [CppPlane1]
            //! [CppPlane3]
            SequentialAnimation {
                id: cppPlaneAnimation
                property real rotationDirection : 0;
                NumberAnimation {
                    target: cppPlane; property: "bearing"; duration: 1000
                    easing.type: Easing.InOutQuad
                    to: cppPlaneAnimation.rotationDirection
                }
                ScriptAction { script: berlin2London.startFlight() }
            }
            //! [CppPlane3]

            Component.onCompleted: {
                berlin2London.position = berlin;
                berlin2London.to = london;
                berlin2London.from = berlin;
                berlin2London.arrived.connect(arrived)
            }

            function arrived(){
                if (berlin2London.to === berlin)
                    cppPlane.showMessage(qsTr("Hello Berlin!"))
                else if (berlin2London.to === london)
                    cppPlane.showMessage(qsTr("Hello London!"))
            }

            function departed(){
                if (berlin2London.from === berlin)
                    cppPlane.showMessage(qsTr("See you Berlin!"))
                else if (berlin2London.from === london)
                    cppPlane.showMessage(qsTr("See you London!"))
            }
        //! [CppPlane2]
        }
        //! [CppPlane2]

        visibleRegion: viewOfEurope
    }

    Rectangle {
        id: infoBox
        anchors.centerIn: parent
        color: "white"
        border.width: 1
        width: text.width * 1.3
        height: text.height * 1.3
        radius: 5
        Text {
            id: text
            anchors.centerIn: parent
            text: qsTr("Hit the plane to start the flight!")
        }

        Timer {
            interval: 5000; running: true; repeat: false;
            onTriggered: fadeOut.start()
        }

        NumberAnimation {
            id: fadeOut; target: infoBox;
            property: "opacity";
            to: 0.0;
            duration: 200
            easing.type: Easing.InOutQuad
        }
    }
}
