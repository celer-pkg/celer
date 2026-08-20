# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import names


def main():
    attachToApplication("calqlatr")

    if waitForObjectExists(names.portraitMode).visible:
        test.log("Skip test case. Devices needs to be in Landscape mode")
    else:
        test.log("Landscape mode extra tests")

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
        plusMinus = waitForObjectExists(names.plusMinusButton)

        # Scientific Digit buttons
        euler = waitForObjectExists(names.digitButtonEuler)
        pi = waitForObjectExists(names.digitButtonPi)

        # Scientific operations
        squaring = waitForObjectExists(names.squaringButton)
        denominator = waitForObjectExists(names.denominatorButton)
        squareroot = waitForObjectExists(names.squareRootButton)
        cubed = waitForObjectExists(names.cubedButton)
        sin = waitForObjectExists(names.sinButton)
        absvalue = waitForObjectExists(names.absValueButton)
        logarithm = waitForObjectExists(names.logarithmButton)
        cos = waitForObjectExists(names.cosButton)
        naturallogarithm = waitForObjectExists(names.naturalLogarithmButton)
        tan = waitForObjectExists(names.tanButton)

        # Squared
        test.log("Check that listView shows corresponding number when digit button '8' is tapped")
        tapObject(button8)
        test.compare(str(waitForObjectExists(names.inputTextline1).text), "8")
        test.log("Check that corresponding symbol (x²) appears in display when Square operator button is tapped")
        tapObject(squaring)
        test.compare(str(waitForObjectExists(names.operatorTextline2).text), "x²")
        test.log("Check that corresponding value appears in display for the operation which should be '64'")
        test.compare(str(waitForObjectExists(names.inputTextline2).text), "64")
        test.log("Clear display")
        tapObject(allClear)

        # Denominator
        test.log("Check that listView shows corresponding number when digit button '2' is tapped")
        test.log("Check that corresponding value appears in display when first '0' adding decimal point and then '2' digit buttons are tapped")
        tapObject(button0)
        tapObject(decimal)
        tapObject(button2)
        test.compare(str(waitForObjectExists(names.inputTextline1).text), "0.2")
        test.log("Check that corresponding symbol (⅟x) appears in display when Denominator operator button is tapped")
        tapObject(denominator)
        test.compare(str(waitForObjectExists(names.operatorTextline2).text), "⅟x")
        test.log("Check that corresponding value appears in display for the operation which should be '5'")
        test.compare(str(waitForObjectExists(names.inputTextline2).text), "5")
        test.log("Clear display")
        tapObject(allClear)

        # Square Root
        test.log("Check that listView shows corresponding number when digit button '9' is tapped")
        tapObject(button9)
        test.compare(str(waitForObjectExists(names.inputTextline1).text), "9")
        test.log("Check that corresponding symbol (√) appears in display when Square Root operator button is tapped")
        tapObject(squareroot)
        test.compare(str(waitForObjectExists(names.operatorTextline2).text), "√")
        test.log("Check that corresponding value appears in display for the operation which should be '3'")
        test.compare(str(waitForObjectExists(names.inputTextline2).text), "3")
        test.log("Clear display")
        tapObject(allClear)

        # Cube
        test.log("Check that listView shows corresponding number when digit button '3' is tapped")
        tapObject(button3)
        test.compare(str(waitForObjectExists(names.inputTextline1).text), "3")
        test.log("Check that corresponding symbol (x³) appears in display when Cubed operator button is tapped")
        tapObject(cubed)
        test.compare(str(waitForObjectExists(names.operatorTextline2).text), "x³")
        test.log("Check that corresponding value appears in display for the operation which should be '27'")
        test.compare(str(waitForObjectExists(names.inputTextline2).text), "27")
        test.log("Clear display")
        tapObject(allClear)

        # Sine
        test.log("Check that listView shows corresponding number when digit button '1' is tapped")
        tapObject(button1)
        test.compare(str(waitForObjectExists(names.inputTextline1).text), "1")
        test.log("Check that corresponding symbol (sin) appears in display when Sine operator button is tapped")
        tapObject(sin)
        test.compare(str(waitForObjectExists(names.operatorTextline2).text), "sin")
        test.log("Check that corresponding value appears in display for the operation which should be '0.8414710'")
        test.compare(str(waitForObjectExists(names.inputTextline2).text), "0.8414710")
        test.log("Clear display")
        tapObject(allClear)

        # Absolute Value
        test.log("Check that corresponding value appears in display when first '7' and then '5' digit buttons are tapped")
        tapObject(button7)
        tapObject(button5)
        test.compare(str(waitForObjectExists(names.inputTextline1).text), "75")
        test.log("Check that corresponding number turns to negative when ± button is tapped")
        tapObject(plusMinus)
        test.compare(str(waitForObjectExists(names.inputTextline1).text), "-75")
        test.log("Check that corresponding symbol (|x|) appears in display when Absolute Value operator button is tapped")
        tapObject(absvalue)
        test.compare(str(waitForObjectExists(names.operatorTextline2).text), "|x|")
        test.log("Check that corresponding value appears in display for the operation which should be '75'")
        test.compare(str(waitForObjectExists(names.inputTextline2).text), "75")
        test.log("Clear display")
        tapObject(allClear)

        # Logarithm
        test.log("Check that listView shows corresponding number when digit button '6' is tapped")
        tapObject(button6)
        test.compare(str(waitForObjectExists(names.inputTextline1).text), "6")
        test.log("Check that corresponding symbol (log) appears in display when Logarithm operator button is tapped")
        tapObject(logarithm)
        test.compare(str(waitForObjectExists(names.operatorTextline2).text), "log")
        test.log("Check that corresponding value appears in display for the operation which should be '0.7781513'")
        test.compare(str(waitForObjectExists(names.inputTextline2).text), "0.7781513")
        test.log("Clear display")
        tapObject(allClear)

        # Cosine
        test.log("Check that listView shows corresponding number when digit button 'π' is tapped")
        tapObject(pi)
        test.compare(str(waitForObjectExists(names.inputTextline1).text), "3.1415927")
        test.log("Check that corresponding symbol (cos) appears in display when Cosine operator button is tapped")
        tapObject(cos)
        test.compare(str(waitForObjectExists(names.operatorTextline2).text), "cos")
        test.log("Check that corresponding value appears in display for the operation which should be '1'")
        test.compare(str(waitForObjectExists(names.inputTextline2).text), "-1.0000000")
        test.log("Clear display")
        tapObject(allClear)

        # Natural Logarithm
        test.log("Check that listView shows corresponding number when digit button 'e' is tapped")
        tapObject(euler)
        test.compare(str(waitForObjectExists(names.inputTextline1).text), "2.7182818")
        test.log("Check that corresponding symbol (ln) appears in display when Natural Logarithm operator button is tapped")
        tapObject(naturallogarithm)
        test.compare(str(waitForObjectExists(names.operatorTextline2).text), "ln")
        test.log("Check that corresponding value appears in display for the operation which should be '1'")
        test.compare(str(waitForObjectExists(names.inputTextline2).text), "1.0000000")
        test.log("Clear display")
        tapObject(allClear)

        # Tangent
        test.log("Check that listView shows corresponding number when digit button '4' is tapped")
        tapObject(button4)
        test.compare(str(waitForObjectExists(names.inputTextline1).text), "4")
        test.log("Check that corresponding symbol (tan) appears in display when Tangent operator button is tapped")
        tapObject(tan)
        test.compare(str(waitForObjectExists(names.operatorTextline2).text), "tan")
        test.log("Check that corresponding value appears in display for the operation which should be '1.1578213'")
        test.compare(str(waitForObjectExists(names.inputTextline2).text), "1.1578213")
        test.log("Clear display")
        tapObject(allClear)
