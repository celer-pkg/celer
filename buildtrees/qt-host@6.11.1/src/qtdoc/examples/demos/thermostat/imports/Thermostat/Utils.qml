// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

pragma Singleton

import QtQml
import QtQuick

QtObject {
    function rgbBlend(a: color, b: color, factor: real) : color {
        if (factor <= 0.0)
            return a
        if (factor >= 1.0)
            return b

        let factorComplement = 1.0 - factor
        return Qt.rgba(a.r * factorComplement + b.r * factor,
                       a.g * factorComplement + b.g * factor,
                       a.b * factorComplement + b.b * factor)
    }
}
