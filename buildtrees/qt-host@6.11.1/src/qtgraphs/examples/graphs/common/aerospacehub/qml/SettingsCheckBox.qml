// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls.Universal

CheckBox {
    id: checkbox

    background: Rectangle {
        y: checkbox.topPadding / 2
        implicitWidth: GlobalSettings.settingscheckboxwidth
        height: GlobalSettings.settingsitemheight
        radius: height / 2
        color: checked ? "#00A1FF" : "lightgray"
    }

    indicator: Rectangle {
        x: checked ? checkbox.background.width - height - y : y
        y: (checkbox.topPadding + checkbox.background.height - height) / 2
        implicitWidth: GlobalSettings.settingscheckboxheight
        implicitHeight: GlobalSettings.settingscheckboxheight
        radius: GlobalSettings.settingscheckboxheight / 2
        color: "#FCFCFC"
    }
}
