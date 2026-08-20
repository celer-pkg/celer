// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtTest
import QtGrpc
import QmlTestUri

TestCase {
    id: root
    name: "qtgrpcCallOptionsTest"

    GrpcCallOptions {
        id: options
        metadata: grpcData
        deadlineTimeout: { 1000 }
    }

    GrpcCallOptions {
        id: options_dup
        metadata: options.metadata
        deadlineTimeout: options.deadlineTimeout
    }

    GrpcMetadata {
        id: grpcData
        data: ({ "user-name": "localhost",
                 "user-password": "qwerty"})
    }

    GrpcCallOptions {
        id: optionsWithChangedProperties
        metadata: grpcData
        deadlineTimeout: { 2000 }
    }

    GrpcCallOptions {
        id: optionsWithDefaultProperty
    }

    Component.onCompleted: {
        optionsWithChangedProperties.metadata = null
        optionsWithChangedProperties.deadlineTimeout = 3000
    }

    function test_OptionTypes_data() {
        return [
                    { tag: "options is an object",
                        field: typeof options, answer: "object" },
                    { tag: "options.metadata is an object",
                        field: typeof options.metadata, answer: "object" },
                    { tag: "options.metadata.data is an object",
                        field: typeof options.metadata.data, answer: "object" },
                    { tag: "grpcData is an object",
                        field: typeof grpcData, answer: "object" }
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
                    { tag: "metadata deadlineTimeout == 1000",
                        field: options.deadlineTimeout, answer: 1000 },
                    { tag: "options == options_dup",
                        field: options.metadata, answer: options_dup.metadata },
                    { tag: "optionsWithChangedProperties.metadata == null",
                        field: optionsWithChangedProperties.metadata, answer: null },
                    { tag: "optionsWithChangedProperties.deadlineTimeout == 3000",
                        field: optionsWithChangedProperties.deadlineTimeout, answer: 3000 },
                    { tag: "optionsWithDefaultProperty.metadata == null",
                        field: optionsWithDefaultProperty.metadata, answer: null }
                ]
    }

    function test_ChannelOptions(data) {
        compare(data.field, data.answer)
    }
}
