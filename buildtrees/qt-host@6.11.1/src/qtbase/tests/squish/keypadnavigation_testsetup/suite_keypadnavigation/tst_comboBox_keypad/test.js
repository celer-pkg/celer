// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import * as names from 'names.js';

function main() {
    startApplication("keypadnavigation");
    mouseClick(waitForObject(names.scrollAreaLabel7QLabel), 97, 13, Qt.NoModifier, Qt.LeftButton);
    type(waitForObject(names.scrollAreaLineEditQLineEdit), "<Tab>");
    type(waitForObject(names.scrollAreaHorizontalSliderQSlider), "<Tab>");
    type(waitForObject(names.scrollAreaDateEditQDateEdit), "<Tab>");
    type(waitForObject(names.scrollAreaDateEditQDateEdit), "<Tab>");
    type(waitForObject(names.scrollAreaDateEditQDateEdit), "<Tab>");
    test.compare(waitForObjectExists(names.scrollAreaComboBoxQComboBox).currentText, "Item 1");
    type(waitForObject(names.scrollAreaComboBoxQComboBox), "<Down>");
    test.compare(waitForObjectExists(names.scrollAreaComboBoxQComboBox).currentText, "Item 2");
    type(waitForObject(names.scrollAreaComboBoxQComboBox), "<Down>");
    test.compare(waitForObjectExists(names.scrollAreaComboBoxQComboBox).currentText, "Item 3");
    type(waitForObject(names.scrollAreaComboBoxQComboBox), "<Down>");
    test.compare(waitForObjectExists(names.scrollAreaComboBoxQComboBox).currentText, "Item 4");
    type(waitForObject(names.scrollAreaComboBoxQComboBox), "<Down>");
    test.compare(waitForObjectExists(names.scrollAreaComboBoxQComboBox).currentText, "Item 5");
    type(waitForObject(names.scrollAreaComboBoxQComboBox), "<Down>");
    test.compare(waitForObjectExists(names.scrollAreaComboBoxQComboBox).currentText, "Item 6");
    type(waitForObject(names.scrollAreaComboBoxQComboBox), "<Down>");
    test.compare(waitForObjectExists(names.scrollAreaComboBoxQComboBox).currentText, "Item 7");
    type(waitForObject(names.scrollAreaComboBoxQComboBox), "<Down>");
    test.compare(waitForObjectExists(names.scrollAreaComboBoxQComboBox).currentText, "Item 9");
    type(waitForObject(names.scrollAreaComboBoxQComboBox), "<Down>");
    test.compare(waitForObjectExists(names.scrollAreaComboBoxQComboBox).currentText, "Item 10");
    type(waitForObject(names.scrollAreaComboBoxQComboBox), "<Down>");
    test.compare(waitForObjectExists(names.scrollAreaComboBoxQComboBox).currentText, "Item 10");
    type(waitForObject(names.scrollAreaComboBoxQComboBox), "<Down>");
    test.compare(waitForObjectExists(names.scrollAreaComboBoxQComboBox).currentText, "Item 10");
    type(waitForObject(names.scrollAreaComboBoxQComboBox), "<Up>");
    test.compare(waitForObjectExists(names.scrollAreaComboBoxQComboBox).currentText, "Item 9");
    type(waitForObject(names.scrollAreaComboBoxQComboBox), "<Up>");
    test.compare(waitForObjectExists(names.scrollAreaComboBoxQComboBox).currentText, "Item 7");
    type(waitForObject(names.scrollAreaComboBoxQComboBox), "<Up>");
    test.compare(waitForObjectExists(names.scrollAreaComboBoxQComboBox).currentText, "Item 6");
    type(waitForObject(names.scrollAreaComboBoxQComboBox), "<Up>");
    test.compare(waitForObjectExists(names.scrollAreaComboBoxQComboBox).currentText, "Item 5");
    type(waitForObject(names.scrollAreaComboBoxQComboBox), "<Left>");
    test.compare(waitForObjectExists(names.scrollAreaComboBoxQComboBox).currentText, "Item 5");
    type(waitForObject(names.scrollAreaComboBoxQComboBox), "<Left>");
    test.compare(waitForObjectExists(names.scrollAreaComboBoxQComboBox).currentText, "Item 5");
    type(waitForObject(names.scrollAreaComboBoxQComboBox), "<Right>");
    test.compare(waitForObjectExists(names.scrollAreaComboBoxQComboBox).currentText, "Item 5");
    type(waitForObject(names.scrollAreaComboBoxQComboBox), "<Right>");
    test.compare(waitForObjectExists(names.scrollAreaComboBoxQComboBox).currentText, "Item 5");
    type(waitForObject(names.scrollAreaComboBoxQComboBox), "<Up>");
    test.compare(waitForObjectExists(names.scrollAreaComboBoxQComboBox).currentText, "Item 4");
    type(waitForObject(names.scrollAreaComboBoxQComboBox), "<Up>");
    test.compare(waitForObjectExists(names.scrollAreaComboBoxQComboBox).currentText, "Item 3");
    type(waitForObject(names.scrollAreaComboBoxQComboBox), "<Up>");
    test.compare(waitForObjectExists(names.scrollAreaComboBoxQComboBox).currentText, "Item 2");
    type(waitForObject(names.scrollAreaComboBoxQComboBox), "<Up>");
    test.compare(waitForObjectExists(names.scrollAreaComboBoxQComboBox).currentText, "Item 1");
    type(waitForObject(names.scrollAreaComboBoxQComboBox), "<Up>");
    test.compare(waitForObjectExists(names.scrollAreaComboBoxQComboBox).currentText, "Item 1");
    type(waitForObject(names.scrollAreaComboBoxQComboBox), "<Up>");
    test.compare(waitForObjectExists(names.scrollAreaComboBoxQComboBox).currentText, "Item 1");
}
