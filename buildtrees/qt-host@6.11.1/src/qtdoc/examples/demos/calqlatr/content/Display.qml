// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

pragma ComponentBehavior: Bound

import QtQuick

Item {
    id: display
    property int fontSize: 22
    readonly property int maxDigits: Math.min((width / fontSize) + 1, 9)
    readonly property color backgroundColor: "#262626"
    readonly property color qtGreenColor: "#2CDE85"
    property string displayedOperand: ""
    readonly property string errorString: qsTr("ERROR")
    readonly property bool isError: displayedOperand === errorString
    property bool enteringDigits: false

    function displayOperator(operator) {
        calculationsListView.model.append({
                                              "operator": operator,
                                              "operand": ""
                                          });
        enteringDigits = true;
        calculationsListView.positionViewAtEnd();
    }

    function newLine(operator, operand) {
        displayedOperand = displayNumber(operand);
        calculationsListView.model.append({
                                              "operator": operator,
                                              "operand": displayedOperand
                                          });
        enteringDigits = false;
        calculationsListView.positionViewAtEnd();
    }

    function appendDigit(digit) {
        if (!enteringDigits)
            calculationsListView.model.append({
                                                  "operator": "",
                                                  "operand": ""
                                              });
        const i = calculationsListView.model.count - 1;
        calculationsListView.model.get(i).operand = calculationsListView.model.get(i).operand
                + digit;
        enteringDigits = true;
        calculationsListView.positionViewAtEnd();
    }

    function setDigit(digit) {
        const i = calculationsListView.model.count - 1;
        calculationsListView.model.get(i).operand = digit;
        calculationsListView.positionViewAtEnd();
    }

    function backspace() {
        const i = calculationsListView.model.count - 1;
        if (i >= 0) {
            let operand = calculationsListView.model.get(i).operand.toString().slice(0, -1);
            if (operand === "-")
                operand = "";
            calculationsListView.model.get(i).operand = operand;
            return;
        }
        return;
    }

    function isOperandEmpty() {
        const i = calculationsListView.model.count - 1;
        return i >= 0 ? calculationsListView.model.get(i).operand === "" : true;
    }

    function isDisplayEmpty() {
        const i = calculationsListView.model.count - 1;
        return i == -1 ? true : (i == 0 ? calculationsListView.model.get(0).operand === "" : false);
    }

    function clear() {
        displayedOperand = "";
        if (enteringDigits) {
            const i = calculationsListView.model.count - 1;
            if (i >= 0)
                calculationsListView.model.remove(i);
            enteringDigits = false;
        }
    }

    function allClear() {
        display.clear();
        calculationsListView.model.clear();
        enteringDigits = false;
    }

    // Returns a string representation of a number that fits in
    // display.maxDigits characters, trying to keep as much precision
    // as possible. If the number cannot be displayed, returns an
    // error string.
    function displayNumber(num) {
        if (typeof (num) !== "number")
            return errorString;

        // deal with the absolute
        const abs = Math.abs(num);

        if (abs.toString().length <= maxDigits) {
            return isFinite(num) ? num.toString() : errorString;
        }

        if (abs < 1) {
            // check if abs < 0.00001, if true, use exponential form
            // if it isn't true, we can round the number without losing
            // too much precision
            if (Math.floor(abs * 100000) === 0) {
                const expVal = num.toExponential(maxDigits - 6).toString();
                if (expVal.length <= maxDigits + 1)
                    return expVal;
            } else {
                // the first two digits are zero and .
                return num.toFixed(maxDigits - 2);
            }
        } else {
            // if the integer part of num is greater than maxDigits characters, use exp form
            const intAbs = Math.floor(abs);
            if (intAbs.toString().length <= maxDigits)
                return parseFloat(num.toPrecision(maxDigits - 1)).toString();

            const expVal = num.toExponential(maxDigits - 6).toString();
            if (expVal.length <= maxDigits + 1)
                return expVal;
        }
        return errorString;
    }

    Item {
        anchors.fill: parent

        Rectangle {
            anchors.fill: parent
            radius: 8
            color: display.backgroundColor

            ListView {
                id: calculationsListView
                x: 5
                y: 10
                width: parent.width
                height: parent.height - 2 * y
                clip: true
                delegate: Item {
                    height: display.fontSize * 1.1
                    width: calculationsListView.width

                    required property string operator
                    required property string operand

                    Text {
                        x: 6
                        font.pixelSize: display.fontSize
                        color: display.qtGreenColor
                        text: parent.operator
                        Accessible.name: parent.operator
                    }
                    Text {
                        font.pixelSize: display.fontSize
                        anchors.right: parent.right
                        anchors.rightMargin: 16
                        text: parent.operand
                        Accessible.name: parent.operand
                        color: "white"
                    }
                }
                model: ListModel {}
                onHeightChanged: positionViewAtEnd()
            }
        }
    }
}
