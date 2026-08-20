// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import * as names from 'names.js';

function main() {
    startApplication("qgraphicsitemgroup");
    test.compare(waitForObjectExists(names.groupBox2UngroupQPushButton).enabled, false);
    mouseDrag(waitForObject(names.widgetViewQGraphicsView), 220, 80, 357, 315, 1, Qt.LeftButton);
    test.compare(waitForObjectExists(names.groupBox2UngroupQPushButton).enabled, false);
    clickButton(waitForObject(names.groupBoxGroupQPushButton));
    test.compare(waitForObjectExists(names.groupBox2UngroupQPushButton).enabled,true);
    clickButton(waitForObject(names.groupBox2UngroupQPushButton));
    test.compare(waitForObjectExists(names.groupBox2UngroupQPushButton).enabled, false);
    clickButton(waitForObject(names.groupBox2RedQRadioButton));
    test.compare(waitForObjectExists(names.groupBox2UngroupQPushButton).enabled, true);
    clickButton(waitForObject(names.groupBox2UngroupQPushButton));
    test.compare(waitForObjectExists(names.groupBox2UngroupQPushButton).enabled, false);
    clickButton(waitForObject(names.groupBox2GreenQRadioButton));
    test.compare(waitForObjectExists(names.groupBox2UngroupQPushButton).enabled, true);
    clickButton(waitForObject(names.groupBox2YellowQRadioButton));
    test.compare(waitForObjectExists(names.groupBox2UngroupQPushButton).enabled, true);
    clickButton(waitForObject(names.groupBox2UngroupQPushButton));
    test.compare(waitForObjectExists(names.groupBox2UngroupQPushButton).enabled, false);
    clickButton(waitForObject(names.groupBox2GreenQRadioButton));
    test.compare(waitForObjectExists(names.groupBox2UngroupQPushButton).enabled, true);
    clickButton(waitForObject(names.groupBox2UngroupQPushButton));
    test.compare(waitForObjectExists(names.groupBox2UngroupQPushButton).enabled, false);
    clickButton(waitForObject(names.groupBox2BlueQRadioButton));
    test.compare(waitForObjectExists(names.groupBox2UngroupQPushButton).enabled, false);
    clickButton(waitForObject(names.groupBox2RedQRadioButton));
    test.compare(waitForObjectExists(names.groupBox2UngroupQPushButton).enabled, false);
    clickButton(waitForObject(names.groupBox2YellowQRadioButton));
    test.compare(waitForObjectExists(names.groupBox2UngroupQPushButton).enabled, false);
    clickButton(waitForObject(names.groupBox2GreenQRadioButton));
    test.compare(waitForObjectExists(names.groupBox2UngroupQPushButton).enabled, false);
}
