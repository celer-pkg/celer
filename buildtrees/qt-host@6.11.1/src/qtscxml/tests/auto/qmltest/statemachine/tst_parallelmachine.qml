// Copyright (C) 2016 Ford Motor Company
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtTest
import QtQml.StateMachine

TestCase {
    StateMachine {
        id: myStateMachine
        initialState: rootState
        State {
            id: rootState
            childMode: State.ParallelStates
            State {
                id: childState1
                childMode: State.ParallelStates
                State {
                    id: childState11
                }
                State {
                    id: childState12
                }
            }
            State {
                id: childState2
                initialState: childState21
                State {
                    id: childState21
                }
                State {
                    id: childState22
                }
            }
        }
    }
    name: "nestedParallelMachineStates"

    function test_nestedInitalStates() {
        // uncomment me after vm problems are fixed.
        //            compare(myStateMachine.running, false);
        compare(childState1.active, false);
        compare(childState11.active, false);
        compare(childState12.active, false);
        compare(childState2.active, false);
        compare(childState21.active, false);
        compare(childState22.active, false);
        myStateMachine.start();
        tryCompare(myStateMachine, "running", true);
        tryCompare(childState1, "active", true);
        tryCompare(childState11, "active", true);
        tryCompare(childState12, "active", true);
        tryCompare(childState2, "active", true);
        tryCompare(childState21, "active", true);
        tryCompare(childState22, "active", false);
    }
}
