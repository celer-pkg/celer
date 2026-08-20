// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick3D

Model {
    id: trunkLid
    property bool rain
    objectName: "TrunkLid"
    z: -0.5674706697463989
    receivesReflections: true
    property bool isOpen: false
    pickable: true
    Node {
        id: __materialLibrary__
    }
    states: [
        State {
            name: "closed"
            when: !trunkLid.isOpen
        },
        State {
            name: "open"
            when: trunkLid.isOpen

            PropertyChanges {
                target: trunkLid
                eulerRotation.z: -0.00001
                eulerRotation.y: -0
                eulerRotation.x: 28.57009

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
                        target: trunkLid
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
                        target: trunkLid
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
                        target: trunkLid
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
