// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Controls
import ToDoList

Item {
    id: root

    implicitHeight: 80
    implicitWidth: 96

    property alias hourTextField: hour.textField
    property alias minTextField: minutes.textField
    property alias hourText: hour.text
    property alias minText: minutes.text

    property string amPm: amPmButton.isAm ? "AM" : "PM"

    Row {
        id: row
        spacing: 10

        TimeTextField {
            id: hour

            text: "12"
            isFocused: !minutes.isFocused
            regexValidator: /((0[1-9])|(1[0-2]))/
        }

        Label {
            text: ":"
            font.pixelSize: 57
            color: Constants.secondaryColor
        }

        TimeTextField {
            id: minutes

            text: "00"
            regexValidator: /([0-5][0-9])/
        }

        AmPmButton {
            id: amPmButton
        }
    }
}
