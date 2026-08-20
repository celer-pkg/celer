// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick.Controls

Item {
    id: rootItem

    property bool toggled: false
    property real toggledAnimated: toggled
    property alias animatedToggle: toggleBehavior.enabled
    property string textOn: ""
    property string textOff: ""
    property string description

    readonly property real imageScale: (288 / 128)

    height: 64
    width: height * imageScale

    Behavior on toggledAnimated {
        id: toggleBehavior
        NumberAnimation {
            duration: 400
            easing.type: Easing.InOutQuad
        }
    }

    Image {
        id: backgroundImage
        anchors.fill: parent
        source: "images/toggle_background.png"
        mipmap: true
        opacity: 0.5
    }
    Image {
        anchors.fill: parent
        source: "images/toggle_m1.png"
        mipmap: true
        opacity: 1.0 - rootItem.toggledAnimated
    }
    Image {
        anchors.fill: parent
        source: "images/toggle_m2.png"
        mipmap: true
        opacity: rootItem.toggledAnimated
    }
    Text {
        anchors.verticalCenter: parent.verticalCenter
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.horizontalCenterOffset: -5
        text: rootItem.textOff
        font.pixelSize: 14
        font.bold: true
        color: mainView.foregroundColor2
        opacity: rootItem.toggledAnimated
    }
    Text {
        anchors.verticalCenter: parent.verticalCenter
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.horizontalCenterOffset: 5
        text: rootItem.textOn
        font.pixelSize: 14
        color: mainView.foregroundColor2
        font.bold: true
        opacity: 1.0 - rootItem.toggledAnimated
    }
    Rectangle {
        readonly property real marg: parent.height * 0.30
        anchors.verticalCenter: parent.verticalCenter
        height: parent.height - (2 * marg)
        width: height * 0.4
        x: marg + toggledAnimated * (rootItem.width - width - 2 * marg)
        radius: width * 0.5
        color: mainView.foregroundColor2
        border.width: 1
        border.color: mainView.foregroundColor1
        rotation: toggledAnimated * 180
    }
    MouseArea {
        id: customToggleMouseArea
        anchors.fill: parent
        hoverEnabled: true
        onClicked: {
            rootItem.toggled = !rootItem.toggled;
        }
    }
    ToolTip {
        parent: rootItem
        visible: customToggleMouseArea.containsMouse && description
        delay: 1000
        text: description
    }
}
