# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import names
import shared
from dataclasses import dataclass


@dataclass
class Coffee():
    name: str
    coffeeAmount: int
    milkAmount: int
    foamAmount: int
    sugarAmount: int
    button: object

def testCoffees():
    cappuccino = Coffee("cappuccino", 60, 60 ,60, 0, waitForObjectExists(names.cappuccinoButton))
    latte = Coffee("latte", 40, 20, 60, 0, waitForObjectExists(names.latteButton))
    espresso = Coffee("espresso", 80, 0, 0, 0, waitForObjectExists(names.espressoButton))
    macchiato = Coffee("macchiato", 100, 5, 10, 0, waitForObjectExists(names.macchiatoButton))
    coffeeList = [cappuccino, latte, espresso, macchiato]

    for coffee in coffeeList:
        test.log(f"Set shared.py coffee variable to {coffee.name}")
        shared.coffee = coffee.name
        test.log(f"Tap {coffee.name} card")
        tapObject(coffee.button)
        snooze(1)
        shared.settings(waitForObjectExists(names.coffeeSettingsPage))
        shared.testCoffeeDefaultValues(coffee.coffeeAmount, coffee.milkAmount, coffee.foamAmount, coffee.sugarAmount)
        shared.testSliderValue(waitForObject(names.sliderCoffee), "coffee slider", coffee.coffeeAmount)
        shared.testSliderValue(waitForObject(names.sliderMilk), "milk slider", coffee.milkAmount)
        shared.testSliderValue(waitForObject(names.sliderFoam), "foam slider", coffee.foamAmount)
        test.log("Tap back button to move back to coffee selection page")
        tapObject(waitForObject(names.toolBarBackButton))
        snooze(1)

def main():
    attachToApplication("coffeemachine")
    shared.toolbar(waitForObjectExists(names.toolBarBackButton), waitForObjectExists(names.toolBarThemeButton), False, True, 0, 1)
    shared.stackview(waitForObjectExists(names.applicationStackView), 2, "coffee selection page")
    test.log("Test that choosing coffee page state is same as screen orientation")
    shared.testPageStateOrientation(waitForObjectExists(names.coffeeHomePage))
    test.log("Test that cappuccino coffee card is visible")
    test.compare(waitForObjectExists(names.cappuccinoCard).visible, True)
    test.log("Test that button inside cappuccino coffee card is enabled")
    test.compare(waitForObjectExists(names.cappuccinoButton).enabled, True)
    test.log("Test that latte coffee card is visible")
    test.compare(waitForObjectExists(names.latteCard).visible, True)
    test.log("Test that button inside latte coffee card is enabled")
    test.compare(waitForObjectExists(names.latteButton).enabled, True)
    test.log("Test that espresso coffee card is visible")
    test.compare(waitForObjectExists(names.espressoCard).visible, True)
    test.log("Test that button inside espresso coffee card is enabled")
    test.compare(waitForObjectExists(names.espressoButton).enabled, True)
    test.log("Test that macchiato coffee card is visible")
    test.compare(waitForObjectExists(names.macchiatoCard).visible, True)
    test.log("Test that button inside macchiato coffee card is enabled")
    test.compare(waitForObjectExists(names.macchiatoButton).enabled, True)
    test.log("Test that there are 4 cappuccinos left")
    test.compare(waitForObjectExists(names.applicationFlow).cappuccinos, 4)
    test.log("Test that there are 5 lattes left")
    test.compare(waitForObjectExists(names.applicationFlow).lattes, 5)
    test.log("Test that there are 6 espressos left")
    test.compare(waitForObjectExists(names.applicationFlow).espressos, 6)
    test.log("Test that there are 4 macchiatos left")
    test.compare(waitForObjectExists(names.applicationFlow).macchiatos, 4)
    testCoffees()
    shared.testScreenBoundaries(waitForObject(names.applicationContentItem), waitForObject(names.choosingCardsGridLayout),
                                waitForObject(names.coffeeToolBar), coffeeText = waitForObject(names.coffeeHeaderText))
    test.log("Tap macchiato coffee card to move to Settings page testing")
    tapObject(waitForObject(names.macchiatoButton))
