// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtGraphs

Item {
    property alias column: columnAxis
    property alias row: rowAxis
    property alias value: valueAxis
    property alias total: totalAxis

    // Custom labels for columns, since the data contains abbreviated month names.
    //! [0]
    Category3DAxis {
        id: columnAxis
        labels: ["January", "February", "March", "April", "May", "June",
            "July", "August", "September", "October", "November", "December"]
        labelAutoAngle: 30
    }
    //! [0]
    Category3DAxis {
        id: totalAxis
        labels: ["Yearly total"]
        labelAutoAngle: 30
    }
    Category3DAxis {
        // For row labels we can use row labels from data proxy, no labels defined for rows.
        id: rowAxis
        labelAutoAngle: 30
    }

    Value3DAxis {
        id: valueAxis
        min: 0
        max: 35
        labelFormat: "%.2f M\u20AC"
        title: "Monthly income"
        labelAutoAngle: 90
    }
}
