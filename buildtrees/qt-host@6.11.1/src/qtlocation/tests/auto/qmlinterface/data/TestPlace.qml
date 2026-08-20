// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtLocation

Place {
    name: "Test Place"
    placeId: "test-place-id"
    attribution: "Place data by Foo"
    categories: [
        Category {
            name: "Test category 1"
            categoryId: "test-category-id-1"
        },
        Category {
            name: "Test category 2"
            categoryId: "test-category-id-2"
        }
    ]
    location: TestLocation { }
    ratings: ({ average: 3.5, maximum: 5.0, count: 10 })
    icon: ({ parameters: { singleUrl: "http://www.example.com/test-icon.png" }})
    supplier: ({
        name: "Test supplier",
        supplierId: "test-supplier-id",
        url: "http://www.example.com/test-supplier",
        icon: ({ parameters : { singleUrl: "http://www.example.com/test-icon.png" }})
    })
    visibility: Place.PrivateVisibility
}
