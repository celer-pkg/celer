# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import names
import shared
from dataclasses import dataclass


@dataclass
class Slider:
    sliderObject: object
    sliderHandleObject: dict
    name: str
    value: int = 0
    def setSliderValue(self, value):
        test.log(f"Set {self.name} to {value}")
        waitForObject(self.sliderObject).value = value
        self.value = value

    def dragSlider(self, dragCount):
        slider = waitForObject(self.sliderObject)
        sliderHandle = waitForObject(self.sliderHandleObject)
        dragDistance = int(slider.width/dragCount)
        test.log(f"Drag {self.name} forwards")
        touchAndDrag(sliderHandle, 0, sliderHandle.height/2, dragDistance, sliderHandle.height/2)
        self.value = slider.value

def testSliders():
    coffeeSlider = Slider(waitForObject(names.sliderCoffee), names.sliderCoffeeHandle, "coffee slider")
    milkSlider = Slider(waitForObject(names.sliderMilk), names.sliderMilkHandle, "milk slider")
    foamSlider = Slider(waitForObject(names.sliderFoam), names.sliderFoamHandle, "foam slider")
    sugarSlider = Slider(waitForObject(names.sliderSugar), names.sliderSugarHandle, "sugar slider")
    sliders = [coffeeSlider, milkSlider, foamSlider, sugarSlider]
    for slider in sliders:
        if slider.name == "sugar slider":
            for _ in range (2):
                slider.dragSlider(2)
                shared.testSliderValue(slider.sliderObject, slider.name, slider.value)
                test.log("Test that sugar image opacity reflects the current sugar amount, opacity should be exactly the same as sugar slider value")
                test.compare(waitForObject(names.sugarImage).opacity, slider.value)
        else:
            slider.setSliderValue(0)
            for _ in range(3):
                coffeeImageHeight = waitForObject(names.coffeeImage).height
                milkImageHeight = waitForObject(names.milkImage).height
                foamImageHeight = waitForObject(names.foamImage).height
                slider.dragSlider(3)
                shared.testSliderValue(slider.sliderObject, slider.name, slider.value)
                if slider.name == "coffee slider":
                    test.log("Verify that coffee image height has increased since the coffee slider value was increased")
                    test.verify(waitForObject(names.coffeeImage).height > coffeeImageHeight)
                elif slider.name == "milk slider":
                    test.log("Verify that milk image height has increased since the milk slider value was increased")
                    test.verify(waitForObject(names.milkImage).height > milkImageHeight)
                elif slider.name == "foam slider":
                    test.log("Verify that foam image height has increased since the foam slider value was increased")
                    test.verify(waitForObject(names.foamImage).height > foamImageHeight)

def main():
    attachToApplication("coffeemachine")
    shared.toolbar(waitForObjectExists(names.toolBarBackButton), waitForObjectExists(names.toolBarThemeButton), True, True, 1, 1)
    shared.stackview(waitForObjectExists(names.applicationStackView), 3, "settings page")
    test.log("Test that Settings page state is same as screen orientation")
    shared.testPageStateOrientation(waitForObjectExists(names.coffeeHomePage))
    test.log("Test that ApplicationFlow coffeName text object text is 'Macchiato'")
    test.compare(waitForObjectExists(names.coffeeHeaderText).text, "Macchiato")
    testSliders()
    shared.testScreenBoundaries(waitForObject(names.applicationContentItem), waitForObject(names.settingsGridLayout),
                                waitForObject(names.coffeeToolBar), coffeeText = waitForObject(names.coffeeHeaderText))
    test.log("Tap start button to move to insert page")
    tapObject(waitForObject(names.settingsConfirmButton))
