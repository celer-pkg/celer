// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

/*
This is a UI file (.ui.qml) that is intended to be edited in Qt Design Studio only.
It is supposed to be strictly declarative and only uses a subset of QML. If you edit
this file manually, you might introduce QML code that is not supported by Qt Design Studio.
Check out https://doc.qt.io/qtcreator/creator-quick-ui-forms.html for details on .ui.qml files.
*/
import QtQuick
import QtQuick.Controls

Item {
    property string text
    property alias checked: toggleIndicator.checked
    readonly property alias hasFocus: toggleIndicator.activeFocus
    implicitWidth: toggleText.width + toggleIndicator.width
    implicitHeight: 50

    Label {
        id: toggleText
        text: parent.text
        anchors.verticalCenter: toggleIndicator.verticalCenter
    }
    Switch {
        id: toggleIndicator
        anchors.left: toggleText.right
        anchors.rightMargin: 8
    }
}
