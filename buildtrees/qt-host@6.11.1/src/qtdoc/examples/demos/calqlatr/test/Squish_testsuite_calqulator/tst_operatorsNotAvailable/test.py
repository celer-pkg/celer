# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import names


def operator_button_not_available(button_list):
    for button in button_list:
        test.log(f"Check that {button.text} button has been dimmed (disabled before number press)")
        test.compare(button.dimmed, True)

def main():
    attachToApplication("calqlatr")

    button_list = (waitForObjectExists(names.backspaceButton),
                   waitForObjectExists(names.allClearButton),
                   waitForObjectExists(names.plusMinusButton),
                   waitForObjectExists(names.equalsButton),
                   waitForObjectExists(names.divisionButton),
                   waitForObjectExists(names.multiplicationButton),
                   waitForObjectExists(names.plusButton),
                   waitForObjectExists(names.minusButton),
                   waitForObjectExists(names.decimal))

    if waitForObjectExists(names.landscapeMode).visible:
        test.log("Landscape mode extra tests")
        scientific_button_list = (waitForObjectExists(names.squaringButton),
                       waitForObjectExists(names.denominatorButton),
                       waitForObjectExists(names.squareRootButton),
                       waitForObjectExists(names.cubedButton),
                       waitForObjectExists(names.sinButton),
                       waitForObjectExists(names.absValueButton),
                       waitForObjectExists(names.logarithmButton),
                       waitForObjectExists(names.cosButton),
                       waitForObjectExists(names.naturalLogarithmButton),
                       waitForObjectExists(names.tanButton))
        button_list = button_list + scientific_button_list

    operator_button_not_available(button_list)
