// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Layouts

Rectangle {
    id: root

    radius: 8
    color: Theme.legendBackgroundColor

    implicitWidth: Math.max(inUseTxt.implicitWidth, inViewTxt.implicitWidth)
                   + rootLayout.columnSpacing
                   + Math.max(inUseIcon.width, inViewIcon.width)
                   + rootLayout.anchors.leftMargin + rootLayout.anchors.rightMargin
    implicitHeight: inUseTxt.implicitHeight + inViewTxt.implicitHeight
                    + rootLayout.rowSpacing
                    + rootLayout.anchors.topMargin + rootLayout.anchors.bottomMargin

    GridLayout {
        id: rootLayout
        columns: 2
        anchors {
            fill: parent
            topMargin: Theme.defaultSpacing
            bottomMargin: Theme.defaultSpacing
            leftMargin: 2 * Theme.defaultSpacing
            rightMargin: 2 * Theme.defaultSpacing
        }
        columnSpacing: Theme.defaultSpacing
        rowSpacing: 0

        Text {
            id: inUseTxt
            text: qsTr("In Use")
            color: Theme.textMainColor
            font.pixelSize: Theme.smallFontSize
            font.weight: Theme.fontLightWeight
            Layout.alignment: Qt.AlignRight
        }
        Rectangle {
            id: inUseIcon
            implicitHeight: inUseTxt.font.pixelSize
            implicitWidth: implicitHeight
            radius: height / 2
            color: Theme.inUseColor
        }

        Text {
            id: inViewTxt
            text: qsTr("In View")
            color: Theme.textMainColor
            font.pixelSize: Theme.smallFontSize
            font.weight: Theme.fontLightWeight
            Layout.alignment: Qt.AlignRight
        }
        Rectangle {
            id: inViewIcon
            implicitHeight: inViewTxt.font.pixelSize
            implicitWidth: implicitHeight
            radius: height / 2
            color: Theme.inViewColor
        }
    }
}
