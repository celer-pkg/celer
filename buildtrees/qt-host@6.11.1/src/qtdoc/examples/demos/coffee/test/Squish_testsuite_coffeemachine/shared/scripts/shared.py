# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

from squish import waitForObject
import names
import test


def toolbar(backButton, themeButton, backButtonEnabled, themeButtonEnabled, backButtonOpacity, themeButtonOpacity):
    test.log(f"Test that back button enabled property equals to {backButtonEnabled}")
    test.compare(backButton.enabled, backButtonEnabled)
    test.log(f"Test that theme button enabled property equals to {themeButtonEnabled}")
    test.compare(themeButton.enabled, themeButtonEnabled)
    test.log(f"Test that back button opacity is {backButtonOpacity}, 0 meaning completely transparent and 1 meaning not transparent")
    test.compare(backButton.opacity, backButtonOpacity)
    test.log(f"Test that theme button opacity is {themeButtonOpacity}")
    test.compare(themeButton.opacity, themeButtonOpacity)

def stackview(stackView, depth, page):
    test.log(f"Test that stack view depth is {depth} after moving to {page}")
    test.compare(stackView.depth, depth)

def settings(settingsPage):
    test.log(f"Test that settings page is visible after selecting {coffee}")
    test.compare(settingsPage.visible, True)

def testSliderValue(slider, sliderName, value):
    test.log(f"Test that {sliderName} value matches {value}")
    test.compare(slider.value, value)

def testCoffeeDefaultValues(coffeeAmount, milkAmount, foamAmount, sugarAmount):
    coffeePage = waitForObject(names.coffeeSettingsPage)
    test.log(f"Test that Pages coffeeAmount property value updates to {coffeeAmount}")
    test.compare(coffeePage.coffeeAmount, coffeeAmount)
    test.log(f"Test that Pages milkAmount property value updates to {milkAmount}")
    test.compare(coffeePage.milkAmount, milkAmount)
    test.log(f"Test that Pages foamAmount property value updates to {foamAmount}")
    test.compare(coffeePage.foamAmount, foamAmount)
    test.log(f"Test that Pages sugarAmount property value updates to {sugarAmount}")
    test.compare(coffeePage.sugarAmount, sugarAmount)


def testScreenBoundaries(applicationWindow, grid, toolbar, **kwargs):
    coffeeText = kwargs.get('coffeeText')
    if coffeeText is None:
            test.log("Test that application window height is greater than or equal to all the object heights combined inside it")
            test.verify(applicationWindow.height >= grid.height + toolbar.height)
    else:
        test.log("Test that application window height is greater than or equal to all the object heights combined inside it")
        test.verify(applicationWindow.height >= grid.height + coffeeText.height + toolbar.height)
        test.log(f"{applicationWindow.height} and {grid.height + coffeeText.height + toolbar.height}")
    test.log("Test that application window width is greater than or equal to grid width")
    test.verify(applicationWindow.width >= grid.width)

def testPageStateOrientation(page):
    screenOrientationValue = waitForObject(names.coffeeQQuickApplicationWindow).screen.orientation
    pageOrientationState = page.state
    orientationTestPass = False
    logValues = f"(State: '{pageOrientationState}', Value: '{screenOrientationValue}')"
    logDetails = "State portrait should have value 1 and landscape 2 or 8."
    test.log("Check that page state matches with Android screen orientation value")
    if pageOrientationState == 'landscape' and screenOrientationValue in (2, 8):
        orientationTestPass = True
    elif pageOrientationState == 'portrait' and screenOrientationValue == 1:
        orientationTestPass = True
    if orientationTestPass:
        test.passes(f"Page state matches with Android screen orientation value. {logValues}", logDetails)
    else:
        test.fail(f"Page state and Android screen orientation value mismatch. {logValues}", logDetails)
