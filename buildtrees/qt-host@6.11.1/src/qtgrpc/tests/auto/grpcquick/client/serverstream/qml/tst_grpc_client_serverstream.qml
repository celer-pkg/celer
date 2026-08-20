// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtTest
import QmlTestUri
import QtGrpc;
import qtgrpc.tests

Item {
    id: root

    property simpleStringMessage messageArg;
    property simpleStringMessage messageResponse;
    property string result: ""

    property bool errorCallbackCalled: false
    property int times: 0

    readonly property int expectedNumberOfMessages: 4

    Timer {
        id: timer
        running: false
        repeat: false
        interval: testMessageLatencyWithThreshold * (root.expectedNumberOfMessages + 1)
        onTriggered: testCase.when = true;
    }

    function readMessage(msg) {
        root.result += msg.testFieldString
        ++root.times
    }

    function endTest() {
        testCase.when = true
    }

    function errorCallback() {
        root.errorCallbackCalled = true
        root.endTest()
    }

    GrpcHttp2Channel {
        id: httpChannel
        hostUri: "http://localhost:50051"
        options: GrpcChannelOptions {
        }
    }

    TestServiceClient {
        id: clientQml
        channel: httpChannel.channel
    }

    TestCase {
        name: "startServerStream"

        function test_testServerStream() {
            root.messageArg.testFieldString = "streamQml"
            clientQml.testMethodServerStream(root.messageArg, root.readMessage, root.endTest,
                                             root.errorCallback)
            timer.start()
        }
    }

    TestCase {
        id: testCase
        name: "checkServerStreamResult"
        when: false

        function test_testServerStreamCheck() {
            compare(root.result, "streamQml1streamQml2streamQml3streamQml4")
            compare(root.times, expectedNumberOfMessages)
            verify(!root.errorCallbackCalled)
        }
    }
}
