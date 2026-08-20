// Copyright (C) 2017 The Qt Company Ltd.
// Copyright (C) 2016 basysKom GmbH, author Bernd Lamecker <bernd.lamecker@basyskom.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    property alias userName: userName
    property alias loginButton: loginButton
    property alias nameInUseError: nameInUseError

    ColumnLayout {
        anchors.right: parent.right
        anchors.left: parent.left
        anchors.top: parent.top

        TextField {
            id: userName
            focus: true
            onEditingFinished: loginButton.clicked()
            Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
        }

        Button {
            id: loginButton
            text: "Login"
            Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
        }

        Label {
            id: nameInUseError
            text: "Name already in use"
            Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
        }
    }
}
