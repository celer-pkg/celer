// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick

HumidityInfoForm {
    Component.onCompleted: {
        let sum = 0
        let count = humidityValues.length
        for (let i = 0; i < count; ++i)
            sum += humidityValues[i]
        humidityAvg = sum / count
        humidityDiff = 100 * (humidityValues[count-1] / humidityValues[count-2] - 1)
        isMore = humidityDiff > 0
    }
}
