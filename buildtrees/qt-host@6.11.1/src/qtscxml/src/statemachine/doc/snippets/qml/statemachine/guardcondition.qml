// Copyright (C) 2014 Ford Motor Company
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

//! [document]
import QtQuick
import QtQml.StateMachine as DSM

Rectangle {
    Button {
        anchors.fill: parent
        id: button
        DSM.StateMachine {
            DSM.State {
                DSM.SignalTransition {
                    targetState: finalState
                    signal: button.mysignal
                    // the guard condition uses the mystr string argument from mysignal
                    guard: mystr == "test"
                }
            }
            DSM.FinalState {
                id: finalState
            }
        }
        // define the signal the SignalTransition is connected with
        signal mysignal(mystr: string)
        // on clicking the button emit the signal with a single string argument
        onClicked: button.mysignal("test")
    }
}
//! [document]
