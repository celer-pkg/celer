// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick

Image {
    id: container

    source: "images/busy.png";

    NumberAnimation on rotation {
        running: container.visible
        from: 0; to: 360;
        loops: Animation.Infinite;
        duration: 1200
    }
}
