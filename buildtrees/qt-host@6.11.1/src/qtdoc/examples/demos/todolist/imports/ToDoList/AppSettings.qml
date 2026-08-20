// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

pragma Singleton

import QtQuick
import QtCore

Settings {
    id: settings

    property string style
    property string theme: Application.styleHints.colorScheme === Qt.Dark ? "Dark" : "Light"
    property int maxTasksCount: 20
    property int fontSize: 16
    property bool removeDoneTasks: false
}
