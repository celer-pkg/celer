// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
import QtQuick
import QtQuick.Layouts

CoffeeCardForm {
    id: coffeeCard

    coffeeCardRectangle.states: State {
        name: "small"
        when: ((Screen.height * Screen.devicePixelRatio) + (Screen.width * Screen.devicePixelRatio)) < 2900
        PropertyChanges {
            target: coffeeCardRectangle
            implicitHeight: ((applicationFlow.stack.height - ((applicationFlow.width / 2.4) / 1.36)) / 2) + 40
        }
    }

    button.states: State {
        name: "pressed"
        when: coffeeCard.button.pressed
        PropertyChanges {
            target: coffeeCardRectangle
            scale: 0.9
            gradient: Colors.invertedGreenBorder
        }
        PropertyChanges {
            target: coffeeCardCircle
            scale: 0.9
            gradient: Colors.invertedGreenBorder
        }
    }

    innerCoffeeCardRectangle.states: State {
        name: "small"
        when: ((Screen.height * Screen.devicePixelRatio)
               + (Screen.width * Screen.devicePixelRatio)) < 3200
        PropertyChanges {
            target: coffeeText
            font.pixelSize: 14
        }
        PropertyChanges {
            target: ingredientText
            font.pixelSize: 10
        }
        PropertyChanges {
            target: timeText
            font.pointSize: 10
        }
        PropertyChanges {
            target: actualTimeText
            font.pixelSize: 10
        }
        PropertyChanges {
            target: box
            radius: 6
            Layout.preferredHeight: 18
            Layout.preferredWidth: 18
        }
        PropertyChanges {
            target: cupsLeftText
            font.pixelSize: 10
        }
    }

    coffeeCardCircle.states: [
        State {
            name: "small"
            when: ((Screen.height * Screen.devicePixelRatio) + (Screen.width * Screen.devicePixelRatio)) < 3200 && ((Screen.height * Screen.devicePixelRatio) + (Screen.width * Screen.devicePixelRatio)) > 1200
            PropertyChanges {
                target: coffeeCardCircle
                implicitWidth: coffeeCardRectangle.implicitWidth / 1.5
            }
        },
        State {
            name: "smaller"
            when: Config.mode == "portrait" && ((Screen.height * Screen.devicePixelRatio) + (Screen.width * Screen.devicePixelRatio)) < 1200
            PropertyChanges {
                target: coffeeCardCircle
                implicitWidth: coffeeCardRectangle.implicitWidth / 2
            }
        }
    ]
}
