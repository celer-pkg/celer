// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import * as names from 'names.js';

function main() {
    startApplication("qgraphicsitemgroup");
    mouseDrag(waitForObject(names.widgetViewQGraphicsView), 244, 96, 103, 97, 1, Qt.LeftButton);
    test.compare(waitForObjectExists(names.groupBoxGroupQPushButton).enabled, false);
    mouseClick(waitForObject(names.viewQGraphicsRectItem), 27, 30, Qt.NoModifier, Qt.LeftButton);
    test.compare(waitForObjectExists(names.groupBoxGroupQPushButton).enabled, false);
    mouseClick(waitForObject(names.viewQGraphicsRectItem_2), 36, 31, Qt.NoModifier, Qt.LeftButton);
    test.compare(waitForObjectExists(names.groupBoxGroupQPushButton).enabled, false);
    mouseClick(waitForObject(names.viewQGraphicsRectItem_3), 7, 28, Qt.NoModifier, Qt.LeftButton);
    test.compare(waitForObjectExists(names.groupBoxGroupQPushButton).enabled, false);
    mouseDrag(waitForObject(names.widgetViewQGraphicsView), 210, 86, 345, 105, 1, Qt.LeftButton);
    test.compare(waitForObjectExists(names.groupBoxGroupQPushButton).enabled, true);
    mouseDrag(waitForObject(names.widgetViewQGraphicsView), 334, 54, -68, 357, 1, Qt.LeftButton);
    test.compare(waitForObjectExists(names.groupBoxGroupQPushButton).enabled, true);
    mouseDrag(waitForObject(names.widgetViewQGraphicsView), 231, 101, 381, 328, 1, Qt.LeftButton);
    test.compare(waitForObjectExists(names.groupBoxGroupQPushButton).enabled, true);
    mouseDrag(waitForObject(names.widgetViewQGraphicsView), 548, 63, -97, 349, 1, Qt.LeftButton);
    test.compare(waitForObjectExists(names.groupBoxGroupQPushButton).enabled, true);
    mouseDrag(waitForObject(names.widgetViewQGraphicsView), 605, 384, -413, -74, 1, Qt.LeftButton);
    test.compare(waitForObjectExists(names.groupBoxGroupQPushButton).enabled, true);
    mouseDrag(waitForObject(names.widgetViewQGraphicsView), 583, 390, -346, -279, 1, Qt.LeftButton);
    test.compare(waitForObjectExists(names.groupBoxGroupQPushButton).enabled, true);
}
