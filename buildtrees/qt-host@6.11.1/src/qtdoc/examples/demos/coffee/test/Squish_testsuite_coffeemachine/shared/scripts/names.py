# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

# ALL
coffeeQQuickApplicationWindow = {"title": "Coffee", "type": "QQuickApplicationWindow"}
applicationContentItem = {"container": coffeeQQuickApplicationWindow, "objectName": "ApplicationWindow", "type": "ContentItem"}
applicationFlow = {"container": applicationContentItem, "type": "ApplicationFlow"}
applicationStackView = {"container": applicationFlow, "id": "stack", "type": "StackView"}
coffeeToolBar = {"container": applicationFlow, "id": "toolbar", "type": "CustomToolBar"}
toolBarBackButton = {"container": coffeeToolBar, "id": "backButton", "type": "Button"}
toolBarThemeButton = {"container": coffeeToolBar, "id": "themeButton", "type": "Button"}
coffeeHeaderText = {"container": applicationFlow, "id": "coffeeText", "type": "Text"}

# Coffee Home Page
coffeeHomePage = {"container": applicationStackView, "id": "home", "type": "Home"}
homeGridLayout = {"container": coffeeHomePage, "id": "grid", "type": "GridLayout"}
homeGetStartedButton = {"container": homeGridLayout, "id": "getStartedButton", "type": "CustomButton"}
homeGetStartedButtonText = {"container": homeGetStartedButton, "text": "Get Started", "type": "Text"}

# Choosing Coffee Page
coffeeChoosingPage = {"container": applicationStackView, "type": "ChoosingCoffee"}
choosingCardsGridLayout = {"container": coffeeChoosingPage, "id": "cards", "type": "GridLayout"}
cappuccinoCard = {"container": choosingCardsGridLayout, "id": "cappuccino", "type": "CoffeeCard"}
cappuccinoButton = {"container": cappuccinoCard, "id": "button", "type": "AbstractButton"}
latteCard = {"container": choosingCardsGridLayout, "id": "latte", "type": "CoffeeCard"}
latteButton = {"container": latteCard, "id": "button", "type": "AbstractButton"}
espressoCard = {"container": choosingCardsGridLayout, "id": "espresso", "type": "CoffeeCard"}
espressoButton = {"container": espressoCard, "id": "button", "type": "AbstractButton"}
macchiatoCard = {"container": choosingCardsGridLayout, "id": "macchiato", "type": "CoffeeCard"}
macchiatoButton = {"container": macchiatoCard, "id": "button", "type": "AbstractButton"}

# Coffee Settings Page
coffeeSettingsPage = {"container": applicationStackView, "type": "Settings"}
settingsGridLayout = {"container": coffeeSettingsPage, "id": "grid", "type": "GridLayout"}
settingsSliderLayout = {"container": settingsGridLayout, "type": "ColumnLayout"}
sliderCoffee = {"container": settingsSliderLayout, "id": "coffeeSlider", "type": "CustomSlider"}
sliderCoffeeHandle = {"container": sliderCoffee, "id": "handle", "type": "Rectangle"}
sliderMilk = {"container": settingsSliderLayout, "id": "milkSlider", "type": "CustomSlider"}
sliderMilkHandle = {"container": sliderMilk, "id": "handle", "type": "Rectangle"}
sliderFoam = {"container": settingsSliderLayout, "id": "foamSlider", "type": "CustomSlider"}
sliderFoamHandle = {"container": sliderFoam, "id": "handle", "type": "Rectangle"}
sliderSugar = {"container": settingsSliderLayout, "id": "sugarSlider", "type": "Slider"}
sliderSugarHandle = {"container": sliderSugar, "id": "handle", "type": "Rectangle"}
settingsCupImageCollection = {"container": settingsGridLayout, "id": "cup", "type": "Cup.ui"}
coffeeImage = {"container": settingsCupImageCollection, "id": "coffee", "type": "Image"}
milkImage = {"container": settingsCupImageCollection, "id": "milk", "type": "Image"}
foamImage = {"container": settingsCupImageCollection, "id": "foam", "type": "Image"}
sugarImage = {"container": settingsCupImageCollection, "id": "sugar", "type": "Image"}
settingsConfirmButton = {"container": settingsGridLayout, "id": "confirmButton", "type": "CustomButton"}

# Insert Coffee Page
coffeeInsertPage = {"container": applicationStackView, "type": "Insert"}
insertGridLayout = {"container": coffeeInsertPage, "id": "grid", "type": "GridLayout"}
insertCupImageCollection = {"container": insertGridLayout, "id": "cup", "type": "Cup.ui"}
insertDialog = {"container": insertGridLayout, "id": "dialog", "type": "Rectangle"}
insertDialogText = {"container": insertDialog, "type": "Text"}
insertContinueButton = {"container": insertGridLayout, "id": "continueButton", "type": "CustomButton"}
insertCancelButton = {"container": insertGridLayout, "id": "cancelButton", "type": "CustomButton"}

# Coffee Progress Page
coffeeProgressPage = {"container": applicationStackView, "type": "Progress"}
progressGridLayout = {"container": coffeeProgressPage, "id": "grid", "type": "GridLayout"}
progressBar = {"container": progressGridLayout, "id": "greenBar", "type": "Rectangle"}

# Coffee Ready Page
coffeeReadyPage = {"container": applicationStackView, "type": "Ready"}
readyGridLayout = {"container": coffeeReadyPage, "id": "grid", "type": "GridLayout"}
readyCupImageCollection = {"container": readyGridLayout, "id": "cup", "type": "Cup.ui"}
