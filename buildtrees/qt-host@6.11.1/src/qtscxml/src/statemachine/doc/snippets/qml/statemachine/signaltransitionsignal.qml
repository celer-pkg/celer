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
                    signal: button.clicked
                }
            }
            DSM.FinalState {
                id: finalState
            }
        }
    }
}
//! [document]
