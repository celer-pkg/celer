// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick

MessageForm {
    property string title
    property string message
    property var backPage

    signal closeForm(var backPage)

    button.onClicked: {
        closeForm(backPage)
    }

    Component.onCompleted: {
        messageText.text = message
        messageTitle.text = title
    }
}
