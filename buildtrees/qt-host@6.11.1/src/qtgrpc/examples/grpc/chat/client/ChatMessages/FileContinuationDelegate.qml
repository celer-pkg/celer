// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import QtGrpcChat.Proto

ColumnLayout {
    id: root

    required property fileMessage message
    property color textColor

    ProgressBar {
        id: fileProgress
        to: root.message.continuation.count
        value: root.message.continuation.index + 1
    }
    RowLayout {
        Text {
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignLeft
            text: root.message.name
            color: root.textColor
            font.pointSize: 14
        }
        Text {
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignRight
            text: fileProgress.value + "/" + fileProgress.to
            color: root.textColor
            font.pointSize: 14
        }
    }

}
