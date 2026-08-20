// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

pragma ComponentBehavior: Bound

import QtQuick
import QtScxml
import TrafficLightApplication

Window {
    width: lights.width
    height: lights.height
    visible: true

    Lights {
        id: lights
        stateMachine: loader.stateMachine
        // Suppress qmllint warning, dynamic statemachine properties not known at compile-time
        // qmllint disable missing-property
        button.source: stateMachine.working ? "pause.png" : "play.png"
        button.onClicked: stateMachine.submitEvent(stateMachine.working ? "smash" : "repair");
        // qmllint enable missing-property
    }

    StateMachineLoader {
        id: loader
        source: Qt.resolvedUrl("statemachine.scxml")
    }
}
