// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

pragma ComponentBehavior: Bound

import QtQuick
import TrafficLightApplication

Window {
    width: lights.width
    height: lights.height
    visible: true

    Lights {
        id: lights
        stateMachine: TrafficLightStateMachine {
            running: true
        }
        button.source: stateMachine.working ? "pause.png" : "play.png"
        button.onClicked: stateMachine.submitEvent(stateMachine.working ? "smash" : "repair");
    }
}
