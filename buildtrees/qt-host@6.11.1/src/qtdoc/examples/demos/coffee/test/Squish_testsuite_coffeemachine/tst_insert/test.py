# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import names
import shared


def testCancelButton():
    test.log("Tap cancel button to move back to coffee selection page")
    tapObject(waitForObject(names.insertCancelButton))
    test.log("Test that stack view depth decreases to 2")
    test.compare(waitForObjectExists(names.applicationStackView).depth, 2)
    test.log("Test that coffee selection page has the active focus on")
    test.compare(waitForObjectExists(names.coffeeChoosingPage).activeFocus, True)
    test.log("Tap macchiato coffee card button in coffee selection page")
    tapObject(waitForObject(names.macchiatoButton))
    test.log("Tap confirm button in Settings page")
    tapObject(waitForObject(names.settingsConfirmButton))

def main():
    attachToApplication("coffeemachine")
    shared.toolbar(waitForObjectExists(names.toolBarBackButton), waitForObjectExists(names.toolBarThemeButton), True, True, 1, 1)
    shared.stackview(waitForObjectExists(names.applicationStackView), 4, "insert page")
    test.log("Test that Insert page state is same as screen orientation")
    shared.testPageStateOrientation(waitForObjectExists(names.coffeeHomePage))
    test.log("Test that cup image is visible")
    test.compare(waitForObjectExists(names.insertCupImageCollection).visible, True)
    test.log("Test that dialog is visible")
    test.compare(waitForObjectExists(names.insertDialog).visible, True)
    test.log("Test that text inside dialog is visible")
    test.compare(waitForObjectExists(names.insertDialogText).text, "Please insert your cup.")
    test.log("Test that continue button is visible")
    test.compare(waitForObjectExists(names.insertContinueButton).visible, True)
    test.log("Test that cancel button is visible")
    test.compare(waitForObjectExists(names.insertCancelButton).visible, True)
    shared.testScreenBoundaries(waitForObject(names.applicationContentItem), waitForObject(names.insertGridLayout),
                                waitForObject(names.coffeeToolBar), coffeeText = waitForObject(names.coffeeHeaderText))
    testCancelButton()
    test.log("Tap continue button to move to insert progress page")
    tapObject(waitForObject(names.insertContinueButton))
