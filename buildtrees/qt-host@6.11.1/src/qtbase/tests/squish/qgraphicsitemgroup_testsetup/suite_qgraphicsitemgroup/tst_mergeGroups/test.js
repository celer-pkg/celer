// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import * as names from 'names.js';

function main() {
    startApplication("qgraphicsitemgroup");
    test.compare(waitForObjectExists(names.groupBoxMergeQPushButton).enabled, false);
    mouseDrag(waitForObject(names.widgetViewQGraphicsView), 244, 86, 324, 110, 1, Qt.LeftButton);
    test.compare(waitForObjectExists(names.groupBoxMergeQPushButton).enabled, false);
    clickButton(waitForObject(names.groupBoxGroupQPushButton));
    test.compare(waitForObjectExists(names.groupBoxMergeQPushButton).enabled, false);
    mouseDrag(waitForObject(names.widgetViewQGraphicsView), 233, 86, 346, 313, 1, Qt.LeftButton);
    test.compare(waitForObjectExists(names.groupBoxMergeQPushButton).enabled, false);
    mouseDrag(waitForObject(names.widgetViewQGraphicsView), 252, 291, 337, 104, 1, Qt.LeftButton);
    test.compare(waitForObjectExists(names.groupBoxMergeQPushButton).enabled, false);
    clickButton(waitForObject(names.groupBoxGroupQPushButton));
    test.compare(waitForObjectExists(names.groupBoxMergeQPushButton).enabled, false);
    mouseDrag(waitForObject(names.widgetViewQGraphicsView), 206, 65, 423, 357, 1, Qt.LeftButton);
    test.compare(waitForObjectExists(names.groupBoxMergeQPushButton).enabled, true);
    clickButton(waitForObject(names.groupBoxMergeQPushButton));
    test.compare(waitForObjectExists(names.groupBoxMergeQPushButton).enabled, false);
}
