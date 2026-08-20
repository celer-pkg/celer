// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
import QtQuick
import QtQuick.Controls.Basic
//! [Set application window size]
ApplicationWindow {
    visible: true
    width: 1000
    height: 600
    title: qsTr("Coffee")

    background: Rectangle {
        color: Colors.currentTheme.background
    }

//! [Set application window size]
    ApplicationFlow {
        width: parent.width
        height: parent.height
    }

    Binding {
        target: Config
        property: "mode"
        value: (Screen.height > Screen.width) ? "portrait" : "landscape"
    }
}
