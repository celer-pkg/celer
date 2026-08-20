// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Controls

//! [1]
DoubleSpinBox {
    id: doubleSpinBox
    from: 0
    value: 1.1
    to: 100
    stepSize: Math.pow(10, -decimals)
    decimals: 2
    editable: true
    anchors.centerIn: parent
}
//! [1]
