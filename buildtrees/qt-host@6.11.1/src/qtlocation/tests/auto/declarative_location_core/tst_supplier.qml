// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtTest
import QtLocation

TestCase {
    id: testCase

    name: "Supplier"

    property supplier emptySupplier;

    function test_empty() {
        compare(emptySupplier.supplierId, "");
        compare(emptySupplier.name, "");
        compare(emptySupplier.url, "");
        verify(emptySupplier.icon);
    }

    property supplier qmlSupplier
    qmlSupplier {
        supplierId: "test-supplier-id"
        name: "Test Supplier"
        url: "http://example.com/test-supplier-id"

        icon: ({ parameters: { singleUrl: "http://example.com/icons/test-supplier.png" }})
    }

    function test_qmlConstructedSupplier() {
        compare(qmlSupplier.supplierId, "test-supplier-id");
        compare(qmlSupplier.name, "Test Supplier");
        compare(qmlSupplier.url, "http://example.com/test-supplier-id");
        verify(qmlSupplier.icon);
        compare(qmlSupplier.icon.parameters.singleUrl, "http://example.com/icons/test-supplier.png");
    }
}
