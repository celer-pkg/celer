// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick3D
import QtQml

Window {
    id: window
    visible: true
    x: 100
    y: 100
    width: 600
    height: 600
    title: qsTr("Deleting last View3D of window test")

    property View3D view: null
    property Component comp: null

    Item {
        id: rootItem
        anchors.fill: parent
    }

    Timer {
        running: true
        triggeredOnStart: true
        interval: 2000
        repeat: true
        onTriggered: {
            if (window.view) {
                console.log("Deleting view");
                window.view.destroy();
            } else {
                console.log("Creating view");
                if (!window.comp)
                    window.comp = Qt.createComponent("view3DComponent.qml");
                if (window.comp.status === Component.Ready)
                    window.view = comp.createObject(rootItem);
                else
                    console.log("failed to create view3DComponent")
            }
        }
    }
}
