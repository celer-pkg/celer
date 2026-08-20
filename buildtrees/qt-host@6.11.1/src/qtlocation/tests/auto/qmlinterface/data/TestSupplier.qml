// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtLocation
import QtQuick

Item {
    property supplier supplier: ({
        name: "Test supplier",
        supplierId: "test-supplier-id",
        url: "http://www.example.com/test-supplier",
        icon: ({ parameters: { singleUrl: "http://www.example.com/test-icon.png" }})
    })
}
