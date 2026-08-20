// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick.Window

Window {
    width: 640
    height: 480
    visible: true
    title: qsTr("Quick Loader Tester")
    Loader {
        id: loader
        anchors.fill: parent
    }
    MouseArea {
        anchors.fill: parent
        onClicked:  {
            console.debug("Click!");
            if (loader.source == "")
                loader.source = "MyComponent.qml";
            else
                loader.source = "";
        }
    }
}
