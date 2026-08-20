// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import * as names from 'names.js';

function main() {
    startApplication("keypadnavigation");
    test.compare(waitForObjectExists(names.scrollAreaLineEditQLineEdit).focus, true);
    test.compare(waitForObjectExists(names.scrollAreaHorizontalSliderQSlider).focus, false);
    test.compare(waitForObjectExists(names.scrollAreaHorizontalSliderQSlider).sliderPosition, 0);
    test.compare(waitForObjectExists(names.scrollAreaLineEdit4QLineEdit).focus, false);
    test.compare(waitForObjectExists(names.scrollAreaLineEdit4QLineEdit).text, "");
    test.compare(waitForObjectExists(names.scrollAreaHorizontalSlider2QSlider).focus, false);
    test.compare(waitForObjectExists(names.scrollAreaHorizontalSlider2QSlider).sliderPosition, 0);
    test.compare(waitForObjectExists(names.scrollAreaComboBoxQComboBox).currentText, "Item 1");
    sendEvent("QWheelEvent", waitForObject(names.scrollAreaScrollAreaWidgetContentsQWidget), 188, 140, -120, 0, 2);
    sendEvent("QWheelEvent", waitForObject(names.scrollAreaScrollAreaWidgetContentsQWidget), 188, 200, -120, 0, 2);
    sendEvent("QWheelEvent", waitForObject(names.scrollAreaScrollAreaWidgetContentsQWidget), 188, 260, -120, 0, 2);
    sendEvent("QWheelEvent", waitForObject(names.scrollAreaPlainTextEditQPlainTextEdit), 176, 53, -120, 0, 2);
    sendEvent("QWheelEvent", waitForObject(names.scrollAreaScrollAreaWidgetContentsQWidget), 188, 380, -120, 0, 2);
    test.compare(waitForObjectExists(names.scrollAreaPlainTextEditQPlainTextEdit).plainText, "Edit some text");
    test.compare(waitForObjectExists(names.groupBoxRadioButtonQRadioButton).enabled, false);
    test.compare(waitForObjectExists(names.groupBoxRadioButton2QRadioButton).enabled, false);
    test.compare(waitForObjectExists(names.groupBox2CheckBoxQCheckBox).enabled, false);
    test.compare(waitForObjectExists(names.groupBox2CheckBox2QCheckBox).enabled, false);
}
