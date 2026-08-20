// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    // This is to hide the Label's implicitWidth from any layout
    // since the label reports the unwrapped width instead of
    // the wrapped width
    property alias text: label.text
    Layout.fillWidth: true
    height: label.implicitHeight
    implicitWidth: width
    implicitHeight: label.implicitHeight
    Label {
        id: label
        width: parent.width
        textFormat: Text.MarkdownText
        wrapMode: Text.WordWrap
    }
}
