# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

qQuickWindowQmlImpl = {"type": "QQuickWindowQmlImpl"}
portraitMode = {"container": qQuickWindowQmlImpl, "id": "portraitMode", "type": "ColumnLayout"}
landscapeMode = {"container": qQuickWindowQmlImpl, "id": "landscapeMode", "type": "RowLayout"}

# Digit buttons
digitButton0 = {"checkable": False, "container": qQuickWindowQmlImpl, "text": 0, "type": "DigitButton"}
digitButton1 = {"checkable": False, "container": qQuickWindowQmlImpl, "text": 1, "type": "DigitButton"}
digitButton2 = {"checkable": False, "container": qQuickWindowQmlImpl, "text": 2, "type": "DigitButton"}
digitButton3 = {"checkable": False, "container": qQuickWindowQmlImpl, "text": 3, "type": "DigitButton"}
digitButton4 = {"checkable": False, "container": qQuickWindowQmlImpl, "text": 4, "type": "DigitButton"}
digitButton5 = {"checkable": False, "container": qQuickWindowQmlImpl, "text": 5, "type": "DigitButton"}
digitButton6 = {"checkable": False, "container": qQuickWindowQmlImpl, "text": 6, "type": "DigitButton"}
digitButton7 = {"checkable": False, "container": qQuickWindowQmlImpl, "text": 7, "type": "DigitButton"}
digitButton8 = {"checkable": False, "container": qQuickWindowQmlImpl, "text": 8, "type": "DigitButton"}
digitButton9 = {"checkable": False, "container": qQuickWindowQmlImpl, "text": 9, "type": "DigitButton"}
decimal = {"checkable": False, "container": qQuickWindowQmlImpl, "text": ".", "type": "DigitButton"}

# Main operations
backspaceButton = {"checkable": False, "container": qQuickWindowQmlImpl, "text": "bs", "type": "BackspaceButton"}
allClearButton = {"checkable": False, "container": qQuickWindowQmlImpl, "text": "AC", "type": "OperatorButton"}
plusMinusButton = {"checkable": False, "container": qQuickWindowQmlImpl, "text": "±", "type": "OperatorButton"}
equalsButton = {"checkable": False, "container": qQuickWindowQmlImpl, "text": "=", "type": "OperatorButton"}
divisionButton = {"checkable": False, "container": qQuickWindowQmlImpl, "text": "÷", "type": "OperatorButton"}
multiplicationButton = {"checkable": False, "container": qQuickWindowQmlImpl, "text": "×", "type": "OperatorButton"}
plusButton = {"checkable": False, "container": qQuickWindowQmlImpl, "text": "+", "type": "OperatorButton"}
minusButton = {"checkable": False, "container": qQuickWindowQmlImpl, "text": "−", "type": "OperatorButton"}

# Scientific Digit buttons
digitButtonEuler = {"checkable": False, "container": qQuickWindowQmlImpl, "text": "e", "type": "DigitButton"}
digitButtonPi = {"checkable": False, "container": qQuickWindowQmlImpl, "text": "π", "type": "DigitButton"}

# Scientific operations
squaringButton = {"checkable": False, "container": qQuickWindowQmlImpl, "text": "x²", "type": "OperatorButton"}
denominatorButton = {"checkable": False, "container": qQuickWindowQmlImpl, "text": "⅟x", "type": "OperatorButton"}
squareRootButton = {"checkable": False, "container": qQuickWindowQmlImpl, "text": "√", "type": "OperatorButton"}
cubedButton = {"checkable": False, "container": qQuickWindowQmlImpl, "text": "x³", "type": "OperatorButton"}
sinButton = {"checkable": False, "container": qQuickWindowQmlImpl, "text": "sin", "type": "OperatorButton"}
absValueButton = {"checkable": False, "container": qQuickWindowQmlImpl, "text": "|x|", "type": "OperatorButton"}
logarithmButton = {"checkable": False, "container": qQuickWindowQmlImpl, "text": "log", "type": "OperatorButton"}
cosButton = {"checkable": False, "container": qQuickWindowQmlImpl, "text": "cos", "type": "OperatorButton"}
naturalLogarithmButton = {"checkable": False, "container": qQuickWindowQmlImpl, "text": "ln", "type": "OperatorButton"}
tanButton = {"checkable": False, "container": qQuickWindowQmlImpl, "text": "tan", "type": "OperatorButton"}

# Display listview
calculationsListView = {"container": qQuickWindowQmlImpl, "id": "calculationsListView", "type": "ListView"}
line1ListViewItem = {"container": calculationsListView, "index": 0, "type": "Item"}
line2ListViewItem = {"container": calculationsListView, "index": 1, "type": "Item"}
line3ListViewItem = {"container": calculationsListView, "index": 2, "type": "Item"}
operatorTextline1 = {"container": line1ListViewItem, "type": "Text"}
inputTextline1 = {"container": line1ListViewItem, "occurrence": 2, "type": "Text"}
operatorTextline2 = {"container": line2ListViewItem, "type": "Text"}
inputTextline2 = {"container": line2ListViewItem, "occurrence": 2, "type": "Text"}
operatorTextline3 = {"container": line3ListViewItem, "type": "Text"}
inputTextline3 = {"container": line3ListViewItem, "occurrence": 2, "type": "Text"}
