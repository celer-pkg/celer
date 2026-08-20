// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQml.Models

ListView {
    width: 400
    height: 400
    model: 8

    delegate: DelegateChooser {
        DelegateChoice {
            index: 0
            delegate: Item {
                property string choiceType: "1"
                width: parent.width
                height: 50
            }
        }
        DelegateChoice {
            index: 1
            delegate: Item {
                property string choiceType: "2"
                width: parent.width
                height: 50
            }
        }
        DelegateChoice {
            index: 2
            delegate: Item {
                property string choiceType: "3"
                width: parent.width
                height: 50
            }
        }
        DelegateChoice {
            index: 5
            delegate: Item {
                property string choiceType: "3"
                width: parent.width
                height: 50
            }
        }
        DelegateChoice {
            delegate: Item {
                property string choiceType: "4"
                width: parent.width
                height: 50
            }
        }
    }
}
