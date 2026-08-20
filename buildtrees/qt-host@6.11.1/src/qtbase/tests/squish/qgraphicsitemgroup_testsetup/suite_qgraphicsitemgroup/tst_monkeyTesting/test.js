// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import * as names from 'names.js';

function main() {
    startApplication("qgraphicsitemgroup");
    mouseDrag(waitForObject(names.widgetViewQGraphicsView), 208, 100, 373, 314, 1, Qt.LeftButton);
    clickButton(waitForObject(names.groupBoxGroupQPushButton));
    clickButton(waitForObject(names.groupBox2UngroupQPushButton));
    clickButton(waitForObject(names.groupBox2YellowQRadioButton));
    clickButton(waitForObject(names.groupBox2UngroupQPushButton));
    mouseClick(waitForObject(names.qGraphicsRectItem_2), 5, 36, Qt.NoModifier, Qt.LeftButton);
    scrollTo(waitForObject(names.groupBoxScaleQSlider), 5);
    scrollTo(waitForObject(names.groupBox2ScaleItemQSlider), 5);
    clickButton(waitForObject(names.groupBox2BlueQRadioButton));
    scrollTo(waitForObject(names.groupBox2ScaleItemQSlider), 5);
    scrollTo(waitForObject(names.groupBox2RotateItemQSlider), 55);
    scrollTo(waitForObject(names.groupBox2ScaleItemQSlider), 7);
    clickButton(waitForObject(names.groupBox2RedQRadioButton));
    scrollTo(waitForObject(names.groupBox2RotateItemQSlider), 31);
    scrollTo(waitForObject(names.groupBox2RotateItemQSlider), 99);
    scrollTo(waitForObject(names.groupBox2RotateItemQSlider), 109);
    scrollTo(waitForObject(names.groupBox2ScaleItemQSlider), 12);
    scrollTo(waitForObject(names.groupBox2ScaleItemQSlider), 12);
    clickButton(waitForObject(names.groupBox2GreenQRadioButton));
    scrollTo(waitForObject(names.groupBox2RotateItemQSlider), 336);
    scrollTo(waitForObject(names.groupBox2ScaleItemQSlider), 28);
    clickButton(waitForObject(names.groupBox2UngroupQPushButton));
    clickButton(waitForObject(names.groupBox2YellowQRadioButton));
    scrollTo(waitForObject(names.groupBox2RotateItemQSlider), 31);
    mouseDrag(waitForObject(names.widgetGroupBox2QGroupBox), 104, 129, 13, 7, 1, Qt.LeftButton);
    scrollTo(waitForObject(names.groupBox2ScaleItemQSlider), 27);
    clickButton(waitForObject(names.groupBoxDismantleQPushButton));
    mouseDrag(waitForObject(names.widgetViewQGraphicsView), 245, 94, 321, 193, 1, Qt.LeftButton);
    clickButton(waitForObject(names.groupBoxGroupQPushButton));
    scrollTo(waitForObject(names.groupBoxRotateQSlider), 275);
    scrollTo(waitForObject(names.groupBoxScaleQSlider), 10);
    scrollTo(waitForObject(names.groupBoxScaleQSlider), 10);
    mouseClick(waitForObject(names.widgetGroupBoxQGroupBox), 261, 111, Qt.NoModifier, Qt.LeftButton);
    clickButton(waitForObject(names.groupBoxDismantleQPushButton));
    mouseDrag(waitForObject(names.widgetViewQGraphicsView), 254, 40, 386, 106, 1, Qt.LeftButton);
    mouseDrag(waitForObject(names.widgetViewQGraphicsView), 661, 189, -330, 146, 1, Qt.LeftButton);
    clickButton(waitForObject(names.groupBoxGroupQPushButton));
    mouseClick(waitForObject(names.viewQGraphicsRectItem_4), 23, 31, Qt.NoModifier, Qt.LeftButton);
    mouseDrag(waitForObject(names.widgetViewQGraphicsView), 252, 20, 416, 382, 1, Qt.LeftButton);
    clickButton(waitForObject(names.groupBoxGroupQPushButton));
    clickButton(waitForObject(names.groupBoxDismantleQPushButton));
    mouseDrag(waitForObject(names.widgetViewQGraphicsView), 657, 197, -233, -61, 1, Qt.LeftButton);
    mouseDrag(waitForObject(names.widgetViewQGraphicsView), 292, 70, 338, 101, 1, Qt.LeftButton);
    clickButton(waitForObject(names.groupBoxGroupQPushButton));
    mouseDrag(waitForObject(names.widgetViewQGraphicsView), 525, 292, -291, -22, 1, Qt.LeftButton);
    clickButton(waitForObject(names.groupBoxGroupQPushButton));
    mouseDrag(waitForObject(names.widgetViewQGraphicsView), 623, 351, -331, -204, 1, Qt.LeftButton);
    clickButton(waitForObject(names.groupBoxMergeQPushButton));
    mouseDrag(waitForObject(names.widgetGroupBoxQGroupBox), 41, 132, 23, 12, 1, Qt.LeftButton);
    scrollTo(waitForObject(names.groupBoxScaleQSlider), 5);
    scrollTo(waitForObject(names.groupBoxRotateQSlider), 240);
    clickButton(waitForObject(names.groupBoxDismantleQPushButton));
}
