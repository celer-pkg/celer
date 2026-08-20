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
import QtQuick.Controls
import ToDoList

Page {
    id: root

    property var builtInStyles
    property var themeSettingsModel
    property var tasksSettingsModel
    property bool isThemeOptionAvailable

    property alias backButton: navBar.backButton

    header: NavBar {
        id: navBar

        titleText: qsTr("Settings")
        previousPageTitle: qsTr("Home")
    }

    ListModel {
        id: settings

        ListElement {
            setting: qsTr("Style")
            page: "Style"
            iconSource: "images/Style_Icon.svg"
        }
        ListElement {
            setting: qsTr("Theme")
            page: "Theme"
            iconSource: "images/Theme_Icon.svg"
        }
        ListElement {
            setting: qsTr("Remove completed tasks")
            page: "Tasks"
            iconSource: "images/Remove_Done_Icon.svg"
        }
        ListElement {
            setting: qsTr("Maximum number of tasks")
            page: "MaxTasks"
            iconSource: "images/Tasks_Icon.svg"
        }
        ListElement {
            setting: qsTr("Font Size")
            page: "FontSize"
            iconSource: "images/Font_Size_Icon.svg"
        }
    }

    ListView {
        id: listView
        anchors.fill: parent
        model: settings
        clip: true

        delegate: ItemDelegate {
            id: settingsItem

            required property string setting
            required property string page
            required property string iconSource

            width: listView.width
            text: settingsItem.setting
            font.pixelSize: AppSettings.fontSize
            icon.source: settingsItem.iconSource
            icon.color: Constants.secondaryColor

            Image {
                source: !Constants.isDarkModeActive ? "images/Right_Arrow_Icon_Dark.svg" : "images/Right_Arrow_Icon.svg"
                width: 20
                height: 20
                anchors.right: parent.right
                anchors.rightMargin: 10
                fillMode: Image.PreserveAspectFit
                anchors.verticalCenter: parent.verticalCenter
            }

            Connections {
                function onClicked() {
                    let stackView = (root.Window.window as App).globalNavigationStackView
                    if (settingsItem.page === "Style") {
                        stackView.push(settingsItem.page + "Settings.qml", {
                                           "builtInStyles": root.builtInStyles
                                       })
                    } else if (settingsItem.page === "Theme") {
                        stackView.push("OtherSettings.qml", {
                                           "otherSettingsModel": root.themeSettingsModel,
                                           "titleText": qsTr("Theme")
                                       })
                    } else if (settingsItem.page === "Tasks") {
                        stackView.push("OtherSettings.qml", {
                                           "otherSettingsModel": root.tasksSettingsModel,
                                           "titleText": qsTr("Tasks")
                                       })
                    } else {
                        stackView.push(settingsItem.page + "Settings.qml")
                    }
                }
            }
        }
    }

    //Prepare the model to include only the setting options available in current style.
    Connections {
        target: Component
        function onCompleted() {
            for (let i = 0; i < settings.count; i++) {
                if (settings.get(i).page === "Theme"
                        && !root.isThemeOptionAvailable) {
                    settings.remove(i)
                    return
                }
            }
        }
    }
}
