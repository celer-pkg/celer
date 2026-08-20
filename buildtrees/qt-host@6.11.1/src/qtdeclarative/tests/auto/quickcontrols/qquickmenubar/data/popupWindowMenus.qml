// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick.Controls

ApplicationWindow {
    id: root
    width: 1080
    height: 720

    property Menu subMenu: submenu

    signal newEmitted
    signal openEmitted
    signal sub1Emitted
    signal sub2Emitted
    signal cutEmitted
    signal copyEmitted
    signal pasteEmitted
    signal aboutEmitted

    menuBar: MenuBar {
        focus: true
        Menu {
            title: qsTr("&File")
            popupType: Popup.Window
            Action {
                text: qsTr("&New")
                onTriggered: root.newEmitted()
            }
            MenuItem {
                text: qsTr("&Open")
                onClicked: root.openEmitted()
            }
            Menu {
                id: submenu
                title: qsTr("Sub menu")
                popupType: Popup.Window
                Action {
                    text: qsTr("Sub 1")
                    onTriggered: root.sub1Emitted()
                }
                Action {
                    text: qsTr("Sub 2")
                    onTriggered: root.sub2Emitted()
                }
            }
        }
        Menu {
            title: qsTr("&Edit")
            popupType: Popup.Window
            Action {
                text: qsTr("Cut")
                shortcut: StandardKey.Cut
                onTriggered: root.cutEmitted()
            }
            MenuItem {
                text: qsTr("Copy")
                action: Action {
                    shortcut: StandardKey.Copy
                }
                onClicked: root.copyEmitted()
            }
            Action {
                text: qsTr("Paste")
                shortcut: StandardKey.Paste
                onTriggered: root.pasteEmitted()
            }
        }
        MenuBarItem {
            menu: Menu {
                title: qsTr("&Help")
                popupType: Popup.Window
                Action {
                    text: qsTr("&About")
                    onTriggered: root.aboutEmitted
                }
            }
        }
    }
}

