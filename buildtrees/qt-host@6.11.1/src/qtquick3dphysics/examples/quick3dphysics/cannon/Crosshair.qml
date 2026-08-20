// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
import QtQuick
import QtQuick3D
import QtQuick3D.Physics
import QtQuick3D.Helpers
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: crossHair
    width: 31
    height: width
    readonly property real halfSize: width * 0.5 - 0.5
    Rectangle {
        width: crossHair.halfSize
        height: 1
        anchors.left: parent.left
        anchors.verticalCenter: parent.verticalCenter
        color: "black"
    }
    Rectangle {
        width: crossHair.halfSize
        height: 1
        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter
        color: "black"
    }
    Rectangle {
        width: 1
        height: crossHair.halfSize
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        color: "black"
    }
    Rectangle {
        width: 1
        height: crossHair.halfSize
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        color: "black"
    }
}
