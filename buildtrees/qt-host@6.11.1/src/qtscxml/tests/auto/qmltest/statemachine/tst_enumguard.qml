// Copyright (C) 2016 Ford Motor Company
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
                signal: testCase.mysignal
                guard: alignment === QState.ParallelStates
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

    signal mysignal(int alignment)

    name: "testEnumGuard"
    function test_enumGuard()
    {
        // Start statemachine, should not have reached finalState yet.
        machine.start()
        tryCompare(finalStateActive, "count", 0)
        tryCompare(machine, "running", true)

        // Emit the signalTrans.signal which will evaluate the guard. The
        // guard should return true, finalState be reached and the
        // statemachine be stopped.
        testCase.mysignal(QState.ParallelStates)
        tryCompare(finalStateActive, "count", 1)
        tryCompare(machine, "running", false)

        // Restart machine.
        machine.start()
        tryCompare(machine, "running", true)
        tryCompare(finalStateActive, "count", 2)

        // Emit signal that makes the signalTrans.guard return false. The
        // finalState should not have been triggered.
        testCase.mysignal(QState.ExclusiveStates)
        tryCompare(finalStateActive, "count", 2)
        tryCompare(machine, "running", true)
    }
}
