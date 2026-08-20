// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQml

QtObject {
    property string name
    property string floor
    property string iconName
    property string mode
    property bool active
    property int temp
    property int thermostatTemp
    property int humidity
    property int energy
    property list<int> humidityStats
    property list<int> energyStats
    property list<real> tempStats
}
