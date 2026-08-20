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

    function readMessage(msg) {
        root.result += msg.testFieldString

        if (root.times < expectedNumberOfMessages) {
            ++root.times
            root.messageArg.testFieldString = "streamQml" + root.times
            testCase.verify(root.streamSender,
                            "readMessage callback is called without active stream sender")
            root.streamSender.writeMessage(root.messageArg)

        }
    }

    function endTest() {
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
        name: "startBidiStream"

        function test_testBidiStream() {
            root.messageArg.testFieldString = "streamQml" + root.times
            root.streamSender = clientQml.testMethodBiStream(root.messageArg,
                                                             root.readMessage,
                                                             root.endTest,
                                                             root.errorCallback)
            timer.start()
        }
    }

    TestCase {
        id: testCase
        name: "checkBidiStreamResult"
        when: false

        function test_testBidiStreamCheck() {
            compare(root.result, "streamQml11streamQml22streamQml33streamQml44")
            compare(root.times, root.expectedNumberOfMessages)
            verify(!root.errorCallbackCalled)
        }
    }
}
