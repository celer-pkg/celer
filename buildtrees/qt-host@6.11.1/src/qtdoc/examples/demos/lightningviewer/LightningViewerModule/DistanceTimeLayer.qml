// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Layouts

Item {
    id: distanceTimeLayer
    readonly property color titleColor: "white"
    readonly property int titleFontWeight: 700
    readonly property int titleFontSize: 14
    readonly property color valueColor: "#2CDE85"
    readonly property int valueFontWeight: 400
    readonly property int valueFontSize: 14

    Rectangle {
        anchors.fill: parent
        color: "#262626"
        opacity: 0.7
        radius: 15
    }

    GridLayout {
        anchors.fill: parent
        anchors.margins: 10
        columns: 2
        rows: 3

        Text {
            Layout.alignment: Qt.AlignLeft
            color: distanceTimeLayer.titleColor
            font.pixelSize: distanceTimeLayer.titleFontSize
            font.weight: distanceTimeLayer.titleFontWeight
            text: qsTr("Distance:")
        }
        Text {
            Layout.alignment: Qt.AlignRight
            color: distanceTimeLayer.valueColor
            font.pixelSize: distanceTimeLayer.valueFontSize
            font.weight: distanceTimeLayer.valueFontWeight
            readonly property double distance: LightningController.lastStrikeDistance
            text: distance < 0 ? "--" : `${(distance/1000).toFixed(1)}km`
        }

        Text {
            Layout.alignment: Qt.AlignLeft
            color: distanceTimeLayer.titleColor
            font.pixelSize: distanceTimeLayer.titleFontSize
            font.weight: distanceTimeLayer.titleFontWeight
            text: qsTr("Direction:")
        }
        Text {
            Layout.alignment: Qt.AlignRight
            color: distanceTimeLayer.valueColor
            font.pixelSize: distanceTimeLayer.valueFontSize
            font.weight: distanceTimeLayer.valueFontWeight
            readonly property real direction: LightningController.lastStrikeDirection
            text: toText(direction.toFixed(0))

            function toText(direction_) {
                if (direction_ < 0)
                    return "--";
                direction_ = direction_ % 360;
                if (direction_ < 90)
                    return `${direction_}° NE`;
                else if (direction_ < 180)
                    return `${180 - direction_}° SE`;
                else if (direction_ < 270)
                    return `${direction_ - 180}° SW`;
                else
                    return `${360 - direction_}° NW`;
            }
        }

        Text {
            Layout.alignment: Qt.AlignLeft
            color: distanceTimeLayer.titleColor
            font.pixelSize: distanceTimeLayer.titleFontSize
            font.weight: distanceTimeLayer.titleFontWeight
            text: qsTr("Time:")
        }
        Text {
            Layout.alignment: Qt.AlignRight
            color: distanceTimeLayer.valueColor
            font.pixelSize: distanceTimeLayer.valueFontSize
            font.weight: distanceTimeLayer.valueFontWeight
            readonly property bool valid: LightningController.lastStrikeTime > 0
            readonly property real duration: time.now - LightningController.lastStrikeTime
            text: valid ? toText(duration) : "--"

            function toText(duration_) {
                if (duration_ > 0) {
                    const dh = Math.floor(duration_ / 3600);
                    const dm = Math.floor((duration_ % 3600) / 60);
                    const ds = Math.floor(duration_ % 60);
                    if (dh > 0)
                        return `${dh}h ` + qsTr("ago");
                    else if (dm > 0)
                        return `${dm}m ` + qsTr("ago");
                    else if (ds > 0)
                        return `${ds}s ` + qsTr("ago");
                }
                return qsTr("now");
            }
        }
    }

    Timer {
        id: time
        interval: 1000
        running: distanceTimeLayer.visible
        repeat: true
        property int now: (new Date()) / 1000
        onTriggered: now = (new Date()) / 1000;
        onRunningChanged: now = (new Date()) / 1000;
    }
}
