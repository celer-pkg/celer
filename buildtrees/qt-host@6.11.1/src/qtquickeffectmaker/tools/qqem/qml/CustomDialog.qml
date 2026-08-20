// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick.Controls

Dialog {
    id: rootItem
    Material.roundedScale: Material.SmallScale

    Overlay.modal: Rectangle {
        color: "#B0000000"
    }
}
