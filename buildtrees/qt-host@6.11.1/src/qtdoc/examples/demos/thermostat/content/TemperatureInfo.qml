// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick

TemperatureInfoForm {
    Component.onCompleted: {
        let sum = 0.0
        let count = temperatureValues.length
        for (let i = 0; i < count; ++i)
            sum += temperatureValues[i]
        avgTemp = sum / count
        minTemp = Math.min(...temperatureValues)
        maxTemp = Math.max(...temperatureValues)
    }
}
