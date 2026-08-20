// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtGrpcChat.Proto

Image {
    id: root

    required property fileMessage message
    // Images with PreserveAspectFit have a different height
    // then their actual paintedHeight. Limit the actual height
    // of the delegate to the height ratio.
    readonly property real heightRatio: paintedHeight / paintedWidth

    source: new URL(message.content)
    width: implicitWidth
    height: implicitHeight
    fillMode: Image.PreserveAspectFit

    clip: true
    smooth: true
    antialiasing: true
    asynchronous: true
    TapHandler { onTapped: ChatEngine.openUrl(root.message.content) }
}
