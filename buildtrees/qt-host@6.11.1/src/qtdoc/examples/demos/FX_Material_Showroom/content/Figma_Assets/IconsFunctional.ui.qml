// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Studio.Components

Item {
    id: iconsFunctional
    width: 48
    height: 48
    state: "state_icon_Close"

    SvgPathItem {
        id: close_Vector
        x: 10
        y: 10
        width: 28
        height: 28
        path: "M 25.180004119873047 0 L 28.000003814697266 2.8199996948242188 L 16.82000160217285 14.000001907348633 L 28.000003814697266 25.180004119873047 L 25.180004119873047 28.000003814697266 L 14.000001907348633 16.82000160217285 L 2.8199996948242188 28.000003814697266 L 0 25.180004119873047 L 11.180000305175781 14.000001907348633 L 0 2.8199996948242188 L 2.8199996948242188 0 L 14.000001907348633 11.180000305175781 L 25.180004119873047 0 Z"
        fillColor: "#ffffff"
        strokeWidth: 1
        strokeColor: "transparent"
    }

    SvgPathItem {
        id: arrow_Vector
        x: 8
        y: 8
        width: 32
        height: 32
        path: "M 13.180000305175781 2.8199996948242188 L 16 0 L 32 16 L 16 32 L 13.180000305175781 29.18000030517578 L 24.34000015258789 17.999998092651367 L 0 17.999998092651367 L 0 14 L 24.34000015258789 14 L 13.180000305175781 2.8199996948242188 Z"
        fillColor: "#ffffff"
        strokeWidth: 1
        strokeColor: "transparent"
    }
    states: [
        State {
            name: "state_icon_Close"

            PropertyChanges {
                target: arrow_Vector
                visible: false
            }
        },
        State {
            name: "state_icon_ArrowRight"
            extend: "state_icon_Close"

            PropertyChanges {
                target: close_Vector
                visible: false
            }

            PropertyChanges {
                target: arrow_Vector
                visible: true
            }
        }
    ]
}

/*##^##
Designer {
    D{i:0;uuid:"cc6a230c-8aa4-5be8-9645-07e3db2ac100"}D{i:1;uuid:"d8c7d35b-ba9a-58be-a046-a12fc4a6761b"}
D{i:2;uuid:"3076d5e4-0f44-5782-8399-5bc34efa43a0"}
}
##^##*/

