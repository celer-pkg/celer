// Copyright (C) 2014 Ford Motor Company
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

//! [document]
import QtQuick
import QtQml.StateMachine as DSM

Rectangle {
    Button {
        anchors.fill: parent
        id: button
        text: "Finish state"
        enabled: !stateMachine.running
        onClicked: stateMachine.running = true
        DSM.StateMachine {
            id: stateMachine
            initialState: state
            running: true
            DSM.State {
                id: state
                DSM.TimeoutTransition {
                    targetState: finalState
                    timeout: 1000
                }
            }
            DSM.FinalState {
                id: finalState
            }
        }
    }
}
//! [document]
