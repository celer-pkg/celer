// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Shapes
import SatelliteInformation

Rectangle {
    id: root

    // Multisample this item to get rid of aliasing in our custom shapes.
    layer.enabled: true
    layer.samples: 4
    // Sample text to calculate sky view size properly
    Text {
        id: sampleText
        visible: false
        text: "270°"
        font.pixelSize: Theme.smallFontSize
        font.weight: Theme.fontLightWeight
    }

    required property SatelliteModel satellitesModel
    required property color inViewColor
    required property color inUseColor


    readonly property real fullRadius: width < height ? width / 2 - sampleText.width * 1.1
                                                      : height / 2 - sampleText.height * 1.1

    readonly property real maxUsedRadius: 0.9 * fullRadius
    readonly property real radiusStep: maxUsedRadius / 9
    readonly property int centerX: width / 2
    readonly property int centerY: height / 2

    color: Theme.darkBackgroundColor

    LegendBox {
        id: legend
        anchors {
            top: root.top
            right: root.right
            margins: Theme.defaultSpacing
        }
    }

    // Shape for the SkyView
    Shape {
        id: rootShape
        readonly property color mainColor: Theme.textMainColor
        readonly property color dashedColor: Qt.rgba(mainColor.r,
                                                     mainColor.g,
                                                     mainColor.b,
                                                     0.5)

        // Circles
        component ElevationCircle: ShapePath {
            id: circle

            property color color: "white"
            property real radius: 0
            property bool dashed: false

            strokeColor: circle.color
            fillColor: "transparent"
            strokeStyle: circle.dashed ? ShapePath.DashLine : ShapePath.SolidLine
            dashPattern: [4, 4]
            PathAngleArc {
                centerX: root.centerX
                centerY: root.centerY
                radiusX: circle.radius
                radiusY: circle.radius
                startAngle: 0
                sweepAngle: 360
            }
        }

        ElevationCircle {
            radius: root.fullRadius
            color: Theme.greenColor
        }
        ElevationCircle {
            radius: root.radiusStep * 9
            color: rootShape.dashedColor
            dashed: true
        }
        ElevationCircle {
            radius: root.radiusStep * 7
            color: rootShape.dashedColor
            dashed: true
        }
        ElevationCircle {
            radius: root.radiusStep * 5
            color: rootShape.dashedColor
            dashed: true
        }
        ElevationCircle {
            radius: root.radiusStep * 3
            color: rootShape.dashedColor
            dashed: true
        }
        ElevationCircle {
            radius: root.radiusStep
            color: rootShape.mainColor
        }

        // Lines
        component Line: ShapePath {
            id: line
            readonly property color green: Theme.greenColor
            strokeColor: Qt.rgba(green.r, green.g, green.b, 0.5)
            property real angle
            property real radianAngle: line.angle / 180 * Math.PI
            startX: root.centerX + root.fullRadius * Math.cos(radianAngle)
            startY: root.centerY + root.fullRadius * Math.sin(radianAngle)
            PathLine {
                x: root.centerX + root.fullRadius * Math.cos(line.radianAngle + Math.PI)
                y: root.centerY + root.fullRadius * Math.sin(line.radianAngle + Math.PI)
            }
        }

        Line {
            angle: 0
        }
        Line {
            angle: 30
        }
        Line {
            angle: 60
        }
        Line {
            angle: 90
        }
        Line {
            angle: 120
        }
        Line {
            angle: 150
        }

        // Azimuth captions
        component AzimuthCaption: Text {
            id: textElement
            property real angle
            property real offsetX: 0
            property real offsetY: 0
            // Subtract 90, because 0 should be on top, not on the right
            property real radianAngle: (angle - 90) / 180 * Math.PI
            // end of line coordinates
            property int lineX: root.centerX + root.fullRadius * Math.cos(radianAngle)
            property int lineY: root.centerY + root.fullRadius * Math.sin(radianAngle)
            x: lineX + offsetX
            y: lineY + offsetY
            text: angle.toFixed(0) + '°'
            color: Theme.textMainColor
            font.pixelSize: Theme.smallFontSize
            font.weight: Theme.fontLightWeight
        }

        AzimuthCaption {
            text: 'N'
            angle: 0
            offsetY: -height
            offsetX: -width / 2
        }
        AzimuthCaption {
            angle: 30
            offsetY: -height
        }
        AzimuthCaption {
            angle: 60
            offsetX: width * 0.5
            offsetY: -height / 2
        }
        AzimuthCaption {
            text: 'E'
            angle: 90
            offsetX: width * 0.5
            offsetY: -height / 2
        }
        AzimuthCaption {
            angle: 120
        }
        AzimuthCaption {
            angle: 150
        }
        AzimuthCaption {
            text: 'S'
            angle: 180
            offsetX: -width / 2
        }
        AzimuthCaption {
            angle: 210
            offsetX: -width / 2
            offsetY: height / 2
        }
        AzimuthCaption {
            angle: 240
            offsetX: -width
        }
        AzimuthCaption {
            text: 'W'
            angle: 270
            offsetX: -width * 1.5
            offsetY: -height / 2
        }
        AzimuthCaption {
            angle: 300
            offsetX: -width
            offsetY: -height
        }
        AzimuthCaption {
            angle: 330
            offsetX: -width
            offsetY: -height
        }

        // Elevation captions
        component ElevationCaption: Text {
            property real step
            x: root.centerX - implicitWidth / 2
            y: root.centerY - (step * root.radiusStep + implicitHeight)
            color: Theme.textMainColor
            font.pixelSize: Theme.smallFontSize
            font.weight: Theme.fontLightWeight
        }

        ElevationCaption {
            text: "80°"
            step: 1
        }
        ElevationCaption {
            text: "60°"
            step: 3
        }
        ElevationCaption {
            text: "40°"
            step: 5
        }
        ElevationCaption {
            text: "20°"
            step: 7
        }
        ElevationCaption {
            text: "0°"
            step: 9
        }
    }

    // Actual satellite positions
    Item {
        id: satelliteItemView
        anchors.fill: parent

        property string selectedName: ""
        property var selectedData: ({"name": "", "azimuth": 0, "elevation": 0})
        // selected{X,Y} holds the center of the selected circle
        property int selectedX: -1
        property int selectedY: -1

        Repeater {
            model: root.satellitesModel
            delegate: Rectangle {
                id: satItem

                required property var modelData
                property bool selected: satelliteItemView.selectedName === modelData.name

                readonly property color normalColor: modelData.inUse ? root.inUseColor
                                                                     : root.inViewColor
                readonly property color selectedColor: Qt.rgba(normalColor.r,
                                                               normalColor.g,
                                                               normalColor.b,
                                                               0.5)
                readonly property color centerColor: selected ? Theme.whiteColor
                                                              : Theme.blackColor
                readonly property int normalWidth: 10
                readonly property int centerWidth: 6
                readonly property int outerWidth: selected ? normalWidth + 10 : normalWidth

                width: outerWidth
                height: width
                radius: width / 2
                color: selectedColor
                visible: modelData.azimuth > -1 && modelData.elevation > -1
                property real angle: (modelData.azimuth - 90) / 180 * Math.PI
                property real distance: root.maxUsedRadius * (90 - modelData.elevation) / 90
                x: root.centerX + distance * Math.cos(angle) - width / 2
                y: root.centerY + distance * Math.sin(angle) - height / 2
                Rectangle {
                    anchors.centerIn: parent
                    width: satItem.normalWidth
                    height: width
                    radius: width / 2
                    color: satItem.normalColor

                    Rectangle {
                        anchors.centerIn: parent
                        width: satItem.centerWidth
                        height: width
                        radius: width / 2
                        color: satItem.centerColor
                    }
                }
                MouseArea {
                    anchors {
                        fill: parent
                        margins: -20 // so that it's easier to select the item
                    }
                    onClicked: {
                        satelliteItemView.selectedName = satItem.modelData.name
                        satelliteItemView.selectedData = satItem.modelData
                        satelliteItemView.selectedX = satItem.x + satItem.width / 2
                        satelliteItemView.selectedY = satItem.y + satItem.height / 2
                    }
                }

                onXChanged: {
                    if (satItem.selected)
                        satelliteItemView.selectedX = satItem.x + satItem.width / 2
                }
                onYChanged: {
                    if (satItem.selected)
                        satelliteItemView.selectedY = satItem.y + satItem.height / 2
                }

                Component.onCompleted: {
                    if (satItem.selected) {
                        satelliteItemView.selectedData = satItem.modelData
                        satelliteItemView.selectedX = satItem.x + satItem.width / 2
                        satelliteItemView.selectedY = satItem.y + satItem.height / 2
                    }
                }
            }
        }

        Rectangle {
            id: blurRect
            anchors.fill: parent
            color: Theme.backgroundBlurColor
            visible: satelliteItemView.selectedName !== ""
            MouseArea {
                anchors.fill: parent
                onClicked: satelliteItemView.selectedName = ""
            }
        }

        Rectangle {
            id: satInfoRect
            visible: satelliteItemView.selectedName !== ""
            implicitWidth: Math.max(satName.width, satAzimuth.width, satElevation.width)
                           + satInfoCol.anchors.leftMargin + satInfoCol.anchors.rightMargin
            implicitHeight: satName.height + satAzimuth.height + satElevation.height
                            + satInfoCol.anchors.topMargin + satInfoCol.anchors.bottomMargin
                            + 2 * satInfoCol.spacing
            // We need to make sure that the popup fits into the view.
            // This depends on the position of the actual satellite.
            // We try to show the popup top-right from the satellite,
            // but adjust if it's too close to the window border.
            // Keep in mind that (0, 0) is the top left corner.
            readonly property int xOffset: 15
            readonly property int yOffset: 25
            x: satelliteItemView.selectedX + satInfoRect.xOffset + satInfoRect.width
               < satelliteItemView.width
               ? satelliteItemView.selectedX + satInfoRect.xOffset
               : satelliteItemView.selectedX - satInfoRect.xOffset - satInfoRect.width
            y: satelliteItemView.selectedY - satInfoRect.yOffset - satInfoRect.height > 0
               ? satelliteItemView.selectedY - satInfoRect.yOffset - satInfoRect.height
               : satelliteItemView.selectedY + satInfoRect.yOffset
            color: Theme.whiteColor
            border {
                color: Theme.lightGrayColor
                width: 1
            }
            radius: 8
            MouseArea {
                anchors.fill: parent
                onClicked: {} // suppress mouse area of blurRect
            }
            Column {
                id: satInfoCol
                anchors {
                    fill: parent
                    topMargin: Theme.defaultSpacing
                    bottomMargin: Theme.defaultSpacing
                    leftMargin: Theme.defaultSpacing * 2
                    rightMargin: Theme.defaultSpacing * 2
                }
                spacing: 0
                Text {
                    id: satName
                    text: qsTr("Sat ") + satelliteItemView.selectedData.name
                    font.pixelSize: Theme.smallFontSize
                    font.weight: Theme.fontDefaultWeight
                    color: Theme.darkGrayColor
                }
                Text {
                    id: satAzimuth
                    text: qsTr("Azimuth:   ")
                          + satelliteItemView.selectedData.azimuth.toFixed(0) + "°"
                    font.pixelSize: Theme.smallFontSize
                    font.weight: Theme.fontLightWeight
                    color: Theme.grayColor
                }
                Text {
                    id: satElevation
                    text: qsTr("Elevation: ")
                          + satelliteItemView.selectedData.elevation.toFixed(0) + "°"
                    font.pixelSize: Theme.smallFontSize
                    font.weight: Theme.fontLightWeight
                    color: Theme.grayColor
                }
            }
        }

        Shape {
            id: connectionLine
            visible: satelliteItemView.selectedName !== ""
            ShapePath {
                strokeColor: Theme.whiteColor
                strokeWidth: 1
                startX: satelliteItemView.selectedX
                startY: satelliteItemView.selectedY
                PathLine {
                    x: satInfoRect.x > satelliteItemView.selectedX
                       ? satInfoRect.x : satInfoRect.x + satInfoRect.width
                    y: satInfoRect.y > satelliteItemView.selectedY
                       ? satInfoRect.y : satInfoRect.y + satInfoRect.height
                }
            }
        }
    }
}
