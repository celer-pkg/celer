// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

ChoosingCoffeeForm {
    required property ApplicationFlow appFlow
    state: Config.mode
    cappuccino.cupsLeft: appFlow.cappuccinos
    latte.cupsLeft: appFlow.lattes
    espresso.cupsLeft: appFlow.espressos
    macchiato.cupsLeft: appFlow.macchiatos

    cappuccino.button.onClicked: appFlow.cappuccino()
    macchiato.button.onClicked: appFlow.macchiato()
    espresso.button.onClicked: appFlow.espresso()
    latte.button.onClicked: appFlow.latte()
}
