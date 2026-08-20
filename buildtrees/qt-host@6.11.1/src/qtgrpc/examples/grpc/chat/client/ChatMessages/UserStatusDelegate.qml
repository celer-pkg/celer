// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick

import QtGrpcChat.Proto

Text {
    id: innerText

    required property var base

    color: base.lightColor
    font.pointSize: 14
    text: {
        switch (base.display.userStatus.type) {
            case UserStatus.Type.LOGIN:
                return "<b><i>" + base.messageOwner + "</i></b> " + qsTr("joined the chat")
            case UserStatus.Type.LOGOUT:
            return "<b><i>" + base.messageOwner + "</i></b> " + qsTr("left the chat")
            case UserStatus.Type.ACTIVE:
                return "<b><i>" + base.messageOwner + "</i></b> " + qsTr("went active")
            case UserStatus.Type.INACTIVE:
                return "<b><i>" + base.messageOwner + "</i></b> " + qsTr("went inactive")
        }
    }
}
