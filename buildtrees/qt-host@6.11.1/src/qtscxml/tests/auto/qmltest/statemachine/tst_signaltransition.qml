// Copyright (C) 2017 Ford Motor Company
// Copyright (C) 2017 The Qt Company
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtTest
import QtQml.StateMachine

TestCase {
    id: testCase
    StateMachine {
        id: machine
        initialState: startState
        State {
            id: startState
            SignalTransition {
                id: signalTrans
                signal: testCase.onMysignal
                targetState: finalState
            }
        }
        FinalState {
            id: finalState
        }
    }

    SignalSpy {
        id: finalStateActive
        target: finalState
        signalName: "activeChanged"
    }

    signal mysignal()

    name: "testSignalTransition"
    function test_signalTransition()
    {
        // Start statemachine, should not have reached finalState yet.
        machine.start()
        tryCompare(finalStateActive, "count", 0)
        tryCompare(machine, "running", true)

        testCase.mysignal()
        tryCompare(finalStateActive, "count", 1)
        tryCompare(machine, "running", false)
    }
}
