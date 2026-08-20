// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtTest
import QtLocation
import "utils.js" as Utils

TestCase {
    id: testCase

    name: "Icon"

    property icon emptyIcon

    function test_empty() {
        compare(emptyIcon.parameters, {})
    }


    property icon qmlIconSingleUrl: ({ parameters: { singleUrl: "http://example.com/icon.png" }})

    function test_qmlSingleUrlIcon() {
        var u = qmlIconSingleUrl.url(Qt.size(64, 64));
        compare(u, "http://example.com/icon.png");

        u = qmlIconSingleUrl.url(Qt.size(20, 20));
        compare(u, "http://example.com/icon.png");

        var params = qmlIconSingleUrl.parameters
        params.singleUrl = "/home/user/icon.png"
        qmlIconSingleUrl.parameters = params
        u = qmlIconSingleUrl.url(Qt.size(20, 20));
        compare(u, "file:///home/user/icon.png");
    }
}
