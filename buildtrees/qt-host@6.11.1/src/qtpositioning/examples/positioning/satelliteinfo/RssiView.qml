// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Layouts
import QtQuick.Shapes

import SatelliteInformation

Rectangle {
    id: root

    required property SatelliteModel satellitesModel
    required property color inViewColor
    required property color inUseColor

    color: Theme.darkBackgroundColor

    // A rectangle which is rounded only from the top side
    component SemiRoundedRectangle : Shape {
        id: roundRect
        property int radius: 4
        property color color: "white"
        visible: roundRect.height > 0
        ShapePath {
            id: path
            readonly property int radius: Math.min(roundRect.radius,
                                                   roundRect.width / 2,
                                                   roundRect.height / 2)

            strokeWidth: 1
            strokeColor: roundRect.color
            fillColor: roundRect.color

            startX: 0
            startY: roundRect.height
            PathLine {
                x: 0
                y: path.radius
            }
            PathArc {
                x: path.radius
                y: 0
                radiusX: path.radius
                radiusY: path.radius
                useLargeArc: false
            }
            PathLine {
                x: roundRect.width - path.radius
                y: 0
            }
            PathArc {
                x: roundRect.width
                y: path.radius
                radiusX: path.radius
                radiusY: path.radius
                useLargeArc: false
            }
            PathLine {
                x: roundRect.width
                y: roundRect.height
            }
            PathLine {
                x: 0
                y: roundRect.height
            }
        }
    }

    component DashedLine : Shape {
        id: line
        property color color: "white"
        ShapePath {
            strokeWidth: line.height
            strokeColor: line.color
            strokeStyle: ShapePath.DashLine
            dashPattern: [4, 4]
            fillColor: "transparent"
            startX: 0
            startY: 0
            PathLine {
                x: line.width
                y: 0
            }
        }
    }

    component SatelliteBox : Rectangle {
        id: satBox

        required property var modelData

        implicitWidth: satRect.border.width * 2 + satRect.anchors.margins * 2
                       + satRectLayout.anchors.leftMargin
                       + satRectLayout.anchors.rightMargin
                       + satIdText.width + satImg.width
        implicitHeight: satRect.border.width * 2 + satRect.anchors.margins * 2
                        + satRectLayout.anchors.bottomMargin + rssiText.height
                        + Math.max(satIdText.height + satRectLayout.anchors.topMargin,
                                   satImg.height)

        color: "transparent"

        Rectangle {
            id: satRect
            anchors {
                fill: parent
                margins: Theme.defaultSpacing
            }
            border {
                width: 1
                color: Theme.satelliteItemBorderColor
            }
            radius: 8
            color: "transparent"
            ColumnLayout {
                id: satRectLayout
                anchors {
                    fill: parent
                    topMargin: Theme.defaultSpacing
                    bottomMargin: Theme.defaultSpacing
                    leftMargin: Theme.defaultSpacing * 2
                    rightMargin: Theme.defaultSpacing * 2
                }
                spacing: 0
                Text {
                    id: satIdText
                    text: qsTr("Sat #") + satBox.modelData.id
                    color: Theme.satelliteItemMainColor
                    font.pixelSize: Theme.mediumFontSize
                    font.weight: Theme.fontDefaultWeight
                    Layout.alignment: Qt.AlignLeft
                }
                Text {
                    id: rssiText
                    text: satBox.modelData.rssi >= 0 ? satBox.modelData.rssi + qsTr(" dBm")
                                                     : qsTr("N/A")
                    color: Theme.satelliteItemSecondaryColor
                    font.pixelSize: Theme.smallFontSize
                    font.weight: Theme.fontLightWeight
                    Layout.alignment: Qt.AlignCenter
                }
            }
        }
        Image {
            id: satImg
            anchors {
                top: parent.top
                right: satRect.right
                rightMargin: Theme.defaultSpacing
            }
            source: "icons/satellite_small.png"
        }
    }

    // Hidden SatelliteBox instance to determine a proper height for the ListView
    SatelliteBox {
        id: hiddenSatelliteBox
        visible: false
        modelData: ({"id": 999, "rssi": 999})
    }

    Rectangle {
        id: satListRect
        readonly property int spacing: Theme.defaultSpacing * 2
        anchors.top: parent.top
        color: Theme.backgroundColor
        height: hiddenSatelliteBox.height + spacing * 2
        width: parent.width

        ListView {
            id: satView
            anchors {
                fill: parent
                topMargin: satListRect.spacing
                bottomMargin: satListRect.spacing
            }
            model: root.satellitesModel
            delegate: SatelliteBox {
                id: satDelegate
            }
            orientation: ListView.Horizontal
        }
    }

    Rectangle {
        id: separator
        anchors {
            top: satListRect.bottom
        }
        color: Theme.separatorColor
        height: 1
        width: root.width
    }

    ColumnLayout {
        anchors{
            top: separator.bottom
            bottom: parent.bottom
            left: parent.left
            right: parent.right
            margins: Theme.defaultSpacing
        }

        spacing: Theme.defaultSpacing

        LegendBox {
            id: legend
            Layout.alignment: Qt.AlignRight
        }

        Item {
            id: rect
            readonly property int maxVisibleLevel: 70

            Layout.fillHeight: true
            Layout.fillWidth: true

            Rectangle {
                id: axis
                readonly property int scaleWidth: 2
                readonly property int scaleSpacing: 2
                readonly property int offset: maxRssiLabel.width + scaleWidth + scaleSpacing

                color: "transparent"
                height: rect.height - maxRssiLabel.height
                width: rect.width

                Text {
                    id: maxRssiLabel
                    anchors {
                        top: axis.top
                        left: axis.left
                    }
                    text: "70"
                    color: Theme.textSecondaryColor
                    font.pixelSize: Theme.smallFontSize
                    font.weight: Theme.fontLightWeight
                }
                Rectangle {
                    id: greenRect
                    width: axis.scaleWidth
                    height: axis.height * 20 / rect.maxVisibleLevel
                    anchors {
                        top: axis.top
                        left: maxRssiLabel.right
                        leftMargin: axis.scaleSpacing
                    }
                    color: "#489d22"
                }
                DashedLine {
                    anchors {
                        top: greenRect.top
                        left: greenRect.right
                        right: axis.right
                    }
                    height: 1
                    color: "#338e8e8e"
                }
                Text {
                    anchors {
                        bottom: greenRect.bottom
                        right: maxRssiLabel.right
                    }
                    text: "50"
                    color: Theme.textSecondaryColor
                    font.pixelSize: Theme.smallFontSize
                    font.weight: Theme.fontLightWeight
                }
                Rectangle {
                    id: lightgreenRect
                    width: axis.scaleWidth
                    height: axis.height * 10 / rect.maxVisibleLevel
                    anchors {
                        top: greenRect.bottom
                        left: greenRect.left
                    }
                    color: "#2cde85"
                }
                DashedLine {
                    anchors {
                        top: lightgreenRect.top
                        left: lightgreenRect.right
                        right: axis.right
                    }
                    height: 1
                    color: "#338e8e8e"
                }
                Text {
                    anchors {
                        bottom: lightgreenRect.bottom
                        right: maxRssiLabel.right
                    }
                    text: "40"
                    color: Theme.textSecondaryColor
                    font.pixelSize: Theme.smallFontSize
                    font.weight: Theme.fontLightWeight
                }
                Rectangle {
                    id: yellowRect
                    width: axis.scaleWidth
                    height: axis.height * 10 / rect.maxVisibleLevel
                    anchors {
                        top: lightgreenRect.bottom
                        left: lightgreenRect.left
                    }
                    color: "#ffcc00"
                }
                DashedLine {
                    anchors {
                        top: yellowRect.top
                        left: yellowRect.right
                        right: axis.right
                    }
                    height: 1
                    color: "#338e8e8e"
                }
                Text {
                    anchors {
                        bottom: yellowRect.bottom
                        right: maxRssiLabel.right
                    }
                    text: "30"
                    color: Theme.textSecondaryColor
                    font.pixelSize: Theme.smallFontSize
                    font.weight: Theme.fontLightWeight
                }
                Rectangle {
                    id: goldRect
                    width: axis.scaleWidth
                    height: axis.height * 10 / rect.maxVisibleLevel
                    anchors {
                        top: yellowRect.bottom
                        left: yellowRect.left
                    }
                    color: "#ffb800"
                }
                DashedLine {
                    anchors {
                        top: goldRect.top
                        left: goldRect.right
                        right: axis.right
                    }
                    height: 1
                    color: "#338e8e8e"
                }
                Text {
                    anchors {
                        bottom: goldRect.bottom
                        right: maxRssiLabel.right
                    }
                    text: "20"
                    color: Theme.textSecondaryColor
                    font.pixelSize: Theme.smallFontSize
                    font.weight: Theme.fontLightWeight
                }
                Rectangle {
                    id: orangeRect
                    width: axis.scaleWidth
                    height: axis.height * 10 / rect.maxVisibleLevel
                    anchors {
                        top: goldRect.bottom
                        left: goldRect.left
                    }
                    color: "#f9a70e"
                }
                DashedLine {
                    anchors {
                        top: orangeRect.top
                        left: orangeRect.right
                        right: axis.right
                    }
                    height: 1
                    color: "#338e8e8e"
                }
                Text {
                    anchors {
                        bottom: orangeRect.bottom
                        right: maxRssiLabel.right
                    }
                    text: "10"
                    color: Theme.textSecondaryColor
                    font.pixelSize: Theme.smallFontSize
                    font.weight: Theme.fontLightWeight
                }
                Rectangle {
                    id: redRect
                    width: axis.scaleWidth
                    height: axis.height * 10 / rect.maxVisibleLevel
                    anchors {
                        top: orangeRect.bottom
                        left: orangeRect.left
                    }
                    color: "#c50000"
                }
                DashedLine {
                    anchors {
                        top: redRect.top
                        left: redRect.right
                        right: axis.right
                    }
                    height: 1
                    color: "#338e8e8e"
                }
                Text {
                    anchors {
                        bottom: redRect.bottom
                        right: maxRssiLabel.right
                    }
                    text: "0"
                    color: Theme.textSecondaryColor
                    font.pixelSize: Theme.smallFontSize
                    font.weight: Theme.fontLightWeight
                }
                Rectangle {
                    anchors {
                        top: redRect.bottom
                        left: redRect.left
                        right: axis.right
                    }
                    height: 1
                    color: "#8e8e8e"
                }
            }

            Row {
                id: view

                anchors {
                    fill: rect
                    leftMargin: axis.offset + view.spacing
                }

                property int rows: repeater.model.size
                property int singleWidth: view.width / rows - spacing

                spacing: Theme.defaultSpacing

                //! [0]
                Repeater {
                    id: repeater
                    model: root.satellitesModel
                    delegate: Rectangle {
                        required property var modelData
                        height: rect.height
                        width: view.singleWidth
                        color: "transparent"
                        SemiRoundedRectangle {
                            anchors.bottom: satId.top
                            width: parent.width
                            height: (parent.height - satId.height)
                                    * Math.min(parent.modelData.rssi, rect.maxVisibleLevel)
                                    / rect.maxVisibleLevel
                            color: parent.modelData.inUse ? root.inUseColor : root.inViewColor
                        }
                        Text {
                            id: satId
                            anchors.horizontalCenter: parent.horizontalCenter
                            anchors.bottom: parent.bottom
                            text: parent.modelData.id
                            color: Theme.textSecondaryColor
                            font.pixelSize: Theme.smallFontSize
                            font.weight: Theme.fontLightWeight
                        }
                    }
                }
                //! [0]
            }
        }
    }
}
