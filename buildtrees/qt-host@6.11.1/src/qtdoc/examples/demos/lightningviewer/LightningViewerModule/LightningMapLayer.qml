// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

pragma ComponentBehavior: Bound

import QtQuick
import QtLocation
import QtPositioning

MapItemView {
    id: lightningMapLayer
    model: LightningController.model
    delegate: MapQuickItem {
        required property double longitude
        required property double latitude
        width: sourceItem.width
        height: sourceItem.height
        coordinate: QtPositioning.coordinate(latitude, longitude, 0);
        sourceItem: Item {
            height: lightningMapLayer.strikeIconSize
            width: height
            anchors.centerIn: parent
            Rectangle {
                id: notifCircle
                radius: 0
                width: radius
                height: radius
                anchors.centerIn: parent
                color: "#00414A"
            }
            Image {
                x: -width/2
                y: -height/2
                anchors.fill: parent
                source: "icons/lightning-strike.svg"
            }
            ParallelAnimation {
                id: notifCircleAnim
                running: true
                NumberAnimation {
                    target: notifCircle
                    property: "radius"
                    loops: 1
                    duration: lightningMapLayer.strikeAnimDuration
                    easing.type: Easing.OutCubic
                    from: 0
                    to: lightningMapLayer.strikeCircleNotifRadius
                }
                NumberAnimation {
                    target: notifCircle
                    property: "opacity"
                    loops: 1
                    duration: lightningMapLayer.strikeAnimDuration
                    easing.type: Easing.Linear
                    from: 1
                    to: 0
                }
                onStopped: notifCircle.visible = false
            }
        }
    }

    readonly property int strikeIconSize: 20
    readonly property int strikeCircleNotifRadius: 4 * strikeIconSize
    readonly property int strikeAnimDuration: 1000
}
