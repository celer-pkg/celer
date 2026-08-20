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
            DSM.SignalTransition {
                targetState: finalState
                signal: button.clicked
                guard: guardButton.checked
            }
        }
        DSM.FinalState {
            id: finalState
        }
        onFinished: Qt.quit()
    }
    Row {
        spacing: 2
        Button {
            id: button
            text: "Finish state"
        }

        Button {
            id: guardButton
            checkable: true
            text: checked ? "Press to block the SignalTransition" : "Press to unblock the SignalTransition"
        }
    }
}
//! [document]
