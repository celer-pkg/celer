# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import names
import shared


def main():
    attachToApplication("coffeemachine")
    test.log("Wait for ready page grid object to synchronize test script to application")
    waitForObject(names.coffeeReadyPage)
    shared.toolbar(waitForObjectExists(names.toolBarBackButton), waitForObjectExists(names.toolBarThemeButton), False, True, 0, 1)
    shared.stackview(waitForObjectExists(names.applicationStackView), 6, "ready page")
    shared.testScreenBoundaries(waitForObject(names.applicationContentItem), waitForObject(names.readyGridLayout),
                                waitForObject(names.coffeeToolBar), coffeeText = waitForObject(names.coffeeHeaderText))
    test.log("Test that ready page has active focus on")
    test.compare(waitForObjectExists(names.coffeeReadyPage).activeFocus, True)
    test.log("Test that coffee cup coffeeAmount property value equals 100")
    test.compare(waitForObjectExists(names.readyCupImageCollection).coffeeAmount, 100)
    test.log("Test that coffee cup milkAmount property value equals 5")
    test.compare(waitForObjectExists(names.readyCupImageCollection).milkAmount, 5)
    test.log("Test that coffee cup foamAmount property value equals 10")
    test.compare(waitForObjectExists(names.readyCupImageCollection).foamAmount, 10)
    test.log("Test that coffee cup sugarAmount property value equals 0.0")
    test.compare(waitForObjectExists(names.readyCupImageCollection).sugarAmount, 0.0)
