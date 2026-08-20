// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls.Universal
import QtQuick.Layouts
import AerospaceHubExample

Item {
    id: turbineimage

    //! [statuspane]
    // Masking the turbine image of the background image
    ImageItem {
        anchors.fill: parent
    }

    // Notification
    NotificationItem {
        anchors.top: turbineimage.top
        anchors.horizontalCenter: turbineimage.horizontalCenter
        anchors.topMargin: GlobalSettings.doublespacing
        width: turbineimage.width / 1.25
    }

    // Status
    StatusItem {
        anchors.centerIn: turbineimage
        width: turbineimage.width / 1.5
        height: turbineimage.width / 1.75
    }

    // Turbine name
    Label {
        //! [statuspane]
        anchors.bottom: turbineimage.bottom
        anchors.right: turbineimage.right
        anchors.bottomMargin: GlobalSettings.defaultspacing * 1.5
        anchors.rightMargin: GlobalSettings.doublespacing
        text: "Turbine " + GlobalSettings.turbineid
        color: "#EFEFEF"
        font.pixelSize: GlobalSettings.fontsize54px
        font.bold: true
    }
}
