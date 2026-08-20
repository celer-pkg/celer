// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtGrpcChat.Proto

//! [client-qml-5]
TextEdit {
    id: root

    required property textMessage message

    text: message.content
    color: "#f3f3f3"
    font.pointSize: 14
    wrapMode: TextEdit.Wrap
    readOnly: true
    selectByMouse: true
}
//! [client-qml-5]
