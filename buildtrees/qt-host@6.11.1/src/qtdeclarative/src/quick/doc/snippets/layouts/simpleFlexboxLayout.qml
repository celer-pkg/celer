// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ApplicationWindow {
    width: 640
    height: 480
    visible: true
    title: qsTr("QML Flexbox Layout")
    //! [layout-definition]
    FlexboxLayout {
        id: flexLayout
        anchors.fill: parent
        wrap: FlexboxLayout.Wrap
        direction: FlexboxLayout.Row
        justifyContent: FlexboxLayout.JustifySpaceAround
        Rectangle {
            color: 'teal'
            implicitWidth: 200
            implicitHeight: 200
        }
        Rectangle {
            color: 'plum'
            implicitWidth: 200
            implicitHeight: 200
        }
        Rectangle {
            color: 'olive'
            implicitWidth: 200
            implicitHeight: 200
        }
        Rectangle {
            color: 'beige'
            implicitWidth: 200
            implicitHeight: 200
        }
        Rectangle {
            color: 'darkseagreen'
            implicitWidth: 200
            implicitHeight: 200
        }
    }
    //! [layout-definition]
}
