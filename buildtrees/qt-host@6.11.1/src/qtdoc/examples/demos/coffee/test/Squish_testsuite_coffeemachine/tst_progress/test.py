# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import names
import shared


def main():
    attachToApplication("coffeemachine")
    shared.toolbar(waitForObjectExists(names.toolBarBackButton), waitForObjectExists(names.toolBarThemeButton), False, True, 0, 1)
    shared.stackview(waitForObjectExists(names.applicationStackView), 5, "progress page")
    shared.testScreenBoundaries(waitForObject(names.applicationContentItem), waitForObject(names.progressGridLayout),
                                waitForObject(names.coffeeToolBar), coffeeText = waitForObject(names.coffeeHeaderText))
    test.log("Test that progress page has active focus on")
    test.compare(waitForObjectExists(names.coffeeProgressPage).activeFocus, True)
    progressBarWidth = waitForObject(names.progressBar).width
    test.log("Wait 1 sec so that progress bar length can be saved at 2 different points in time, which then should have different value")
    snooze(1)
    progressBarWidth2 = waitForObject(names.progressBar).width
    test.log("Verify that progress bar has made progress")
    test.verify(progressBarWidth < progressBarWidth2)
