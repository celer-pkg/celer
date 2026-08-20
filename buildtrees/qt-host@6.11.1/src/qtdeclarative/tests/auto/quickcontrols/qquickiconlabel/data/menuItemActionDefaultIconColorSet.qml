// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick.Controls.Basic
import QtQuick.Controls.impl

MenuItem {
    id: control

    action: Action {
        icon.source: "heart.svg"
    }

    contentItem: IconLabel {
        icon: control.icon
        defaultIconColor: "grey"
    }
}
