# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import names


def main():
    attachToApplication("calqlatr")

    # Digit buttons
    button0 = waitForObjectExists(names.digitButton0)
    button1 = waitForObjectExists(names.digitButton1)
    button2 = waitForObjectExists(names.digitButton2)
    button3 = waitForObjectExists(names.digitButton3)
    button4 = waitForObjectExists(names.digitButton4)
    button5 = waitForObjectExists(names.digitButton5)
    button6 = waitForObjectExists(names.digitButton6)
    button7 = waitForObjectExists(names.digitButton7)
    button8 = waitForObjectExists(names.digitButton8)
    button9 = waitForObjectExists(names.digitButton9)
    decimal = waitForObjectExists(names.decimal)

    allClear = waitForObjectExists(names.allClearButton)
    backspace = waitForObjectExists(names.backspaceButton)

    test.log("Test that '.' button is dimmed before digit button press")
    test.compare(decimal.dimmed, True)
    test.log("Check that digit buttons work and corresponding numbers appear on display")
    test.log("Tap '0' digit button")
    tapObject(button0)
    test.log("Test that '.' button is not dimmed after digit button press")
    test.compare(decimal.dimmed, False)
    test.log("Tap '.' button")
    tapObject(decimal)
    test.log("Tap '1' digit button")
    tapObject(button1)
    test.log("Tap '2' digit button")
    tapObject(button2)
    test.log("Tap '3' digit button")
    tapObject(button3)
    test.log("Test that display shows a decimal numbers '0.123'")
    test.compare(str(waitForObjectExists(names.inputTextline1).text), "0.123")
    test.log("Clear display")
    tapObject(allClear)

    test.log("Tap '4' digit button")
    tapObject(button4)
    test.log("Tap '5' digit button")
    tapObject(button5)
    test.log("Tap '6' digit button")
    tapObject(button6)
    test.log("Tap '7' digit button")
    tapObject(button7)
    test.log("Test that display shows numbers '4567'")
    test.compare(str(waitForObjectExists(names.inputTextline1).text), "4567")
    test.log("Tap '7' Backspace button")
    tapObject(backspace)
    test.log("Test that Backspace removed 7 from the display and current number is '456'")
    test.compare(str(waitForObjectExists(names.inputTextline1).text), "456")
    test.log("Clear display")
    tapObject(allClear)

    test.log("Tap '8' digit button")
    tapObject(button8)
    test.log("Tap '9' digit button")
    tapObject(button9)
    test.log("Test that display shows numbers '89'")
    test.compare(str(waitForObjectExists(names.inputTextline1).text), "89")
    test.log("Clear display")
    tapObject(allClear)

    test.log("After Clearing the display, check that there are no new items in it")
    try:
        findObject(names.inputTextline1)
        test.xpasses("Unexpected Pass: Unexpectedly found new items")
    except LookupError as err:
        test.xfail("Expected Fail: No new items found", str(err))
