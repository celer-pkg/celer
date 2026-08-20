// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Shapes
import QtQuick.Effects
import QtQuick.Layouts

//! [0]
Window {
    id: window
//! [0]
    width: 360
    height: 640
    visible: true

    Shape {//Shape because Rectangle does not support diagonal gradient
        preferredRendererType: Shape.CurveRenderer

        ShapePath {
            strokeWidth: 0
            startX: 0; startY: 0

            PathLine { x: window.width; y: 0 }
            PathLine { x: window.width; y: window.height }
            PathLine { x: 0; y: window.height }
            fillGradient: LinearGradient {
                x1: 0; y1: window.height / 4
                x2: window.width; y2:  window.height / 4 * 3
                GradientStop { position: 0.0; color: "#2CDE85" }
                GradientStop { position: 1.0; color: "#9747FF" }
            }
        }
    }

    Rectangle {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        anchors.topMargin: parent.width / 2
        width: window.height > window.width * 1.5 ? window.height : window.width * 1.5
        height: width
        radius: width / 2
        color: "#000000"
        opacity: 0.15
    }

    Item {
        id: statesItem
        visible: false
        state: "loading"
        states: [
            State {
                name: "loading"
                PropertyChanges { main.opacity: 0 }
                PropertyChanges { wait.opacity: 1 }
            },
            State {
                name: "ready"
                PropertyChanges { main.opacity: 1 }
                PropertyChanges { wait.opacity: 0 }
            }
        ]
    }

//! [1]
    AppModel {
        id: appModel
        onReadyChanged: {
            if (appModel.ready)
                statesItem.state = "ready"
            else
                statesItem.state = "loading"
        }
    }
//! [1]
    Item {
        id: wait
        anchors.fill: parent

        Text {
            text: "Loading weather data..."
            anchors.centerIn: parent
            font.pointSize: 18
        }
    }

    Item {
        id: main
        anchors.fill: parent

        ColumnLayout {
            id: layout
            spacing: 4

            anchors {
                fill: parent
                topMargin: 6; bottomMargin: 6; leftMargin: 6; rightMargin: 6
            }

            Item {
                Layout.preferredHeight: cityButton.height
                Layout.fillWidth: true

                Rectangle {
                    id: cityButton
                    property int margins: 10
                    anchors.centerIn: parent
                    width: cityName.contentWidth + cityIcon.width + 3 * margins
                    height: cityName.contentHeight + 2 * margins
                    radius: 8
                    color: "#3F000000"
                    visible: false

                    Text {
                        id: cityName
                        text: (appModel.hasValidCity ? appModel.city : "Unknown location")
                              + (appModel.useGps ? " (GPS)" : "")
                        anchors.left: parent.left
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.margins: parent.margins
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        font.pixelSize: 24
                        color: "white"
                    }

                    Image {
                        id: cityIcon
                        source: "icons/waypoint.svg"
                        height: cityName.font.pixelSize
                        width: implicitWidth * height / implicitHeight
                        anchors.left: cityName.right
                        anchors.margins: parent.margins
                        anchors.verticalCenter: cityName.verticalCenter
                        anchors.verticalCenterOffset: 2
                        visible: false
                    }
                    MultiEffect {
                        source: cityIcon
                        anchors.fill: cityIcon
                        brightness: 1 // make icons white, remove for dark icons
                    }

                }

                MultiEffect {
                    source: cityButton
                    anchors.fill: cityButton
                    shadowEnabled: true
                    shadowBlur: 0.5
                    shadowHorizontalOffset: 0
                    shadowVerticalOffset: 2
                }

                MouseArea {
                    anchors.fill: cityButton
                    onClicked: {
                        if (appModel.useGps) {
                            appModel.useGps = false
                            appModel.city = "Brisbane"
                        } else {
                            switch (appModel.city) {
                            case "Brisbane":
                                appModel.city = "Oslo"
                                break
                            case "Oslo":
                                appModel.city = "Helsinki"
                                break
                            case "Helsinki":
                                appModel.city = "New York"
                                break
                            case "New York":
                                appModel.useGps = true
                                break
                            }
                        }
                    }
                }
            }

        //! [3]
            BigForecastIcon {
                id: current
                Layout.fillWidth: true
                Layout.fillHeight: true

                weatherIcon: (appModel.hasValidWeather
                              ? appModel.weather.weatherIcon
                              : "sunny")
        //! [3]
                topText: (appModel.hasValidWeather
                          ? appModel.weather.temperature
                          : "??")
                bottomText: (appModel.hasValidWeather
                             ? appModel.weather.weatherDescription
                             : "No weather data")
        //! [4]
            }
        //! [4]

            Item {
                implicitWidth: iconRow.implicitWidth + 40
                implicitHeight: iconRow.implicitHeight + 40

                Layout.fillWidth: true

                Rectangle {
                    id: forcastFrame

                    anchors.fill: parent
                    color: "#3F000000"
                    radius: 40

                    Row {
                        id: iconRow
                        anchors.centerIn: parent

                        property int daysCount: appModel.forecast.length
                        property real iconWidth: (daysCount > 0) ? ((forcastFrame.width - 20) / daysCount)
                                                                 : forcastFrame.width

                        Repeater {
                            model: appModel.forecast
                            ForecastIcon {
                                required property string dayOfWeek
                                required property string temperature
                                required property string weatherIcon
                                id: forecast1
                                width: iconRow.iconWidth
                                topText: (appModel.hasValidWeather ? dayOfWeek : "??")
                                bottomText: (appModel.hasValidWeather ? temperature : ("??" + "/??"))
                                middleIcon: (appModel.hasValidWeather ? weatherIcon : "sunny")
                            }
                        }
                    }
                    visible: false
                }

                MultiEffect {
                    source: forcastFrame
                    anchors.fill: forcastFrame
                    shadowEnabled: true
                    shadowBlur: 0.5
                    shadowHorizontalOffset: 0
                    shadowVerticalOffset: 4
                    shadowOpacity: 0.6
                }
            }
        }
    }
//! [2]
}
//! [2]
