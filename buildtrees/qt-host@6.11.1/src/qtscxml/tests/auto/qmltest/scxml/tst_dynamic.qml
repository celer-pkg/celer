// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtTest 1.15
import QtScxml 5.15

TestCase {
    id: testCase

    StateMachineLoader {
        id: loader
        source: "qrc:///statemachine.scxml"
    }

    function test_overloaded_calls_with_dynamic_statemachine()
    {
        // This test calls "submitEvent" invokable function which has 3
        // overloads, differentiated both by parameter types and amounts.
        // Test verifies that the overloads are callable while using
        // a dynamic statemachine which has a dynamic metaobject under the hood
        tryVerify(() => loader.stateMachine.activeStateNames()[0] === "red", 200)
        loader.stateMachine.submitEvent("step")
        tryVerify(() => loader.stateMachine.activeStateNames()[0] === "yellow", 200)
        loader.stateMachine.submitEvent("step", "somedata")
        tryVerify(() => loader.stateMachine.activeStateNames()[0] === "green", 200)
    }
}
