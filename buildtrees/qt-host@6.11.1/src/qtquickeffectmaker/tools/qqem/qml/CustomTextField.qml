// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick.Controls.Basic as ControlStyle

ControlStyle.TextField {
    background: Rectangle {
        color: mainView.backgroundColor2
        border.color: mainView.foregroundColor1
        border.width: 1
    }
    color: mainView.foregroundColor2
    font.pixelSize: 14
}
