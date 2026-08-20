// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import * as names from 'names.js';

function main() {
    startApplication("qlayout");
    first = waitForObjectExists(names.o123QLineEdit_2).geometry.x;
    second = waitForObjectExists(names.qComboBox).geometry.x;
    third =  waitForObjectExists(names.qComboBox_3).geometry.x;
    fourth = waitForObjectExists(names.o123QDateTimeEdit).geometry.x;
    fifth = waitForObjectExists(names.qDateTimeEdit).geometry.x;
    sixth = waitForObjectExists(names.o123QSpinBox).geometry.x;

    test.compare((first < second), true);
    test.compare((second < third), true);
    test.compare((third < fourth), true);
    test.compare((fourth < fifth), true);
    test.compare((fifth < sixth), true);
}
