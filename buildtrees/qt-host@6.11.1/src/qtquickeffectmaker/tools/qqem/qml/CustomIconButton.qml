// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick.Controls

Item {
    id: rootItem

    property string icon
    property string toggledIcon
    property string description
    property bool toggleButton: false
    property bool toggled: false

    signal clicked

    Image {
        id: iconImage
        property real effectiveOpacity: 1.0
        anchors.centerIn: parent
        height: parent.height * 0.8
        width: parent.width * 0.8
        fillMode: Image.PreserveAspectFit
        source: (toggleButton && toggled) ? rootItem.toggledIcon : rootItem.icon
        mipmap: true
        opacity: rootItem.enabled ? effectiveOpacity : 0.3
    }
    MouseArea {
        id: iconButtomMouseArea
        anchors.fill: parent
        hoverEnabled: true
        onClicked: {
            if (!toggleButton)
                clickAnimation.restart();
            rootItem.clicked();
        }
    }
    SequentialAnimation {
        id: clickAnimation
        NumberAnimation {
            target: iconImage
            property: "effectiveOpacity"
            to: 0.5
            easing.type: Easing.InOutQuad
            duration: 200
        }
        NumberAnimation {
            target: iconImage
            property: "effectiveOpacity"
            to: 1.0
            easing.type: Easing.InOutQuad
            duration: 200
        }
    }
    ToolTip {
        parent: rootItem
        visible: iconButtomMouseArea.containsMouse && description
        delay: 1000
        text: description
    }
}
