// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtTest
import QtGrpc
import QmlTestUri

TestCase {
    id: root
    name: "qtgrpcMetadataQmlTest"

    GrpcMetadata {
        id: grpcData
        data: ({ "user-name": "localhost",
                   "user-password": "qwerty"})
    }
    GrpcMetadata {
        id: grpcMultiData;
        data: ({ "1-*756g": "localhost, remotehost",
                   "<---->": "qwerty, asdfgh7**"})
    }
    GrpcMetadata {
        id: grpcData_dup

        data: grpcData.data
    }

    function test_2GrpcTypes_data() {
        return [
                    { tag: "grpcData is an object",
                        field: typeof grpcData, answer: "object" },
                    { tag: "grpcData.data is an object",
                        field: typeof grpcData.data, answer: "object" },
                    { tag: "grpcMultiData is an object",
                        field: typeof grpcMultiData, answer: "object" },
                    { tag: "grpcMultiData.data is an object",
                        field: typeof grpcMultiData.data, answer: "object" },
                    { tag: "grpcData_dup is an object",
                        field: typeof grpcData_dup, answer: "object" },
                    { tag: "grpcData_dup.data is an object",
                        field: typeof grpcData_dup.data, answer: "object" }
                ]
    }

    function test_2GrpcTypes(data) {
        compare(data.field, data.answer)
    }

    function test_3GrpcMetadata_data() {
        return [
                    { tag: "grpcData[user-name] key exists",
                        field: "user-name" in grpcData.data, answer: true },
                    { tag: "grpcData[user-password] key exists",
                        field: "user-password" in grpcData.data, answer: true },
                    { tag: "grpcMultiData[1-*756g] key exists",
                        field: "1-*756g" in grpcMultiData.data, answer: true },
                    { tag: "grpcMultiData[<---->] key exists",
                        field: "<---->" in grpcMultiData.data, answer: true },
                    { tag: "grpcData[user-name] == localhost",
                        field: grpcData.data["user-name"], answer: "localhost" },
                    { tag: "grpcData[password] == qwerty",
                        field: grpcData.data["user-password"], answer: "qwerty" },
                    { tag: "grpcMultiData[user-name] == localhost, remotehost",
                        field: grpcMultiData.data["1-*756g"], answer: "localhost, remotehost" },
                    { tag: "grpcMultiData[password] == qwerty, asdfgh7**",
                        field: grpcMultiData.data["<---->"], answer: "qwerty, asdfgh7**" },
                    { tag: "grpcData_dup == grpcData",
                        field: grpcData_dup.data, answer: grpcData.data },
                ]
    }

    function test_3GrpcMetadata(data) {
        compare(data.field, data.answer)
    }
}
