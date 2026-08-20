// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Controls

//![0]
ApplicationWindow {
//![0]
    flags: Qt.ExpandedClientAreaHint | Qt.NoTitleBarBackgroundHint
    visible: true
//![1]
    // Remove automatic safe area padding
    topPadding: 0

    Flickable {
        // Inset content by safe area margin
        topMargin: SafeArea.margins.top
//![1]
        anchors.fill: parent

        Rectangle {
            id: rect
            width: 1000
            height: 1000
            gradient: Gradient.DustyGrass
        }

        contentWidth: rect.width
        contentHeight: rect.height

         onTopMarginChanged: {
             // Workaround for QTBUG-131478
             contentY = -topMargin
         }
//![2]
    }
}
//![2]
