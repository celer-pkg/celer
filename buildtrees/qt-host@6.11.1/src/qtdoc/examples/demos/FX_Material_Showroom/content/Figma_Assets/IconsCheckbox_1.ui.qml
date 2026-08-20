// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Studio.Components

Item {
    id: iconsCheckbox
    width: 28
    height: 28
    state: "state_name_CheckEmpty"

    ArcItem {
        id: checkEmptyVector
        x: 2
        y: 2
        width: 24
        height: 24
        visible: false
        begin: 90
        end: 450.00001
        fillColor: "transparent"
        antialiasing: true
        strokeColor: "#989899"
        arcWidth: 12
        outlineArc: true
        strokeWidth: 1
    }

    SvgPathItem {
        id: checkFilledVector
        x: 2
        y: 2
        width: 24
        height: 24
        visible: false
        path: "M 12 24 C 18.627416610717773 24 24 18.627416610717773 24 12 C 24 5.372582912445068 18.627416610717773 0 12 0 C 5.372582912445068 0 0 5.372582912445068 0 12 C 0 18.627416610717773 5.372582912445068 24 12 24 Z M 9.666585922241211 17.83333396911621 L 3.833251953125 12 L 5.4782514572143555 10.354999542236328 L 9.666585922241211 14.531667709350586 L 18.521587371826172 5.676666259765625 L 20.16658592224121 7.3333330154418945 L 9.666585922241211 17.83333396911621 Z"
        fillColor: "#ffffff"
        strokeWidth: 1
        strokeColor: "transparent"
    }
    states: [
        State {
            name: "state_name_CheckEmpty"

            PropertyChanges {
                target: checkEmptyVector
                visible: true
            }
        },
        State {
            name: "state_name_CheckFilled"

            PropertyChanges {
                target: checkFilledVector
                visible: true
            }
        }
    ]
}

/*##^##
Designer {
    D{i:0;height:28;width:28}D{i:1;uuid:"4c637224-c99e-5eea-b42c-930909e37dd7"}D{i:2;uuid:"1c35c035-31b4-5c3b-a256-d071255c8171"}
}
##^##*/

