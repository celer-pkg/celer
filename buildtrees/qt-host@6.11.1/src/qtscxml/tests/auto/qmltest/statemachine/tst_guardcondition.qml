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
                guard: mystr == "test1"
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

    signal mysignal(string mystr, bool mybool, int myint)

    name: "testGuardCondition"
    function test_guardCondition()
    {
        // Start statemachine, should not have reached finalState yet.
        machine.start()
        tryCompare(finalStateActive, "count", 0)
        tryCompare(machine, "running", true)

        // Emit the signalTrans.signal which will evaluate the guard. The
        // guard should return true, finalState be reached and the
        // statemachine be stopped.
        testCase.mysignal("test1", true, 2)
        tryCompare(finalStateActive, "count", 1)
        tryCompare(machine, "running", false)

        // Restart machine.
        machine.start()
        tryCompare(machine, "running", true)
        tryCompare(finalStateActive, "count", 2)

        // Emit signal that makes the signalTrans.guard return false. The
        // finalState should not have been triggered.
        testCase.mysignal("test2", true, 2)
        tryCompare(finalStateActive, "count", 2)
        tryCompare(machine, "running", true)

        // Change the guard in javascript to test that boolean true/false
        // works as expected.
        signalTrans.guard = false;
        testCase.mysignal("test1", true, 2)
        tryCompare(finalStateActive, "count", 2)
        tryCompare(machine, "running", true)
        signalTrans.guard = true;
        testCase.mysignal("test1", true, 2)
        tryCompare(finalStateActive, "count", 3)
        tryCompare(machine, "running", false)
    }
}
