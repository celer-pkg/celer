// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick.Layouts
import QtQuick.Window
import QtQuick.Controls

ApplicationWindow {
    id: window
    width: 400
    height: 400
    objectName: "Rectangle"

    property alias drawer: drawer
    property alias dialog: dialog
    property alias outsideButton1: outsideButton1
    property alias outsideButton2: outsideButton2
    property alias drawerButton1: drawerButton1
    property alias drawerButton2: drawerButton2
    property alias dialogButton1: dialogButton1
    property alias dialogButton2: dialogButton2

    Drawer {
        id: drawer
        width: 0.5 * window.width
        height: window.height
        visible: true
        focus: false
        modal: false
        ColumnLayout {
            anchors.fill: parent
            Button {
                id: drawerButton1
                Layout.alignment: Qt.AlignHCenter
                text: "Button 1"
            }
            Button {
                id: drawerButton2
                Layout.alignment: Qt.AlignHCenter
                text: "Button 2"
                Dialog {
                    id: dialog
                    objectName: "Dialog"
                    popupType: Popup.Item
                    focus: false
                    y: drawerButton2.height
                    visible: true
                    ColumnLayout {
                        Button {
                            id: dialogButton1
                            text: "Button 1"
                        }
                        Button {
                            id: dialogButton2
                            text: "Button 2"
                        }
                    }
                }
            }
        }
    }

    ColumnLayout {
        id: buttonColumn
        x: drawer.width
        Button {
            id: outsideButton1
            text: "Button1"
            focus: true
        }
        Button {
            id: outsideButton2
            text: "Button2"
        }
    }
}
