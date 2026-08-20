# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import names


def main():
    attachToApplication("calqlatr")

    button0 = waitForObjectExists(names.digitButton0)
    button1 = waitForObjectExists(names.digitButton1)
    button2 = waitForObjectExists(names.digitButton2)
    button5 = waitForObjectExists(names.digitButton5)

    division = waitForObjectExists(names.divisionButton)
    multiplication = waitForObjectExists(names.multiplicationButton)
    minus = waitForObjectExists(names.minusButton)
    plus = waitForObjectExists(names.plusButton)

    equals = waitForObjectExists(names.equalsButton)
    allClear = waitForObjectExists(names.allClearButton)

    # Division
    test.log("Check that display shows 10 after pressing digits 1 and 0 in sequence")
    tapObject(button1)
    tapObject(button0)
    test.compare(str(waitForObjectExists(names.inputTextline1).text), "10")
    test.log("Check that divide symbol (÷) is shown when tapping divide button")
    tapObject(division)
    test.compare(str(waitForObjectExists(names.operatorTextline2).text), "÷")
    test.log("Check that number '5' is shown when tapping corresponding button")
    tapObject(button5)
    test.compare(str(waitForObjectExists(names.inputTextline2).text), "5")
    test.log("Check that equal symbol (=) is shown when tapping equal button")
    tapObject(equals)
    test.compare(str(waitForObjectExists(names.operatorTextline3).text), "=")
    test.log("Check that number '2' is shown after division operation")
    test.compare(str(waitForObjectExists(names.inputTextline3).text), "2")
    test.log("Clear display")
    tapObject(allClear)

    # Multiplication
    test.log("Check that number '5' is shown when tapping corresponding button")
    tapObject(button5)
    test.compare(str(waitForObjectExists(names.inputTextline1).text), "5")
    test.log("Check that multiply symbol (×) is shown when tapping multiply button")
    tapObject(multiplication)
    test.compare(str(waitForObjectExists(names.operatorTextline2).text), "×")
    test.log("Check that number '5' is shown when tapping corresponding button for the second time")
    tapObject(button5)
    test.compare(str(waitForObjectExists(names.inputTextline2).text), "5")
    test.log("Check that equal symbol (=) is shown when tapping equal button")
    doubleTap(equals)
    test.compare(str(waitForObjectExists(names.operatorTextline3).text), "=")
    test.log("Check that number '25' is shown after multiplication operation")
    test.compare(str(waitForObjectExists(names.inputTextline3).text), "25")
    test.log("Clear display")
    tapObject(allClear)

    # Subtraction
    test.log("Check that number '5' is shown when tapping corresponding button")
    tapObject(button5)
    test.compare(str(waitForObjectExists(names.inputTextline1).text), "5")
    test.log("Check that subtract symbol (−) is shown when tapping subtract button")
    tapObject(minus)
    test.compare(str(waitForObjectExists(names.operatorTextline2).text), "−")
    test.log("Check that number '2' is shown when tapping corresponding button")
    tapObject(button2)
    test.compare(str(waitForObjectExists(names.inputTextline2).text), "2")
    test.log("Check that equal symbol (=) is shown when tapping equal button")
    tapObject(equals)
    test.compare(str(waitForObjectExists(names.operatorTextline3).text), "=")
    test.log("Check that number '3' is shown after subtraction operation")
    test.compare(str(waitForObjectExists(names.inputTextline3).text), "3")
    test.log("Clear display")
    tapObject(allClear)

    # Addition
    test.log("Check that number '1' is shown when tapping corresponding button")
    tapObject(button1)
    test.compare(str(waitForObjectExists(names.inputTextline1).text), "1")
    test.log("Check that add symbol (+) is shown when tapping add button")
    tapObject(plus)
    test.compare(str(waitForObjectExists(names.operatorTextline2).text), "+")
    test.log("Check that number '1' is shown when tapping corresponding button for the second time")
    tapObject(button1)
    test.compare(str(waitForObjectExists(names.inputTextline2).text), "1")
    test.log("Check that equal symbol (=) is shown when tapping equal button")
    tapObject(equals)
    test.compare(str(waitForObjectExists(names.operatorTextline3).text), "=")
    test.log("Check that number '2' is shown after addition operation")
    test.compare(str(waitForObjectExists(names.inputTextline3).text), "2")
    test.log("Clear display")
    tapObject(allClear)
