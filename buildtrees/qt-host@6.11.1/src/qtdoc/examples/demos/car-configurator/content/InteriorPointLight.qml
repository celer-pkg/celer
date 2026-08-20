// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick3D

PointLight {
    id: pointLight
    color: "#fdffe3"
    shadowFilter: 93
    shadowBias: 1
    shadowMapFar: 100
    linearFade: 0.2
    constantFade: 0.2
    shadowFactor: 83
    shadowMapQuality: Light.ShadowMapQualityMedium
    castsShadow: false

    brightness: 7
    z: -0
    property bool isOn: false

    Node {
        id: __materialLibrary__
    }

    states: [
        State {
            name: "On"
            when: pointLight.isOn

            PropertyChanges {
                target: pointLight
                brightness: 10
            }
        },
        State {
            name: "Off"
            when: !pointLight.isOn

            PropertyChanges {
                target: pointLight
                brightness: 0
            }
        }
    ]

    transitions: [
        Transition {
            id: transition
            ParallelAnimation {
                SequentialAnimation {
                    PauseAnimation {
                        duration: 0
                    }

                    PropertyAnimation {
                        target: pointLight
                        property: "brightness"
                        duration: 2000
                    }
                }
            }
            to: "*"
            from: "*"
        }
    ]
}
