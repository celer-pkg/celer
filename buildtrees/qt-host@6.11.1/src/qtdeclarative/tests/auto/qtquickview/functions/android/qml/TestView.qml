// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtTest

Item {
    id: root

    TestActivityCommunicator {
        id: communicator
    }

    signal onEmptyFunction();
    signal onParameterisedFunction(i: int, d: double, r: real, s: string, b: bool);

    function emptyFunction() {
        root.onEmptyFunction()
    }

    function parameterisedFunction(i: int, d: double, r: real, s: string, b: bool) {
        root.onParameterisedFunction(i, d, r, s, b)
    }

    function shouldNotBeGenerated(uh: Date, oh: int) {}

    function shouldNotBeGeneratedEither(params, should, be, typed) {}

    SignalSpy {
        id: onEmptyFunctionSpy
        target: root
        signalName: "onEmptyFunction"
    }

    SignalSpy {
        id: onParameterisedFunctionSpy
        target: root
        signalName: "onParameterisedFunction"
    }

    SignalSpy {
        id: invalidFunctionsTestSpy
        target: communicator
        signalName: "onInvalidFunctionsTestFinished"
    }

    TestCase {
        when: windowShown
        name: "emptyFunction"

        function test_empty() {
            onEmptyFunctionSpy.clear()
            communicator.callFunction("emptyFunction")
            onEmptyFunctionSpy.wait()
            compare(onEmptyFunctionSpy.count, 1)
        }

        function test_empty_qtquickviewcontent() {
            onEmptyFunctionSpy.clear()
            communicator.callFunctionQuickViewContent()
            onEmptyFunctionSpy.wait()
            compare(onEmptyFunctionSpy.count, 1)
        }
    }

    TestCase {
        when: windowShown
        name: "parameterisedFunction"

        function test_parameters() {
            onParameterisedFunctionSpy.clear()
            communicator.callFunctionWithParams("parameterisedFunction", 1, 2.2, 3.3, "string", true)
            onParameterisedFunctionSpy.wait()
            compare(onParameterisedFunctionSpy.count, 1)
            compare(onParameterisedFunctionSpy.signalArguments[0].length, 5)
            compare(onParameterisedFunctionSpy.signalArguments[0][0], 1)
            compare(onParameterisedFunctionSpy.signalArguments[0][1], 2.2)
            compare(onParameterisedFunctionSpy.signalArguments[0][2], 3.3)
            compare(onParameterisedFunctionSpy.signalArguments[0][3], "string")
            compare(onParameterisedFunctionSpy.signalArguments[0][4], true)
        }

        function test_paramaterised_qtquickviewcontent() {
            onParameterisedFunctionSpy.clear()
            communicator.callFunctionWithParamsQuickViewContent(1, 2.2, 3.3, "string", true)
            onParameterisedFunctionSpy.wait()
            compare(onParameterisedFunctionSpy.count, 1)
            compare(onParameterisedFunctionSpy.signalArguments[0].length, 5)
            compare(onParameterisedFunctionSpy.signalArguments[0][0], 1)
            compare(onParameterisedFunctionSpy.signalArguments[0][1], 2.2)
            compare(onParameterisedFunctionSpy.signalArguments[0][2], 3.3)
            compare(onParameterisedFunctionSpy.signalArguments[0][3], "string")
            compare(onParameterisedFunctionSpy.signalArguments[0][4], true)
        }
    }

    TestCase {
        when: windowShown
        name: "invalidFunctions"

        function test_nonGeneratedFunctions() {
            invalidFunctionsTestSpy.clear()
            communicator.testInvalidFunctions(["shouldNotBeGenerated", "shouldNotBeGeneratedEither"])
            invalidFunctionsTestSpy.wait()
            compare(invalidFunctionsTestSpy.count, 1)
            compare(invalidFunctionsTestSpy.signalArguments[0][0], true)
        }
    }
}
