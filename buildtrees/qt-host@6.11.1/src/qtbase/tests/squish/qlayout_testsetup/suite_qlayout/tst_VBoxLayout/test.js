// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import * as names from 'names.js';

function main() {
    startApplication("qlayout");
    mouseClick(waitForObject(names.qComboBox_2), 181, 21, Qt.NoModifier, Qt.LeftButton);
    mouseClick(waitForObjectItem(names.qComboBox_2, "VBox Layout"), 143, 7, Qt.NoModifier, Qt.LeftButton);

    first = waitForObjectExists(names.o123QLineEdit).geometry.y;
    second = waitForObjectExists(names.qComboBox).geometry.y;
    third = waitForObjectExists(names.qComboBox_3).geometry.y;
    fourth = waitForObjectExists(names.o123QDateTimeEdit).geometry.y;
    fifth = waitForObjectExists(names.o123QDateTimeEdit_2).geometry.y;
    sixth = waitForObjectExists(names.o123QSpinBox_2).geometry.y;

    test.compare((first < second), true);
    test.compare((second < third), true);
    test.compare((third < fourth), true);
    test.compare((fourth < fifth), true);
    test.compare((fifth < sixth), true);
}
