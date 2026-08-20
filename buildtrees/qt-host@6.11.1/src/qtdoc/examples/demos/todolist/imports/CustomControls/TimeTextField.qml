// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Controls.Fusion as Fusion
import ToDoList

Item {
    id: root

    implicitWidth: 96
    implicitHeight: 80

    property alias textField: textField
    property alias text: textField.text
    property alias regexValidator: regexValidator.regularExpression

    property bool isFocused: textField.focus

    //TextField from Fusion is used to ensure that the item looks the same in every style.
    Fusion.TextField {
        id: textField

        font.pixelSize: 60
        implicitWidth: 96
        implicitHeight: 80
        color: Constants.secondaryColor

        horizontalAlignment: TextInput.AlignHCenter
        verticalAlignment: TextInput.AlignVCenter

        inputMethodHints: Qt.ImhDigitsOnly

        background: Rectangle {
            color: root.isFocused ? "#41CD52" : "transparent"
            border.color: Constants.secondaryColor
            radius: 8
        }

        validator: RegularExpressionValidator {
            id: regexValidator
        }
    }
}
