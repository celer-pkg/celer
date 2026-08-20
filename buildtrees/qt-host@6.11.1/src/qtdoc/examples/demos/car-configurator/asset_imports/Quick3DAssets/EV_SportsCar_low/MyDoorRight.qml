// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick3D

Model {
    id: doorRight
    pickable: true
    objectName: "MyDoorRight"
    property bool rain
    z: 0.8587785363197327
    receivesReflections: true
    property bool isOpen: false
    Node {
        id: __materialLibrary__
    }
    states: [
        State {
            name: "closed"
            when: !doorRight.isOpen
        },
        State {
            name: "open"
            when: doorRight.isOpen

            PropertyChanges {
                target: doorRight
                eulerRotation.z: -49

                eulerRotation.y: 19
                eulerRotation.x: 39
                // 30 16 -35
            }
        }
    ]
    transitions: [
        Transition {
            id: transition
            ParallelAnimation {
                SequentialAnimation {
                    PauseAnimation {
                        duration: 50
                    }

                    PropertyAnimation {
                        target: doorRight
                        property: "eulerRotation.x"
                        duration: 1000
                        easing.type: Easing.InOutQuad
                    }
                }

                SequentialAnimation {
                    PauseAnimation {
                        duration: 50
                    }

                    PropertyAnimation {
                        target: doorRight
                        property: "eulerRotation.y"
                        duration: 1000
                        easing.type: Easing.InOutQuad
                    }
                }

                SequentialAnimation {
                    PauseAnimation {
                        duration: 50
                    }

                    PropertyAnimation {
                        target: doorRight
                        property: "eulerRotation.z"
                        duration: 1000
                        easing.type: Easing.InOutQuad
                    }
                }
            }
            to: "*"
            from: "*"
        }
    ]
}
