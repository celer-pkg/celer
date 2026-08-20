// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Controls
import ToDoList

StyleSettingsForm {

    styleSettingsButton.onClicked: (button) => {
        AppSettings.style = button.text
        infoLabel.visible = true
    }

    backButton.onClicked: StackView.view.pop()
}
