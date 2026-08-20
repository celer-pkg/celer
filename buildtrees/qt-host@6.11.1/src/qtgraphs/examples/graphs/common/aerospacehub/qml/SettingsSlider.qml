// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls.Universal

Slider {
    id: slider
    snapMode: Slider.SnapAlways

    background: Rectangle {
        x: slider.leftPadding * 2
        y: slider.topPadding + slider.availableHeight / 2 - height / 2
        implicitWidth: GlobalSettings.settingssliderwidth
        width: slider.availableWidth
        height: GlobalSettings.settingssliderheight
        radius: height / 2
        color: "#EFEFEF"

        Rectangle {
            width: slider.handle.x
            height: parent.height
            color: "#27BF73"
            radius: height / 2
        }
    }

    handle: Rectangle {
        x: slider.leftPadding * 2 + slider.visualPosition * (slider.availableWidth - width)
        y: slider.topPadding + slider.availableHeight / 2 - height / 2
        implicitWidth: GlobalSettings.settingssliderhandleheight
        implicitHeight: GlobalSettings.settingssliderhandleheight
        radius: GlobalSettings.settingssliderhandleheight / 2
        color: "#27BF73"
    }
}
