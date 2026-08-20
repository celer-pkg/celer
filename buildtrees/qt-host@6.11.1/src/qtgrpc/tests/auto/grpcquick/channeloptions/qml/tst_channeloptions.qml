// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtTest
import QtGrpc
import QmlTestUri
import QtNetwork

Item {
    id: root

    property bool sslSupport: options.sslConfiguration !== undefined

    property Item sslDynamicItem;

    function createSslConfigurationItem() {
        return Qt.createQmlObject("import QtQuick; import QtGrpc; import QtNetwork; Item { \
                                   property sslConfiguration defaultConfig; }", root)
    }

    Component.onCompleted: {
        if (sslSupport) {
            sslDynamicItem = root.createSslConfigurationItem()
            options.sslConfiguration = sslDynamicItem.defaultConfig
        }
        optionsWithChangedProperties.metadata = null
        optionsWithChangedProperties.deadlineTimeout = 3000
        options.serializationFormat = QtGrpc.SerializationFormat.Json

    }

    GrpcChannelOptions {
        id: options
        deadlineTimeout: { 1000 }
        metadata: GrpcMetadata {
            id: grpcData
            data: ({ "user-name": "localhost",
                     "user-password": "qwerty"})
        }
        serializationFormat: {QtGrpc.SerializationFormat.Protobuf}
    }

    GrpcChannelOptions {
        id: optionsWithChangedProperties
        metadata: GrpcMetadata {
            data: ({ "user-name": "localhost",
                       "user-password": "qwerty"})
        }
        deadlineTimeout: { 1000 }
        serializationFormat: {QtGrpc.SerializationFormat.Json}
    }

    TestCase {
        id: optionsCase
        name: "qtgrpcSslChannelOptionsTest"

        function initTestCase()
        {
            if (!sslSupport) {
                warn("No SSL support detected!")
                compare(options.sslConfiguration, undefined)
            }
        }

        function test_sslOptionType_data() {
            if (sslSupport) {
                return [
                            { tag: "peerVerifyMode == AutoVerifyPeer",
                                field: options.sslConfiguration.peerVerifyMode , answer: SslSocket.AutoVerifyPeer }
                        ]
            } else {
                return [
                            { tag: "SSL option is not creatable",
                                field: options.sslConfiguration, answer: undefined }
                        ]
            }
        }

        function test_sslOptionType(data) {
            compare(data.field, data.answer)
        }
    }

    TestCase {
        name: "qtgrpcChannelOptionsTest"
        function test_OptionTypes_data() {
            return [
                        { tag: "options is an object",
                            field: typeof options, answer: "object" },
                        { tag: "options.metadata is an object",
                            field: typeof options.metadata, answer: "object" },
                        { tag: "options.metadata.data is an object",
                            field: typeof options.metadata.data, answer: "object" },
                        { tag: "grpcData is an object",
                            field: typeof grpcData, answer: "object" },
                        { tag: "QtGrpc.SerializationFormat.Protobuf == 1",
                            field: QtGrpc.SerializationFormat.Protobuf, answer: 1 },
                        { tag: "QtGrpc.SerializationFormat.Json == 2",
                            field: QtGrpc.SerializationFormat.Json, answer: 2 }
                    ]
        }

        function test_OptionTypes(data) {
            compare(data.field, data.answer)
        }

        function test_ChannelOptions_data() {
            return [
                        { tag: "options.metadata == grpcData",
                            field: options.metadata, answer: grpcData },
                        { tag: "options.metadata.data == grpcData.data",
                            field: options.metadata.data, answer: grpcData.data },
                        { tag: "options.metadata.data[user-name] == localhost",
                            field: options.metadata.data["user-name"], answer: "localhost" },
                        { tag: "options.metadata.data[user-password] == qwerty",
                            field: options.metadata.data["user-password"], answer: "qwerty" },
                        { tag: "metadata deadlineTimeout = 1000",
                            field: options.deadlineTimeout, answer: 1000 },
                        { tag: "optionsWithChangedProperties.metadata == null",
                            field: optionsWithChangedProperties.metadata, answer: null },
                        { tag: "optionsWithChangedProperties.deadlineTimeout == 3000",
                            field: optionsWithChangedProperties.deadlineTimeout, answer: 3000 }
                    ]
        }

        function test_ChannelOptions(data) {
            compare(data.field, data.answer)
        }

        function test_serializationFormat_data() {
            return [
                        { tag: "options.serializationFormat == QtGrpc.SerializationFormat.Json",
                            field: options.serializationFormat,
                            answer: QtGrpc.SerializationFormat.Json },
                        { tag: "qtgrpcSslChannelOptionsTest.serializationFormat == QtGrpc.SerializationFormat.Json",
                            field: optionsWithChangedProperties.serializationFormat,
                            answer: QtGrpc.SerializationFormat.Json },
                        { tag: "optionsWithChangedProperties.metadata == null",
                            field: optionsWithChangedProperties.metadata, answer: null },
                        { tag: "optionsWithChangedProperties.deadlineTimeout == 3000",
                            field: optionsWithChangedProperties.deadlineTimeout, answer: 3000 }
                    ]
        }

        function test_serializationFormat(data) {
            compare(data.field, data.answer)
        }
    }
}
