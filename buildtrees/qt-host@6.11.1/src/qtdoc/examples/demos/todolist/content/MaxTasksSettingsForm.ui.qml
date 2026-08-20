// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause


/*
This is a UI file (.ui.qml) that is intended to be edited in Qt Design Studio only.
It is supposed to be strictly declarative and only uses a subset of QML. If you edit
this file manually, you might introduce QML code that is not supported by Qt Design Studio.
Check out https://doc.qt.io/qtcreator/creator-quick-ui-forms.html for details on .ui.qml files.
*/
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import ToDoList

Page {
    id: root

    property alias backButton: navBar.backButton
    property alias maxTasksSpinbox: maxTasksSpinbox

    topPadding: 12

    header: NavBar {
        id: navBar

        titleText: qsTr("Max Tasks")
        previousPageTitle: qsTr("Settings")
        acceptButton.visible: false
    }

    ColumnLayout {
        width: parent.width

        Label {
            id: maxTasksText

            color: Constants.secondaryColor
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.Wrap
            text: qsTr("Choose the maximum amount of tasks.")
            font.pixelSize: AppSettings.fontSize

            Layout.fillWidth: true
        }

        SpinBox {
            id: maxTasksSpinbox
            editable: true
            from: 5
            value: AppSettings.maxTasksCount
            to: 50

            Layout.alignment: Qt.AlignHCenter
            Layout.topMargin: 10
        }
    }
}
