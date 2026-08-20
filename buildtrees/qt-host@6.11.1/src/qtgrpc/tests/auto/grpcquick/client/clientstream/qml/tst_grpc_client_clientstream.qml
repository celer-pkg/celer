// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtTest
import QmlTestUri
import QtGrpc;
import qtgrpc.tests

Item {
    id: root

    readonly property int expectedNumberOfMessages: 4

    property simpleStringMessage messageArg;
    property simpleStringMessage messageResponse;
    property string result: ""
    property var streamSender: null
    property bool errorCallbackCalled: false
    property int times: 1

    Timer {
        id: timer
        running: false
        repeat: false
        interval: testMessageLatencyWithThreshold * root.expectedNumberOfMessages
        onTriggered:  testCase.when = true;
    }


    function endTest(msg) {
        result = msg.testFieldString
        testCase.when = true
    }

    function errorCallback() {
        root.errorCallbackCalled = true
        endTest()
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
        name: "startClientStream"

        function test_testClientStream() {
            root.messageArg.testFieldString = "streamQml"
            root.streamSender = clientQml.testMethodClientStream(root.messageArg,
                                                                 root.endTest,
                                                                 root.errorCallback)
            timer.start()
        }
    }

    Timer {
        running: root.streamSender != null && root.times < root.expectedNumberOfMessages
        interval: testMessageLatency
        onTriggered: {
            root.streamSender.writeMessage(root.messageArg)
            ++times
        }
    }

    TestCase {
        id: testCase
        name: "checkClientStreamResult"
        when: false

        function test_testClientStreamCheck() {
            compare(root.result, "streamQml1streamQml2streamQml3streamQml4")
            compare(root.times, root.expectedNumberOfMessages)
            verify(!root.errorCallbackCalled)
        }
    }

    Component.onCompleted: {
        root.messageArg.testFieldString = "streamQml"
    }
}
