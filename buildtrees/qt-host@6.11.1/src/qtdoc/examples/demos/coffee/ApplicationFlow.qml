// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
import QtQuick

ApplicationFlowForm {
    id: applicationFlow
    state: "Home"
    property int animationDuration: 400
    property string platform: Qt.platform.os

    CoffeeConfig {
        id: coffeeConfig
    }



    stack.initialItem: Home {
        id: home
        visible: true
        //! [On clicked]
        getStartedbutton.onClicked: {
            applicationFlow.state = "Coffee-selection"
            applicationFlow.stack.pushItem(applicationFlow.choosingCoffee, {appFlow: applicationFlow})
        }
        //! [On clicked]
    }

    //! [Theme button]
    function themeButton() {
        if (Colors.currentTheme == Colors.dark) {
            Colors.currentTheme = Colors.light
        } else {
            Colors.currentTheme = Colors.dark
        }
    }
    //! [Theme button]
    function cappuccino() {
        applicationFlow.state = "Settings"
        applicationFlow.coffeeName = "Cappuccino"
        coffeeConfig.coffeeAmount = 60
        coffeeConfig.milkAmount = 60
        coffeeConfig.foamAmount = 60
        coffeeConfig.brewTime = 5000
        coffeeConfig.sugarAmount = 0
        stack.pushItem(settings, {appFlow: applicationFlow, coffeeConfig})
        coffeeText.text = "Cappuccino"
    }
    function espresso() {
        applicationFlow.state = "Settings"
        applicationFlow.coffeeName = "Espresso"
        coffeeConfig.coffeeAmount = 80
        coffeeConfig.milkAmount = 0
        coffeeConfig.foamAmount = 0
        coffeeConfig.brewTime = 4000
        coffeeConfig.sugarAmount = 0
        stack.pushItem(settings, {appFlow: applicationFlow, coffeeConfig})
        coffeeText.text = "Espresso"
    }
    function latte() {
        applicationFlow.state = "Settings"
        applicationFlow.coffeeName = "Latte"
        coffeeConfig.coffeeAmount = 40
        coffeeConfig.milkAmount = 20
        coffeeConfig.foamAmount = 60
        coffeeConfig.brewTime = 6000
        coffeeConfig.sugarAmount = 0
        stack.pushItem(settings, {appFlow: applicationFlow, coffeeConfig})
        coffeeText.text = "Latte"
    }
    function macchiato() {
        applicationFlow.state = "Settings"
        applicationFlow.coffeeName = "Macchiato"
        coffeeConfig.coffeeAmount = 100
        coffeeConfig.milkAmount = 5
        coffeeConfig.foamAmount = 10
        coffeeConfig.brewTime = 8000
        coffeeConfig.sugarAmount = 0
        stack.pushItem(settings, {appFlow: applicationFlow, coffeeConfig})
        coffeeText.text = "Macchiato"
    }
    function backButton() {
        stack.pop()
        applicationFlow.state = applicationFlow.previousState
    }
    function confirmButton() {
        stack.pushItem(insert, {appFlow: applicationFlow})
        applicationFlow.state = "Insert"
    }
    function continueButton() {
        stack.pushItem(progress, {appFlow: applicationFlow, coffeeConfig})
        applicationFlow.state = "Progress"
        applicationFlow.progressBarValue = 1
        applicationFlow.progressCupState = "1"
        if (applicationFlow.coffeeName == "Cappuccino") {
            applicationFlow.cappuccinos = applicationFlow.cappuccinos - 1
        } else if (applicationFlow.coffeeName == "Espresso") {
            applicationFlow.espressos = applicationFlow.espressos - 1
        } else if (applicationFlow.coffeeName == "Latte") {
            applicationFlow.lattes = applicationFlow.lattes - 1
        } else {
            applicationFlow.macchiatos = applicationFlow.macchiatos - 1
        }
    }
    function cancelButton() {
        applicationFlow.state = "Coffee-selection"
        stack.pop(stack.get(1))
    }
    function onFinished() {
        stack.pushItem(ready, {appFlow: applicationFlow, coffeeConfig})
        applicationFlow.state = "Ready"
    }
    function onReturnToStart() {
        stack.pop(stack.get(0))
        applicationFlow.state = "Home"
        applicationFlow.progressBarValue = 0
        applicationFlow.progressCupState = "0"
    }

    toolbar.onBackClicked: applicationFlow.backButton()
    toolbar.onThemeChangeRequested: applicationFlow.themeButton()

    //! [States]
    states: [
        State {
            name: "Home"
            PropertyChanges {
                target: toolbar
                backButton.opacity: 0
                backButton.enabled: false
                themeButton.opacity: 0
                themeButton.enabled: false
                logo.sourceSize.width: 70
                logo.sourceSize.height: 50
            }
            //! [States]
            PropertyChanges {
                target: coffeeText
                visible: false
            }
            PropertyChanges {
                target: stack
                anchors.top: coffeeText.bottom
            }
        },
        State {
            name: "Coffee-selection"
            PropertyChanges {
                target: applicationFlow
                previousState: "Home"

            }
            PropertyChanges {
                target: coffeeText
                text: "Coffee Selection"
            }
            PropertyChanges {
                target: toolbar
                backButton.opacity: 0
                backButton.enabled: false
            }
            PropertyChanges {
                target: stack
                anchors.top: coffeeText.bottom
            }
        },
        State {
            name: "Settings"
            PropertyChanges {
                target: applicationFlow
                previousState: "Coffee-selection"
            }
            PropertyChanges {
                target: stack
                anchors.top: coffeeText.bottom
            }
        },
        State {
            name: "Insert"
            PropertyChanges {
                target: applicationFlow
                previousState: "Settings"
            }
            PropertyChanges {
                target: stack
                anchors.top: coffeeText.bottom
            }
        },
        State {
            name: "Progress"
            PropertyChanges {
                target: applicationFlow
                previousState: "Insert"

            }
            PropertyChanges {
                target: toolbar
                backButton.opacity: 0
                backButton.enabled: false
            }
            PropertyChanges {
                target: stack
                anchors.top: coffeeText.bottom
            }
        },
        State {
            name: "Ready"
            PropertyChanges {
                target: applicationFlow
                previousState: "Progress"

            }
            PropertyChanges {
                target: toolbar
                backButton.opacity: 0
                backButton.enabled: false
            }
            PropertyChanges {
                target: stack
                anchors.top: coffeeText.bottom
            }
        }

    ]
}
