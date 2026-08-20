// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick.Controls

ApplicationWindow {
    id: window
    width: Screen.width
    height: Screen.height

    property alias mainMenu: popupWindow
    property alias subMenu: subMenu1

    Menu {
        id: popupWindow
        x: window.width - implicitWidth
        title: "Menu1"

        MenuItem {
            text: "MenuItem1"
        }

        MenuItem {
            text: "MenuItem2"
        }

        Menu {
            id: subMenu1
            title: "SubMenu1"

            MenuItem {
                text: "SubMenuItem1"
            }

            MenuItem {
                text: "SubMenuItem2"
            }
        }
    }
}
