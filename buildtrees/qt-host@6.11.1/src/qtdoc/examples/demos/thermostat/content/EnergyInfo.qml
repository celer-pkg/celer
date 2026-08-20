// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick

EnergyInfoForm {
    Component.onCompleted: {
        for (let i = 0; i < energyValues.length; ++i)
            totalEnergy += energyValues[i]
    }
}
