// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import * as names from 'names.js';

function main() {
    startApplication("qgraphicsitemgroup");
    test.compare(waitForObjectExists(names.groupBoxDismantleQPushButton).enabled, false);
    mouseDrag(waitForObject(names.widgetViewQGraphicsView), 243, 105, 355, 102, 1, Qt.LeftButton);
    test.compare(waitForObjectExists(names.groupBoxDismantleQPushButton).enabled, false);
    clickButton(waitForObject(names.groupBoxGroupQPushButton));
    test.compare(waitForObjectExists(names.groupBoxDismantleQPushButton).enabled, true);
    mouseDrag(waitForObject(names.widgetViewQGraphicsView), 585, 407, -343, -129, 1, Qt.LeftButton);
    test.compare(waitForObjectExists(names.groupBoxDismantleQPushButton).enabled, false);
    clickButton(waitForObject(names.groupBoxGroupQPushButton));
    test.compare(waitForObjectExists(names.groupBoxDismantleQPushButton).enabled, true);
    mouseDrag(waitForObject(names.widgetViewQGraphicsView), 250, 105, 313, 113, 1, Qt.LeftButton);
    test.compare(waitForObjectExists(names.groupBoxDismantleQPushButton).enabled, true);
    clickButton(waitForObject(names.groupBoxDismantleQPushButton));
    test.compare(waitForObjectExists(names.groupBoxDismantleQPushButton).enabled, false);
    mouseDrag(waitForObject(names.widgetViewQGraphicsView), 242, 93, 351, 120, 1, Qt.LeftButton);
    test.compare(waitForObjectExists(names.groupBoxDismantleQPushButton).enabled, false);
    mouseDrag(waitForObject(names.widgetViewQGraphicsView), 572, 426, -320, -129, 1, Qt.LeftButton);
    test.compare(waitForObjectExists(names.groupBoxDismantleQPushButton).enabled, true);
    clickButton(waitForObject(names.groupBoxDismantleQPushButton));
    test.compare(waitForObjectExists(names.groupBoxDismantleQPushButton).enabled, false);
    mouseDrag(waitForObject(names.widgetViewQGraphicsView), 588, 189, -385, -75, 1, Qt.LeftButton);
    test.compare(waitForObjectExists(names.groupBoxDismantleQPushButton).enabled, false);
    mouseDrag(waitForObject(names.widgetViewQGraphicsView), 564, 416, -321, -126, 1, Qt.LeftButton);
    test.compare(waitForObjectExists(names.groupBoxDismantleQPushButton).enabled, false);
    mouseDrag(waitForObject(names.widgetViewQGraphicsView), 234, 112, 126, 298, 1, Qt.LeftButton);
    test.compare(waitForObjectExists(names.groupBoxDismantleQPushButton).enabled, false);
    clickButton(waitForObject(names.groupBoxGroupQPushButton));
    test.compare(waitForObjectExists(names.groupBoxDismantleQPushButton).enabled, true);
    mouseDrag(waitForObject(names.widgetViewQGraphicsView), 455, 103, 103, 319, 1, Qt.LeftButton);
    test.compare(waitForObjectExists(names.groupBoxDismantleQPushButton).enabled, false);
    mouseDrag(waitForObject(names.widgetViewQGraphicsView), 370, 418, -126, -335, 1, Qt.LeftButton);
    test.compare(waitForObjectExists(names.groupBoxDismantleQPushButton).enabled, true);
    clickButton(waitForObject(names.groupBoxDismantleQPushButton));
    test.compare(waitForObjectExists(names.groupBoxDismantleQPushButton).enabled, false);
}
