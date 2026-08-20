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
        path: "M 12 24 C 18.627416610717773 24 24 18.627416610717773 24 12 C 24 5.372582912445068 18.627416610717773 0 12 0 C 5.372582912445068 0 0 5.372582912445068 0 12 C 0 18.627416610717773 5.372582912445068 24 12 24 Z M 9.666585922241211 17.83333396911621 L 3.833251953125 12 L 5.4782514572143555 10.354999542236328 L 9.666585922241211 14.531667709350586 L 18.521587371826172 5.676666259765625 L 20.16658592224121 7.3333330154418945 L 9.666585922241211 17.83333396911621 Z"
        fillColor: "#ffffff"
        strokeWidth: 1
        strokeColor: "transparent"
    }
    states: [
        State {
            name: "state_name_CheckEmpty"

            PropertyChanges {
                target: checkFilledVector
                visible: false
            }
        },
        State {
            name: "state_name_CheckFilled"
            extend: "state_name_CheckEmpty"

            PropertyChanges {
                target: checkFilledVector
                visible: true
            }

            PropertyChanges {
                target: checkEmptyVector
                visible: false
            }
        }
    ]
}

/*##^##
Designer {
    D{i:0;uuid:"1d835453-f12d-55cc-93b6-64b6520c15c5"}D{i:1;uuid:"5cff71a4-0143-5689-a7e2-e2566dc04940"}
D{i:2;uuid:"233f1ecd-a0e7-5ffc-8ae4-7e08d2732e7b"}
}
##^##*/

