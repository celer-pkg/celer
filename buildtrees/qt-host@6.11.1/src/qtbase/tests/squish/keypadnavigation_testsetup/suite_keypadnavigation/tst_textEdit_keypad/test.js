// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import * as names from 'names.js';

function main() {
    startApplication("keypadnavigation");
    type(waitForObject(names.scrollAreaLineEditQLineEdit), "<Tab>");
    type(waitForObject(names.scrollAreaHorizontalSliderQSlider), "<Tab>");
    type(waitForObject(names.scrollAreaDateEditQDateEdit), "<Tab>");
    type(waitForObject(names.scrollAreaDateEditQDateEdit), "<Tab>");
    type(waitForObject(names.scrollAreaDateEditQDateEdit), "<Tab>");
    type(waitForObject(names.scrollAreaComboBoxQComboBox), "<Tab>");
    test.compare(waitForObjectExists(names.scrollAreaPlainTextEditQPlainTextEdit).focus, true);
    test.compare(waitForObjectExists(names.scrollAreaPlainTextEditQPlainTextEdit).plainText, "Edit some text");
    type(waitForObject(names.scrollAreaPlainTextEditQPlainTextEdit), "<Right>");
    type(waitForObject(names.scrollAreaPlainTextEditQPlainTextEdit), "<Right>");
    type(waitForObject(names.scrollAreaPlainTextEditQPlainTextEdit), " ");
    test.compare(waitForObjectExists(names.scrollAreaPlainTextEditQPlainTextEdit).plainText, "Ed it some text");
    type(waitForObject(names.scrollAreaPlainTextEditQPlainTextEdit), "<Right>");
    type(waitForObject(names.scrollAreaPlainTextEditQPlainTextEdit), "<Right>");
    type(waitForObject(names.scrollAreaPlainTextEditQPlainTextEdit), "<Right>");
    type(waitForObject(names.scrollAreaPlainTextEditQPlainTextEdit), " ");
    test.compare(waitForObjectExists(names.scrollAreaPlainTextEditQPlainTextEdit).plainText, "Ed it  some text");
    type(waitForObject(names.scrollAreaPlainTextEditQPlainTextEdit), "<Left>");
    type(waitForObject(names.scrollAreaPlainTextEditQPlainTextEdit), "<Left>");
    type(waitForObject(names.scrollAreaPlainTextEditQPlainTextEdit), "<Left>");
    type(waitForObject(names.scrollAreaPlainTextEditQPlainTextEdit), " ");
    test.compare(waitForObjectExists(names.scrollAreaPlainTextEditQPlainTextEdit).plainText, "Ed i t  some text");
    type(waitForObject(names.scrollAreaPlainTextEditQPlainTextEdit), "<Left>");
    type(waitForObject(names.scrollAreaPlainTextEditQPlainTextEdit), "<Left>");
    type(waitForObject(names.scrollAreaPlainTextEditQPlainTextEdit), "<Left>");
    type(waitForObject(names.scrollAreaPlainTextEditQPlainTextEdit), "<Left>");
    type(waitForObject(names.scrollAreaPlainTextEditQPlainTextEdit), " ");
    test.compare(waitForObjectExists(names.scrollAreaPlainTextEditQPlainTextEdit).plainText, "E d i t  some text");
    type(waitForObject(names.scrollAreaPlainTextEditQPlainTextEdit), "<Down>");
    type(waitForObject(names.scrollAreaPlainTextEditQPlainTextEdit), "<Down>");
    type(waitForObject(names.scrollAreaPlainTextEditQPlainTextEdit), "<Down>");
    type(waitForObject(names.scrollAreaPlainTextEditQPlainTextEdit), " ");
    test.compare(waitForObjectExists(names.scrollAreaPlainTextEditQPlainTextEdit).plainText, "E  d i t  some text");
    type(waitForObject(names.scrollAreaPlainTextEditQPlainTextEdit), "<Up>");
    type(waitForObject(names.scrollAreaPlainTextEditQPlainTextEdit), "<Up>");
    type(waitForObject(names.scrollAreaPlainTextEditQPlainTextEdit), "<Up>");
    type(waitForObject(names.scrollAreaPlainTextEditQPlainTextEdit), " ");
    test.compare(waitForObjectExists(names.scrollAreaPlainTextEditQPlainTextEdit).plainText, "E   d i t  some text");
}
