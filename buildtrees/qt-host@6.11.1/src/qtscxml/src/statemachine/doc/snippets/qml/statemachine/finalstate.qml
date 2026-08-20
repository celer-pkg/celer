// Copyright (C) 2014 Ford Motor Company
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

//! [document]
import QtQuick
import QtQml.StateMachine as DSM

Rectangle {
    DSM.StateMachine {
        id: stateMachine
        initialState: state
        running: true
        DSM.State {
            id: state
            DSM.TimeoutTransition {
                targetState: finalState
                timeout: 200
            }
        }
        DSM.FinalState {
            id: finalState
        }
        onFinished: console.log("state finished")
    }
}
//! [document]
