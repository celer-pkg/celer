// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtTest

Item {
    id: root
    TestCase {
    name:"CreateDestroy"
    when:windowShown
    function test_create_destroy() {
        var webv = Qt.createQmlObject(
        "import QtQuick; import QtWebView; WebView { url: 'http://www.qt.io/'; anchors.fill: parent; }", root);
        webv.destroy(10000);
        wait(20000);
    }
    }
}
