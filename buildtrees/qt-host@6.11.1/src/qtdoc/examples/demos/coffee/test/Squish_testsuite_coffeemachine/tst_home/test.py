# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import names
import shared


def main():
    attachToApplication("coffeemachine")
    shared.toolbar(waitForObjectExists(names.toolBarBackButton), waitForObjectExists(names.toolBarThemeButton), False, False, 0, 0)
    shared.stackview(waitForObjectExists(names.applicationStackView), 1, "home page")
    test.log("Test that home page state is the same as screen orientation")
    shared.testPageStateOrientation(waitForObjectExists(names.coffeeHomePage))
    shared.testScreenBoundaries(waitForObject(names.applicationContentItem), waitForObject(names.homeGridLayout), waitForObject(names.coffeeToolBar))
    test.log("Tap 'Get Started' button and move to coffee selection page")
    tapObject(waitForObject(names.homeGetStartedButton))
