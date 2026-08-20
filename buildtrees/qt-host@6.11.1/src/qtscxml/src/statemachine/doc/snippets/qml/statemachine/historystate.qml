// Copyright (C) 2014 Ford Motor Company
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

//! [document]
import QtQuick
import QtQml.StateMachine as DSM

Rectangle {
    Button {
        anchors.fill: parent
        id: button
        text: "Press me"
        DSM.StateMachine {
            id: stateMachine
            initialState: parentState
            running: true
            DSM.State {
                id: parentState
                initialState: child2
                onEntered: console.log("parentState entered")
                onExited: console.log("parentState exited")
                DSM.State {
                    id: child1
                    onEntered: console.log("child1 entered")
                    onExited: console.log("child1 exited")
                }
                DSM.State {
                    id: child2
                    onEntered: console.log("child2 entered")
                    onExited: console.log("child2 exited")
                }
                DSM.HistoryState {
                    id: historyState
                    defaultState: child1
                }
                DSM.SignalTransition {
                    targetState: historyState

                    // Clicking the button will cause the state machine to enter the child state
                    // that parentState was in the last time parentState was exited, or the history state's default
                    // state if parentState has never been entered.
                    signal: button.clicked
                }
            }
        }
    }
}
//! [document]
