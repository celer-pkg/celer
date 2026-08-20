// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
import QtQuick

Item {
    id: root
    property int coffeeAmount
    property int milkAmount
    property int foamAmount
    property double sugarAmount
    property alias coffee: coffee

    Image {
        id: foam
        source: "./images/Ingredients/Milk_foam.svg"
        anchors.bottom: cup.bottom
        sourceSize.width: cup.width
        anchors.horizontalCenter: parent.horizontalCenter
        sourceSize.height: milk.height + (root.foamAmount * (cup.height / 700))
    }
    Image {
        id: milk
        source: "./images/Ingredients/milk.svg"
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: cup.bottom
        sourceSize.width: cup.width
        sourceSize.height: coffee.height + (root.milkAmount * (cup.height / 700))
    }
    Image {
        id: coffee
        source: "./images/Ingredients/espresso_coffee.svg"
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: cup.bottom
        sourceSize.width: cup.width
        sourceSize.height: (cup.height / 6.4) + (root.coffeeAmount * (cup.height / 550))
    }
    Image {
        id: sugar
        source: "./images/Ingredients/sugar.svg"
        anchors.horizontalCenter: cup.horizontalCenter
        anchors.top: coffee.top
        sourceSize.width: cup.width / 6
        opacity: root.sugarAmount
    }
    Image {
        id: cup
        source: (Colors.currentTheme == Colors.dark) ? "./images/Cups/dark_cup.svgz" : "./images/Cups/light_cup.svgz"
        sourceSize.width: parent.width
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
    }
}
