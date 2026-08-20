// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtDataVisualization

Item {
    property alias column: columnAxis
    property alias row: rowAxis
    property alias value: valueAxis
    property alias total: totalAxis

    // Custom labels for columns, since the data contains abbreviated month names.
    //! [0]
    CategoryAxis3D {
        id: columnAxis
        labels: ["January", "February", "March", "April", "May", "June",
            "July", "August", "September", "October", "November", "December"]
        labelAutoRotation: 30
    }
    //! [0]
    CategoryAxis3D {
        id: totalAxis
        labels: ["Yearly total"]
        labelAutoRotation: 30
    }
    CategoryAxis3D {
        // For row labels we can use row labels from data proxy, no labels defined for rows.
        id: rowAxis
        labelAutoRotation: 30
    }

    ValueAxis3D {
        id: valueAxis
        min: 0
        max: 35
        labelFormat: "%.2f M\u20AC"
        title: "Monthly income"
        labelAutoRotation: 90
    }
}
