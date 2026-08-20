// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import * as names from 'names.js';

function main() {
    startApplication("qgraphicsitemgroup");
    scrollTo(waitForObject(names.groupBox2ScaleItemQSlider), 30);
    clickButton(waitForObject(names.groupBox2RedQRadioButton));
    scrollTo(waitForObject(names.groupBox2ScaleItemQSlider), 30);
    clickButton(waitForObject(names.groupBox2GreenQRadioButton));
    scrollTo(waitForObject(names.groupBox2ScaleItemQSlider), 30);
    clickButton(waitForObject(names.groupBox2YellowQRadioButton));
    scrollTo(waitForObject(names.groupBox2ScaleItemQSlider), 30);
    mouseDrag(waitForObject(names.widgetViewQGraphicsView), 577, 4, -302, 226, 1, Qt.LeftButton);
    clickButton(waitForObject(names.groupBoxGroupQPushButton));
    scrollTo(waitForObject(names.groupBoxRotateQSlider), 179);
    mouseDrag(waitForObject(names.widgetViewQGraphicsView), 703, 124, -563, 318, 1, Qt.LeftButton);
    clickButton(waitForObject(names.groupBoxGroupQPushButton));
    clickButton(waitForObject(names.groupBoxDismantleQPushButton));
}
