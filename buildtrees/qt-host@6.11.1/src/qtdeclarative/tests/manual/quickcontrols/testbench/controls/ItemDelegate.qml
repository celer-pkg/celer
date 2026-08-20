// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick.Controls

QtObject {
    property var supportedStates: [
        [],
        ["disabled"],
        ["pressed"],
        ["highlighted"],
        ["highlighted", "pressed"],
        ["icon"],
        ["transparent-icon"],
        ["icon", "disabled"],
        ["icon", "pressed"],
        ["icon", "highlighted"],
        ["icon", "highlighted", "pressed"],
        ["icon", "mirrored"]
    ]

    property Component component: ItemDelegate {
        text: "ItemDelegate"
        enabled: !is("disabled")
        checkable: is("checkable")
        // Only set it if it's pressed, or the non-pressed examples will have no press effects
        down: is("pressed") ? true : undefined
        highlighted: is("highlighted")
        icon.source: anyStateContains("icon") ? Utils.iconUrl : ""
        icon.color: is("transparent-icon") ? "transparent" : undefined
        focusPolicy: Qt.StrongFocus
    }

    property Component exampleComponent: ListView {
        implicitWidth: 200
        implicitHeight: 200
        clip: true
        model: 20
        delegate: ItemDelegate {
            width: ListView.view.width
            text: "ItemDelegate"
            focusPolicy: Qt.StrongFocus
        }
    }
}
