// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import * as names from 'names.js';

function main() {
    startApplication("keypadnavigation");
    mouseClick(waitForObject(names.scrollAreaLineEditQLineEdit), 113, 9, Qt.NoModifier, Qt.LeftButton);
    type(waitForObject(names.scrollAreaLineEditQLineEdit), "<Right>");
    type(waitForObject(names.scrollAreaLineEditQLineEdit), "<Left>");
    type(waitForObject(names.scrollAreaLineEditQLineEdit), "<Up>");
    type(waitForObject(names.scrollAreaLineEditQLineEdit), "<Down>");
    test.compare(waitForObjectExists(names.scrollAreaLineEditQLineEdit).focus, true);
    test.compare(waitForObjectExists(names.scrollAreaLineEditQLineEdit).text, "");
}
