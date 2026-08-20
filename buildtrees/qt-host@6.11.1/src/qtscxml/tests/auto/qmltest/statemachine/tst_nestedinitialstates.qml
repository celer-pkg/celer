// Copyright (C) 2016 Ford Motor Company
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtTest
import QtQml.StateMachine

TestCase {
    StateMachine {
        id: myStateMachine
        initialState: parentState
        State {
            id: parentState
            initialState: childState1
            State {
                id: childState1
            }
            State {
                id: childState2
            }
        }
    }
    name: "nestedInitalStates"

    function test_nestedInitalStates() {
        compare(myStateMachine.running, false);
        compare(parentState.active, false);
        compare(childState1.active, false);
        compare(childState2.active, false);
        myStateMachine.start();
        tryCompare(myStateMachine, "running", true);
        tryCompare(parentState, "active", true);
        tryCompare(childState1, "active", true);
        compare(childState2.active, false);
    }
}
