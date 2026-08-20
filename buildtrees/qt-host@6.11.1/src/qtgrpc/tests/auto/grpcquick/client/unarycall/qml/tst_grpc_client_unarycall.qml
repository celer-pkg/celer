// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtGrpc;
import QtProtobuf;
import QtTest
import QmlTestUri
import qtgrpc.tests

Item {
    id: root

    Timer {
        id: timer
        running: false
        repeat: false
        interval: 10000
        onTriggered:  testCase.when = true;
    }

    property simpleStringMessage messageArg;
    property simpleStringMessage messageResponse;

    property bool calbackCalled: false
    property var clientQml
    property var grpcChannel
    property var grpcChannelDeadline
    property var setResponse: function(value) { root.messageResponse = value; testCase.when = true; }
    property var errorCallback: function() { root.calbackCalled = true; testCase.when = true; }

    function createClientItem() {
        return Qt.createQmlObject("import QtQuick; import QtGrpc; \
                                   import qtgrpc.tests; TestServiceClient {}", root)
    }

    function createGrpcChannelItem() {
        return Qt.createQmlObject("import QtQuick; import QtGrpc; GrpcHttp2Channel { \
                                   hostUri: \"http://localhost:50051\"; \
                                   options: GrpcChannelOptions { \
                                   deadlineTimeout: { 2000 } \
                                   metadata: GrpcMetadata {
                                        data: ({ \"common-meta-data\": \"test-channel-metadata\" }) \
                                   }}}", root)
    }

    function createGrpcChannelWithDeadlineItem() {
        return Qt.createQmlObject("import QtQuick; import QtGrpc; GrpcHttp2Channel { \
                                   hostUri: \"http://localhost:50051\"; \
                                   options: GrpcChannelOptions { \
                                   deadlineTimeout: { 1000 } }  }", root)
    }

    TestCase {
        name: "qtgrpcClientRegistration"
        function test_1clientTypes_data() {
            return [
                        { tag: "Grpc Client created",
                            field: typeof clientQml, answer: "object" },
                        { tag: "Grpc Http2 Channel created",
                            field: typeof grpcChannel, answer: "object" },
                        { tag: "Grpc Http2 Deadline Channel created",
                            field: typeof grpcChannelDeadline, answer: "object" }
                    ]
        }

        function test_1clientTypes(data) {
            compare(data.field, data.answer)
        }

        function test_ChannelOptions_data() {
            return [
                        { tag: "grpcChannelOptions URL is set",
                            field: grpcChannel.hostUri, answer: "http://localhost:50051" },
                        { tag: "grpcChannelOptions deadline is set",
                            field: grpcChannelDeadline.options.deadlineTimeout, answer: 1000 }
                    ]
        }

        function test_ChannelOptions(data) {
            compare(data.field, data.answer)
        }

        function test_testMethodCall() {
            clientQml.testMethod(root.messageArg, root.setResponse, root.errorCallback);
            timer.start()
        }
    }

    TestCase {
        id: testCase
        name: "qtgrpcClientTestCall"
        when: false

        function test_testMethodCallCheck() {
            verify(root.messageResponse == root.messageArg)
            verify(!root.calbackCalled)
        }
    }

    TestCase {
        id: unaryCallWithOptions
        name: "unaryCallWithOptions"
        property empty arg;
        property metadataMessage result;
        property bool errorOccurred: false;
        GrpcCallOptions {
            id: options
            metadata: GrpcMetadata {
                data: ({ "user-name": "localhost",
                         "user-password": "qwerty"})
            }
        }

        Timer {
            id: unaryCallWithOptionsTimeout
            running: false
            repeat: false
            interval: 10000
            onTriggered: unaryCallWithOptionsCheck.when = true;
        }
        function test_unaryCallWithOptions() {
            clientQml.replyWithMetadata(unaryCallWithOptions.arg,
                                        function(value) {
                                            unaryCallWithOptions.result = value;
                                            unaryCallWithOptionsCheck.when = true;
                                        },
                                        function() {
                                            unaryCallWithOptions.errorOccurred = true;
                                            unaryCallWithOptionsCheck.when = true;
                                        },
                                        options)
            unaryCallWithOptionsTimeout.start()
        }
    }

    TestCase {
        id: unaryCallWithOptionsCheck
        name: "unaryCallWithOptionsCheck"
        when: false

        function removeElementFromArray(array, element) {
            var index = array.indexOf(element)
            verify(index !== -1)

            array.splice(index, 1)
        }

        function test_unaryCallWithOptionsCheck() {
            verify(!unaryCallWithOptions.errorOccurred, "unaryCallWithOptions ended with error")

            var missingHeaders = Array()
            missingHeaders.push("user-name")
            missingHeaders.push("user-password")
            missingHeaders.push("common-meta-data")

            for (var i = 0; i < unaryCallWithOptions.result.valuesData.length; i++) {
                var md = unaryCallWithOptions.result.valuesData[i]
                if (md.key === "user-name" && md.value === "localhost")
                    removeElementFromArray(missingHeaders, "user-name")
                if (md.key === "user-password" && md.value === "qwerty")
                    removeElementFromArray(missingHeaders, "user-password")
                if (md.key === "common-meta-data" && md.value === "test-channel-metadata")
                    removeElementFromArray(missingHeaders, "common-meta-data")
            }

            verify(missingHeaders.length === 0,
                   "Missing headers from server: " + missingHeaders)
        }
    }


    Component.onCompleted: {
        clientQml = root.createClientItem()
        grpcChannel = root.createGrpcChannelItem()
        grpcChannelDeadline = root.createGrpcChannelWithDeadlineItem()
        clientQml.channel = grpcChannel.channel
    }
}
