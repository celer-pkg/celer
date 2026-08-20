// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import * as names from 'names.js';

function main() {
    startApplication("keypadnavigation");
    scrollTo(waitForObject(names.scrollAreaQScrollBar), 248);
    clickButton(waitForObject(names.scrollAreaPushButtonQPushButton));
    type(waitForObject(names.scrollAreaPushButtonQPushButton), "<Tab>");
    type(waitForObject(names.groupBoxRadioButtonQRadioButton), "<Tab>");
    test.compare(waitForObjectExists(names.groupBox2CheckBoxQCheckBox).checked, false);
    test.compare(waitForObjectExists(names.groupBox2CheckBoxQCheckBox).focus, true);
    test.compare(waitForObjectExists(names.groupBox2CheckBox2QCheckBox).checked, false);
    test.compare(waitForObjectExists(names.groupBox2CheckBox2QCheckBox).focus, false);
    type(waitForObject(names.groupBox2CheckBoxQCheckBox), "<Down>");
    test.compare(waitForObjectExists(names.groupBox2CheckBoxQCheckBox).checked, false);
    test.compare(waitForObjectExists(names.groupBox2CheckBoxQCheckBox).focus, false);
    test.compare(waitForObjectExists(names.groupBox2CheckBox2QCheckBox).checked, false);
    test.compare(waitForObjectExists(names.groupBox2CheckBox2QCheckBox).focus, true);
    type(waitForObject(names.groupBox2CheckBox2QCheckBox), "<Up>");
    test.compare(waitForObjectExists(names.groupBox2CheckBoxQCheckBox).checked, false);
    test.compare(waitForObjectExists(names.groupBox2CheckBoxQCheckBox).focus, true);
    test.compare(waitForObjectExists(names.groupBox2CheckBox2QCheckBox).checked, false);
    test.compare(waitForObjectExists(names.groupBox2CheckBox2QCheckBox).focus, false);
    type(waitForObject(names.groupBox2CheckBoxQCheckBox), "<Down>");
    test.compare(waitForObjectExists(names.groupBox2CheckBoxQCheckBox).checked, false);
    test.compare(waitForObjectExists(names.groupBox2CheckBoxQCheckBox).focus, false);
    test.compare(waitForObjectExists(names.groupBox2CheckBox2QCheckBox).checked, false);
    test.compare(waitForObjectExists(names.groupBox2CheckBox2QCheckBox).focus, true);
    type(waitForObject(names.groupBox2CheckBox2QCheckBox), "<Up>");
    test.compare(waitForObjectExists(names.groupBox2CheckBoxQCheckBox).checked, false);
    test.compare(waitForObjectExists(names.groupBox2CheckBoxQCheckBox).focus, true);
    test.compare(waitForObjectExists(names.groupBox2CheckBox2QCheckBox).checked, false);
    test.compare(waitForObjectExists(names.groupBox2CheckBox2QCheckBox).focus, false);
    type(waitForObject(names.groupBox2CheckBoxQCheckBox), "<Right>");
    test.compare(waitForObjectExists(names.groupBox2CheckBoxQCheckBox).checked, false);
    test.compare(waitForObjectExists(names.groupBox2CheckBoxQCheckBox).focus, false);
    test.compare(waitForObjectExists(names.groupBox2CheckBox2QCheckBox).checked, false);
    test.compare(waitForObjectExists(names.groupBox2CheckBox2QCheckBox).focus, true);
    type(waitForObject(names.groupBox2CheckBox2QCheckBox), "<Left>");
    test.compare(waitForObjectExists(names.groupBox2CheckBoxQCheckBox).checked, false);
    test.compare(waitForObjectExists(names.groupBox2CheckBoxQCheckBox).focus, true);
    test.compare(waitForObjectExists(names.groupBox2CheckBox2QCheckBox).checked, false);
    test.compare(waitForObjectExists(names.groupBox2CheckBox2QCheckBox).focus, false);
    type(waitForObject(names.groupBox2CheckBoxQCheckBox), "<Right>");
    test.compare(waitForObjectExists(names.groupBox2CheckBoxQCheckBox).checked, false);
    test.compare(waitForObjectExists(names.groupBox2CheckBoxQCheckBox).focus, false);
    test.compare(waitForObjectExists(names.groupBox2CheckBox2QCheckBox).checked, false);
    test.compare(waitForObjectExists(names.groupBox2CheckBox2QCheckBox).focus, true);
    type(waitForObject(names.groupBox2CheckBox2QCheckBox), "<Left>");
    test.compare(waitForObjectExists(names.groupBox2CheckBoxQCheckBox).checked, false);
    test.compare(waitForObjectExists(names.groupBox2CheckBoxQCheckBox).focus, true);
    test.compare(waitForObjectExists(names.groupBox2CheckBox2QCheckBox).checked, false);
    test.compare(waitForObjectExists(names.groupBox2CheckBox2QCheckBox).focus, false);
    type(waitForObject(names.groupBox2CheckBoxQCheckBox), "<Right>");
    test.compare(waitForObjectExists(names.groupBox2CheckBoxQCheckBox).checked, false);
    test.compare(waitForObjectExists(names.groupBox2CheckBoxQCheckBox).focus, false);
    test.compare(waitForObjectExists(names.groupBox2CheckBox2QCheckBox).checked, false);
    test.compare(waitForObjectExists(names.groupBox2CheckBox2QCheckBox).focus, true);
    type(waitForObject(names.groupBox2CheckBox2QCheckBox), "<Right>");
    test.compare(waitForObjectExists(names.groupBox2CheckBoxQCheckBox).checked, false);
    test.compare(waitForObjectExists(names.groupBox2CheckBoxQCheckBox).focus, false);
    test.compare(waitForObjectExists(names.groupBox2CheckBox2QCheckBox).checked, false);
    test.compare(waitForObjectExists(names.groupBox2CheckBox2QCheckBox).focus, false);
}
