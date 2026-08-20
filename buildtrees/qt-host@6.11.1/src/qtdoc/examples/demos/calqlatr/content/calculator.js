// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

let accumulator = 0
let pendingOperator = ""
let lastButton = ""
let digits = ""

function isOperationDisabled(op, display) {
    if (digits !== "" && lastButton !== "=" && (op === "π" || op === "e"))
        return true
    if (digits === "" && !((op >= "0" && op <= "9") || op === "π" || op === "e" || op === "AC"))
        return true
    if (op === "bs" && (display.isOperandEmpty() || !((lastButton >= "0" && lastButton <= "9")
                                                      || lastButton === "π" || lastButton === "e" || lastButton === ".")))
        return true
    if (op === '=' && pendingOperator.length != 1)
        return true
    if (op === "." && digits.search(/\./) != -1)
        return true
    if (op === "√" &&  digits.search(/-/) != -1)
        return true
    if (op === "AC" && display.isDisplayEmpty())
        return true

    return false
}

function digitPressed(op, display) {
    if (isOperationDisabled(op, display))
        return
    if (lastButton === "π" || lastButton === "e")
        return
    // handle mathematical constants
    if (op === "π") {
        lastButton = op
        digits = Math.PI.toPrecision(display.maxDigits - 1).toString()
        display.appendDigit(digits)
        return
    }
    if (op === "e") {
        lastButton = op
        digits = Math.E.toPrecision(display.maxDigits - 1).toString()
        display.appendDigit(digits)
        return
    }

    // append a digit to another digit or decimal point
    if (lastButton.toString().length === 1 && ((lastButton >= "0" && lastButton <= "9") || lastButton === ".") ) {
        if (digits.length >= display.maxDigits)
            return
        digits = digits + op.toString()
        display.appendDigit(op.toString())
    // else just write a single digit to display
    } else {
        digits = op.toString()
        display.appendDigit(digits)
    }
    lastButton = op
}

function operatorPressed(op, display) {
    if (isOperationDisabled(op, display))
        return

    if (op === "±") {
        digits = Number(digits.valueOf() * -1).toString()
        display.setDigit(display.displayNumber(Number(digits)))
        return
    }

    if (op === "bs") {
        digits = digits.slice(0, -1)
        if (digits === "-")
            digits = ""
        display.backspace()
        return
    }

    lastButton = op

    if (pendingOperator === "+") {
        digits = (Number(accumulator) + Number(digits.valueOf())).toString()
    } else if (pendingOperator === "−") {
        digits = (Number(accumulator) - Number(digits.valueOf())).toString()
    } else if (pendingOperator === "×") {
        digits = (Number(accumulator) * Number(digits.valueOf())).toString()
    } else if (pendingOperator === "÷") {
        digits = (Number(accumulator) / Number(digits.valueOf())).toString()
    }


    if (op === "+" || op === "−" || op === "×" || op === "÷") {
        pendingOperator = op
        accumulator = digits.valueOf()
        digits = ""
        display.displayOperator(pendingOperator)
        return
    }

    accumulator = 0
    pendingOperator = ""

    if (op === "=") {
        display.newLine("=", Number(digits))
    }

    if (op === "√") {
        digits = (Math.sqrt(digits.valueOf())).toString()
        display.newLine("√", Number(digits))
    } else if (op === "⅟x") {
        digits = (1 / digits.valueOf()).toString()
        display.newLine("⅟x", Number(digits))
    } else if (op === "x²") {
        digits = (digits.valueOf() * digits.valueOf()).toString()
        display.newLine("x²", Number(digits))
    } else if (op === "x³") {
        digits = (digits.valueOf() * digits.valueOf() * digits.valueOf()).toString()
        display.newLine("x³", Number(digits))
    } else if (op === "|x|") {
        digits = (Math.abs(digits.valueOf())).toString()
        display.newLine("|x|", Number(digits))
    } else if (op === "⌊x⌋") {
        digits = (Math.floor(digits.valueOf())).toString()
        display.newLine("⌊x⌋", Number(digits))
    } else if (op === "sin") {
        digits = Number(Math.sin(digits.valueOf())).toString()
        display.newLine("sin", Number(digits))
    } else if (op === "cos") {
        digits = Number(Math.cos(digits.valueOf())).toString()
        display.newLine("cos", Number(digits))
    } else if (op === "tan") {
        digits = Number(Math.tan(digits.valueOf())).toString()
        display.newLine("tan", Number(digits))
    } else if (op === "log") {
        digits = Number(Math.log10(digits.valueOf())).toString()
        display.newLine("log", Number(digits))
    } else if (op === "ln") {
        digits = Number(Math.log(digits.valueOf())).toString()
        display.newLine("ln", Number(digits))
    }

    if (op === "AC") {
        display.allClear()
        accumulator = 0
        lastButton = ""
        digits = ""
        pendingOperator = ""
    }
}
