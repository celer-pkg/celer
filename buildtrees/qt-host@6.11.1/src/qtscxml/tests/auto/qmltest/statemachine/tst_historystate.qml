// Copyright (C) 2016 Ford Motor Company
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtTest
import QtQml.StateMachine

TestCase {

    StateMachine {
        id: stateMachine
        initialState: historyState1

        State {
            id: state1
            SignalTransition {
                id: st1
                targetState: state2
            }
        }

        State {
            id: state2
            initialState: historyState2
            HistoryState {
                id: historyState2
                defaultState: state21
            }
            State {
                id: state21
            }
        }

        HistoryState {
            id: historyState1
            defaultState: state1
        }
    }

    SignalSpy {
        id: state1SpyActive
        target: state1
        signalName: "activeChanged"
    }

    SignalSpy {
        id: state2SpyActive
        target: state2
        signalName: "activeChanged"
    }


    function test_historyStateAsInitialState()
    {
        stateMachine.start();
        tryCompare(stateMachine, "running", true);
        tryCompare(state1SpyActive, "count" , 1);
        tryCompare(state2SpyActive, "count" , 0);
        st1.invoke();
        tryCompare(state1SpyActive, "count" , 2);
        tryCompare(state2SpyActive, "count" , 1);
        tryCompare(state21, "active", true);
        tryCompare(state1, "active", false);
    }
}
