// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Controls

ApplicationWindow {
    visible: true

    LightningView {
        anchors.fill: parent
    }

    Component.onCompleted: {
        if ((width < 200) || (height < 200)) {
            width = LightningViewConfig.appWidth
            height = LightningViewConfig.appHeight
        }
    }
}
