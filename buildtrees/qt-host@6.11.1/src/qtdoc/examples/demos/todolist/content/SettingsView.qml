// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Controls
import ToDoList

SettingsViewForm {

    tasksSettingsModel:[{
        "name": qsTr("Remove done tasks"),
        "checked": AppSettings.removeDoneTasks,
        "actionOnClicked": function() {
            AppSettings.removeDoneTasks = !AppSettings.removeDoneTasks
        }
    }]

    themeSettingsModel:[{
        "name": qsTr("Dark mode"),
        "checked": Constants.isDarkModeActive,
        "actionOnClicked": function() {
            if (AppSettings.theme == "Dark") {
                AppSettings.theme = "Light"
            } else {
                AppSettings.theme = "Dark"
            }
        }
    }]

    isThemeOptionAvailable: AppSettings.style !== "iOS"
                            && AppSettings.style !== "Basic"

    backButton.onClicked: StackView.view.pop()

    Component.onCompleted: {
        if (!isThemeOptionAvailable) {
            AppSettings.theme = Qt.binding( function() {
                return  Application.styleHints.colorScheme === Qt.Dark ? "Dark" : "Light"
            })
        }
    }
}
