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
            initialState: childStateMachine
            StateMachine {
                id: childStateMachine
                initialState: childState2
                State {
                    id: childState1
                }
                State {
                    id: childState2
                }
            }
        }
    }
    name: "nestedStateMachine"

    function test_nestedStateMachine() {
        compare(myStateMachine.running, false);
        compare(parentState.active, false);
        compare(childStateMachine.running, false);
        compare(childState1.active, false);
        compare(childState2.active, false);
        myStateMachine.start();
        tryCompare(myStateMachine, "running", true);
        tryCompare(parentState, "active", true);
        tryCompare(childStateMachine, "running", true);
        tryCompare(childState1, "active", false);
        tryCompare(childState2, "active", true);
    }
}
