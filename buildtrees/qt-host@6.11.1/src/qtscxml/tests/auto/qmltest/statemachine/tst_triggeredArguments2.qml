// Copyright (C) 2017 Ford Motor Company
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtTest
import QtQml.StateMachine

TestCase {
    id: testCase

    property string mystr
    property bool mybool
    property int myint

    StateMachine {
        id: machine
        initialState: startState
        running: true
        State {
            id: startState
            SignalTransition {
                id: signalTrans
                signal: testCase.mysignal
                onTriggered: function(strarg, boolarg, intarg) {
                    testCase.mystr = strarg
                    testCase.mybool = boolarg
                    testCase.myint = intarg
                }
                targetState: finalState
            }
        }
        FinalState {
            id: finalState
        }
    }

    signal mysignal(string mystr, bool mybool, int myint)

    name: "testTriggeredArguments2"
    function test_triggeredArguments()
    {
        tryCompare(startState, "active", true)

        // Emit the signalTrans.signal
        testCase.mysignal("test1", true, 2)
        compare(testCase.mystr, "test1")
        compare(testCase.mybool, true)
        compare(testCase.myint, 2)
    }
}
