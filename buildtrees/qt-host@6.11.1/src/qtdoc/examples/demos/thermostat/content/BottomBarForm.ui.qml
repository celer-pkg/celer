// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause


/*
This is a UI file (.ui.qml) that is intended to be edited in Qt Design Studio only.
It is supposed to be strictly declarative and only uses a subset of QML. If you edit
this file manually, you might introduce QML code that is not supported by Qt Design Studio.
Check out https://doc.qt.io/qtcreator/creator-quick-ui-forms.html for details on .ui.qml files.
*/
pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls.Basic
import Thermostat

TabBar {
    id: root

    required property list<Room> roomsList
    required property ListModel menuOptions
    required property StackView stackView

    contentHeight: 56

    background: Rectangle {
        color: Constants.accentColor
        border.color: Constants.accentTextColor
        radius: 12
    }

    Repeater {
        id: repeater

        model: root.menuOptions
        delegate: TabButton {
            id: menuItem

            required property string name
            required property string view
            required property string iconSource
            readonly property bool active: Constants.currentView === menuItem.view

            background: Rectangle {
                color: "transparent"
            }

            width: menuItem.view === "SettingsView" ? 0 : undefined;
            height: menuItem.view === "SettingsView" ? 0 : undefined;

            icon.width: 24
            icon.height: 24
            icon.source: "images/" + menuItem.iconSource
            icon.color: menuItem.active ? "#2CDE85" : Constants.accentTextColor

            Connections {
                function onClicked() {
                    if (menuItem.view !== "SettingsView"
                            && menuItem.view !== Constants.currentView) {
                        root.stackView.replace(menuItem.view + ".qml", {
                                              "roomsList": root.roomsList
                                          }, StackView.Immediate)
                        Constants.currentView = menuItem.view
                    }
                }
            }
        }
    }
}
