// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
//! [entire]
import QtQuick

Image {
    source: "images/qt-logo.png"
    transform: [
        Scale { origin.x: 25; origin.y: 25; xScale: 1.25 },
        Rotation { origin.x: 45; origin.y: 55; angle: 45 }
    ]
}
//! [entire]
