// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import * as names from 'names.js';

function main() {
    startApplication("qlayout");
    mouseClick(waitForObject(names.qComboBox_2), 478, 16, Qt.NoModifier, Qt.LeftButton);
    mouseClick(waitForObjectItem(names.qComboBox_2, "Grid Layout"), 376, 6, Qt.NoModifier, Qt.LeftButton);

    first = waitForObjectExists(names.o123QLabel).geometry;
    second = waitForObjectExists(names.o123QLabel_2).geometry;
    third = waitForObjectExists(names.o123QDateTimeEdit).geometry;
    fourth = waitForObjectExists(names.qComboBox).geometry;
    fifth = waitForObjectExists(names.qDateTimeEdit).geometry;
    sixth = waitForObjectExists(names.qSpinBox).geometry;

    test.compare((first.x < second.x), true);
    test.compare((second.x < third.x), true);
    test.compare((third.x < fourth.x), true);
    test.compare((fourth.x < fifth.x), true);
    test.compare((fifth.x < sixth.x), true);

    seventh = waitForObjectExists(names.o123QLineEdit).geometry;
    eighth = waitForObjectExists(names.o123QLineEdit_3).geometry;
    ninth = waitForObjectExists(names.o123QCheckBox_2).geometry;
    tenth = waitForObjectExists(names.o123QPushButton).geometry;
    eleventh = waitForObjectExists(names.o123QSpinBox_3).geometry;

    test.compare((seventh.x < eighth.x), true);
    test.compare((eighth.x < ninth.x), true);
    test.compare((ninth.x < tenth.x), true);
    test.compare((tenth.x < eleventh.x), true);

    test.compare((first.y < seventh.y), true);
    test.compare((second.y < eighth.y), true);
    test.compare((third.y < ninth.y), true);
    test.compare((fourth.y < tenth.y), true);
    test.compare((fifth.y < eleventh.y), true);
}
