// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import * as names from 'names.js';

function main() {
    startApplication("qgraphicsitemgroup");
    mouseDrag(waitForObject(names.widgetViewQGraphicsView), 242, 86, 369, 113, 1, Qt.LeftButton);
    test.compare(waitForObjectExists(names.groupBoxGroupQPushButton).enabled, true);
    clickButton(waitForObject(names.groupBoxGroupQPushButton));
    test.compare(waitForObjectExists(names.groupBoxGroupQPushButton).enabled, false);
    mouseDrag(waitForObject(names.widgetViewQGraphicsView), 249, 290, 315, 97, 1, Qt.LeftButton);
    test.compare(waitForObjectExists(names.groupBoxGroupQPushButton).enabled, true);
    clickButton(waitForObject(names.groupBoxGroupQPushButton));
    test.compare(waitForObjectExists(names.groupBoxGroupQPushButton).enabled, false);
    mouseDrag(waitForObject(names.widgetViewQGraphicsView), 579, 205, -352, -108, 1, Qt.LeftButton);
    test.compare(waitForObjectExists(names.groupBoxGroupQPushButton).enabled, false);
    mouseDrag(waitForObject(names.widgetViewQGraphicsView), 221, 296, 408, 109, 1, Qt.LeftButton);
    test.compare(waitForObjectExists(names.groupBoxGroupQPushButton).enabled, false);
    mouseDrag(waitForObject(names.widgetViewQGraphicsView), 606, 403, -361, -288, 1, Qt.LeftButton);
    test.compare(waitForObjectExists(names.groupBoxGroupQPushButton).enabled, true);
    clickButton(waitForObject(names.groupBoxGroupQPushButton));
    test.compare(waitForObjectExists(names.groupBoxGroupQPushButton).enabled, false);
    mouseDrag(waitForObject(names.widgetViewQGraphicsView), 664, 417, -425, -347, 1, Qt.LeftButton);
    test.compare(waitForObjectExists(names.groupBoxGroupQPushButton).enabled, false);
}
