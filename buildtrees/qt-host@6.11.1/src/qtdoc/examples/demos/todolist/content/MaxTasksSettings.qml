// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Controls
import ToDoList

MaxTasksSettingsForm {
    backButton.onClicked: StackView.view.pop()
    maxTasksSpinbox.onValueChanged: AppSettings.maxTasksCount = maxTasksSpinbox.value
}
