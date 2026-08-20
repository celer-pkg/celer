// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtTest

import qtprotobufnamespace.tests
import qtprotobufnamespace.tests.enums as EnumPack_

TestCase {
    name: "qtprotobufEnumTest"
    property simpleEnumMessage localEnumMessage
    property repeatedEnumMessage localRepeatedEnumMessage
    property mixedEnumUsageMessage localMixedEnumMessage
    property simpleFileEnumMessage globalEnumMessage
    property stepChildEnumMessage localStepChildEnumMessage

    property EnumPack_.simpleEnumMessage localEnumMessage2
    property EnumPack_.mixedEnumUsageMessage localMixedEnumMessage2

    property b shortBName;
    property a shortAName;

    function initTestCase() {
        localEnumMessage.localEnum = SimpleEnumMessage.LOCAL_ENUM_VALUE3
        localEnumMessage2.localEnum = EnumPack_.SimpleEnumMessage.LOCAL_ENUM_VALUE3

        localMixedEnumMessage.localEnum = MixedEnumUsageMessage.LOCAL_ENUM_VALUE3
        localMixedEnumMessage2.localEnum
            = EnumPack_.MixedEnumUsageMessage.LOCAL_ENUM_VALUE3

        localMixedEnumMessage.localEnumList = [
                    MixedEnumUsageMessage.LOCAL_ENUM_VALUE0,
                    MixedEnumUsageMessage.LOCAL_ENUM_VALUE1,
                ]
        localRepeatedEnumMessage.localEnumList = [
                    RepeatedEnumMessage.LOCAL_ENUM_VALUE0,
                    RepeatedEnumMessage.LOCAL_ENUM_VALUE1,
                    RepeatedEnumMessage.LOCAL_ENUM_VALUE3
                ]

        globalEnumMessage.globalEnum = TestEnum.TEST_ENUM_VALUE4
        globalEnumMessage.globalEnumList = [
                    TestEnum.TEST_ENUM_VALUE0,
                    TestEnum.TEST_ENUM_VALUE1
                ]

        localStepChildEnumMessage.localStepChildList =[
                    SimpleEnumMessage.LOCAL_ENUM_VALUE0,
                    SimpleEnumMessage.LOCAL_ENUM_VALUE1
                ]

        shortBName.val = A.AVal1
        shortAName.val = B.BVal0
    }

    function test_enumValues_data() {
        return [
                    // TestEnum
                    { tag: "TestEnum.TEST_ENUM_VALUE0 == 0",
                        field: TestEnum.TEST_ENUM_VALUE0, answer: 0 },
                    { tag: "TestEnum.TEST_ENUM_VALUE1 == 1",
                        field: TestEnum.TEST_ENUM_VALUE1, answer: 1 },
                    { tag: "TestEnum.TEST_ENUM_VALUE2 == 2",
                        field: TestEnum.TEST_ENUM_VALUE2, answer: 2 },
                    { tag: "TestEnum.TEST_ENUM_VALUE3 == 4",
                        field: TestEnum.TEST_ENUM_VALUE3, answer: 4 },
                    { tag: "TestEnum.TEST_ENUM_VALUE4 == 3",
                        field: TestEnum.TEST_ENUM_VALUE4, answer: 3 },
                    // EnumPack_.TestEnum
                    { tag: "EnumPack_.TestEnum.TEST_ENUM_VALUE0 == 0",
                        field: EnumPack_.TestEnum.TEST_ENUM_VALUE0, answer: 0 },
                    { tag: "EnumPack_.TestEnum.TEST_ENUM_VALUE1 == 1",
                        field: EnumPack_.TestEnum.TEST_ENUM_VALUE1, answer: 1 },
                    { tag: "EnumPack_.TestEnum.TEST_ENUM_VALUE2 == 2",
                        field: EnumPack_.TestEnum.TEST_ENUM_VALUE2, answer: 2 },
                    { tag: "EnumPack_.TestEnum.TEST_ENUM_VALUE3 == 4",
                        field: EnumPack_.TestEnum.TEST_ENUM_VALUE3, answer: 4 },
                    { tag: "EnumPack_.TestEnum.TEST_ENUM_VALUE4 == 3",
                        field: EnumPack_.TestEnum.TEST_ENUM_VALUE4, answer: 3 },
                    // TestEnumSecondInFile
                    { tag: "TestEnumSecondInFile.TEST_ENUM_SIF_VALUE0 == 0",
                        field: TestEnumSecondInFile.TEST_ENUM_SIF_VALUE0, answer: 0 },
                    { tag: "TestEnumSecondInFile.TEST_ENUM_SIF_VALUE1 == 1",
                        field: TestEnumSecondInFile.TEST_ENUM_SIF_VALUE1, answer: 1 },
                    { tag: "TestEnumSecondInFile.TEST_ENUM_SIF_VALUE2 == 2",
                        field: TestEnumSecondInFile.TEST_ENUM_SIF_VALUE2, answer: 2 },
                    // EnumPack_.TestEnumSecondInFile
                    { tag: "EnumPack_.TestEnumSecondInFile.TEST_ENUM_SIF_VALUE0 == 0",
                        field: EnumPack_.TestEnumSecondInFile.TEST_ENUM_SIF_VALUE0,
                        answer: 0 },
                    { tag: "EnumPack_.TestEnumSecondInFile.TEST_ENUM_SIF_VALUE1 == 1",
                        field: EnumPack_.TestEnumSecondInFile.TEST_ENUM_SIF_VALUE1,
                        answer: 1 },
                    { tag: "EnumPack_.TestEnumSecondInFile.TEST_ENUM_SIF_VALUE2 == 2",
                        field: EnumPack_.TestEnumSecondInFile.TEST_ENUM_SIF_VALUE2,
                        answer: 2 },
                    // SimpleEnumMessage
                    { tag: "SimpleEnumMessage.LOCAL_ENUM_VALUE0 == 0",
                        field: SimpleEnumMessage.LOCAL_ENUM_VALUE0, answer: 0 },
                    { tag: "SimpleEnumMessage.LOCAL_ENUM_VALUE1 == 1",
                        field: SimpleEnumMessage.LOCAL_ENUM_VALUE1, answer: 1 },
                    { tag: "SimpleEnumMessage.LOCAL_ENUM_VALUE2 == 2",
                        field: SimpleEnumMessage.LOCAL_ENUM_VALUE2, answer: 2 },
                    { tag: "SimpleEnumMessage.LOCAL_ENUM_VALUE3 == 3",
                        field: SimpleEnumMessage.LOCAL_ENUM_VALUE3, answer: 3 },
                    { tag: "SimpleEnumMessage.LocalEnumProtoFieldNumber == 1",
                        field: SimpleEnumMessage.LocalEnumProtoFieldNumber, answer: 1 },
                    // EnumPack_.SimpleEnumMessage
                    { tag: "EnumPack_.SimpleEnumMessage.LOCAL_ENUM_VALUE0 == 0",
                        field: EnumPack_.SimpleEnumMessage.LOCAL_ENUM_VALUE0,
                        answer: 0 },
                    { tag: "EnumPack_.SimpleEnumMessage.LOCAL_ENUM_VALUE1 == 1",
                        field: EnumPack_.SimpleEnumMessage.LOCAL_ENUM_VALUE1,
                        answer: 1 },
                    { tag: "EnumPack_.SimpleEnumMessage.LOCAL_ENUM_VALUE2 == 2",
                        field: EnumPack_.SimpleEnumMessage.LOCAL_ENUM_VALUE2,
                        answer: 2 },
                    { tag: "EnumPack_.SimpleEnumMessage.LOCAL_ENUM_VALUE3 == 3",
                        field: EnumPack_.SimpleEnumMessage.LOCAL_ENUM_VALUE3,
                        answer: 3 },
                    { tag: "EnumPack_.SimpleEnumMessage.LocalEnumProtoFieldNumber == 1",
                        field: EnumPack_.SimpleEnumMessage.LocalEnumProtoFieldNumber,
                        answer: 1 },
                    // RepeatedEnumMessage
                    { tag: "RepeatedEnumMessage.LOCAL_ENUM_VALUE0 == 0",
                        field: RepeatedEnumMessage.LOCAL_ENUM_VALUE0, answer: 0 },
                    { tag: "RepeatedEnumMessage.LOCAL_ENUM_VALUE1 == 1",
                        field: RepeatedEnumMessage.LOCAL_ENUM_VALUE1, answer: 1 },
                    { tag: "RepeatedEnumMessage.LOCAL_ENUM_VALUE2 == 2",
                        field: RepeatedEnumMessage.LOCAL_ENUM_VALUE2, answer: 2 },
                    { tag: "RepeatedEnumMessage.LOCAL_ENUM_VALUE3 == 3",
                        field: RepeatedEnumMessage.LOCAL_ENUM_VALUE3, answer: 3 },
                    { tag: "RepeatedEnumMessage.LocalEnumListProtoFieldNumber == 1",
                        field: RepeatedEnumMessage.LocalEnumListProtoFieldNumber, answer: 1 },
                    // MixedEnumUsageMessage
                    { tag: "MixedEnumUsageMessage.LOCAL_ENUM_VALUE0 == 0",
                        field: MixedEnumUsageMessage.LOCAL_ENUM_VALUE0, answer: 0 },
                    { tag: "MixedEnumUsageMessage.LOCAL_ENUM_VALUE1 == 1",
                        field: MixedEnumUsageMessage.LOCAL_ENUM_VALUE1, answer: 1 },
                    { tag: "MixedEnumUsageMessage.LOCAL_ENUM_VALUE2 == 2",
                        field: MixedEnumUsageMessage.LOCAL_ENUM_VALUE2, answer: 2 },
                    { tag: "MixedEnumUsageMessage.LOCAL_ENUM_VALUE3 == 3",
                        field: MixedEnumUsageMessage.LOCAL_ENUM_VALUE3, answer: 3 },
                    { tag: "MixedEnumUsageMessage.LocalEnumProtoFieldNumber == 1",
                        field: MixedEnumUsageMessage.LocalEnumProtoFieldNumber, answer: 1 },
                    { tag: "MixedEnumUsageMessage.LocalEnumListProtoFieldNumber == 2",
                        field: MixedEnumUsageMessage.LocalEnumListProtoFieldNumber, answer: 2 },
                    { tag: "MixedEnumUsageMessage.LocalEnumMapProtoFieldNumber == 3",
                        field: MixedEnumUsageMessage.LocalEnumMapProtoFieldNumber, answer: 3 },
                    // EnumPack_.MixedEnumUsageMessage
                    { tag: "EnumPack_.MixedEnumUsageMessage.LOCAL_ENUM_VALUE0 == 0",
                        field: EnumPack_.MixedEnumUsageMessage.LOCAL_ENUM_VALUE0,
                        answer: 0 },
                    { tag: "EnumPack_.MixedEnumUsageMessage.LOCAL_ENUM_VALUE1 == 1",
                        field: EnumPack_.MixedEnumUsageMessage.LOCAL_ENUM_VALUE1,
                        answer: 1 },
                    { tag: "EnumPack_.MixedEnumUsageMessage.LOCAL_ENUM_VALUE2 == 2",
                        field: EnumPack_.MixedEnumUsageMessage.LOCAL_ENUM_VALUE2,
                        answer: 2 },
                    { tag: "EnumPack_.MixedEnumUsageMessage.LOCAL_ENUM_VALUE3 == 3",
                        field: EnumPack_.MixedEnumUsageMessage.LOCAL_ENUM_VALUE3,
                        answer: 3 },
                    { tag: "EnumPack_.MixedEnumUsageMessage.LocalEnumProtoFieldNumber == 1",
                        field: EnumPack_.MixedEnumUsageMessage.LocalEnumProtoFieldNumber,
                        answer: 1 },
                    { tag: "EnumPack_.MixedEnumUsageMessage.LocalEnumListProtoFieldNumber == 2",
                        field: EnumPack_.MixedEnumUsageMessage.LocalEnumListProtoFieldNumber,
                        answer: 2 },
                    { tag: "EnumPack_.MixedEnumUsageMessage.LocalEnumMapProtoFieldNumber == 3",
                        field: EnumPack_.MixedEnumUsageMessage.LocalEnumMapProtoFieldNumber,
                        answer: 3 },
                    { tag: "A.AVal0 == 0", field: A.AVal0, answer: 0 },
                    { tag: "A.AVal1 == 1", field: A.AVal1, answer: 1 },
                    { tag: "B.BVal0 == 0", field: B.BVal0, answer: 0 },
                    { tag: "B.BVal1 == 1", field: B.BVal1, answer: 1 }
                ]
    }

    function test_enumValues(data) {
        compare(data.field, data.answer)
    }

    function test_checkMessagesFields_data() {
        return [
                    { tag: "localEnumMessage.localEnum == 3",
                        field: localEnumMessage.localEnum, answer: 3 },
                    { tag: "EnumPack_.localEnumMessage2.localEnum == 3",
                        field: localEnumMessage2.localEnum, answer: 3 },
                    { tag: "localMixedEnumMessage.localEnum == 3",
                        field: localMixedEnumMessage.localEnum, answer: 3 },
                    { tag: "EnumPack_.localMixedEnumMessage2.localEnum == 3",
                        field: localMixedEnumMessage2.localEnum, answer: 3 },
                    { tag: "globalEnumMessage.globalEnum == 3",
                        field: globalEnumMessage.globalEnum, answer: 3 },
                    { tag: "localMixedEnumMessage.localEnumList[0] == 0",
                        field: localMixedEnumMessage.localEnumList[0],
                        answer: 0 },
                    { tag: "localMixedEnumMessage.localEnumList[1] == 1",
                        field: localMixedEnumMessage.localEnumList[1],
                        answer: 1 },
                    { tag: "localRepeatedEnumMessage.localEnumList[0] == 0",
                        field: localRepeatedEnumMessage.localEnumList[0],
                        answer: 0 },
                    { tag: "localRepeatedEnumMessage.localEnumList[1] == 1",
                        field: localRepeatedEnumMessage.localEnumList[1],
                        answer: 1 },
                    { tag: "localRepeatedEnumMessage.localEnumList[2] == 3",
                        field: localRepeatedEnumMessage.localEnumList[2],
                        answer: 3 },
                    { tag: "globalEnumMessage.globalEnumList[0] == 0",
                        field: globalEnumMessage.globalEnumList[0],
                        answer: 0 },
                    { tag: "globalEnumMessage.globalEnumList[1] == 1",
                        field: globalEnumMessage.globalEnumList[1],
                        answer: 1 },
                    { tag: "localStepChildEnumMessage.localStepChildList[0] == 0",
                        field: localStepChildEnumMessage.localStepChildList[0],
                        answer: 0 },
                    { tag: "localStepChildEnumMessage.localStepChildList[1] == 1",
                        field: localStepChildEnumMessage.localStepChildList[1],
                        answer: 1 },
                ]
    }

    function test_checkMessagesFields(data) {
        compare(data.field, data.answer)
    }

    function test_enumListSizes_data() {
        return [
                    { tag: "MixedEnumUsageMessage enum list size == 2",
                        field: localMixedEnumMessage.localEnumList.length, answer: 2 },
                    { tag: "StepChildEnumMessage enum list size == 2",
                        field: localStepChildEnumMessage.localStepChildList.length, answer: 2 },
                    { tag: "RepeatedEnumMessage enum list size == 3",
                        field: localRepeatedEnumMessage.localEnumList.length, answer: 3 },
                    { tag: "Global TestEnum list size == 2",
                        field: globalEnumMessage.globalEnumList.length, answer: 2 },
                ]
    }

    function test_enumListSizes(data) {
        compare(data.field, data.answer)
    }

    function test_checkMessageTypes_data() {
        return [
                    { tag: "localEnumMessage.localEnum is number",
                        field: typeof(localEnumMessage.localEnum), answer: "number" },
                    { tag: "localMixedEnumMessage.localEnum is number",
                        field: typeof(localMixedEnumMessage.localEnum), answer: "number" },
                    { tag: "EnumPack_.localEnumMessage2.localEnum is number",
                        field: typeof(localEnumMessage2.localEnum), answer: "number" },
                    { tag: "localMixedEnumMessage2.localEnum is number",
                        field: typeof(localMixedEnumMessage2.localEnum), answer: "number" },
                    { tag: "globalEnumMessage.globalEnum is number",
                        field: typeof(globalEnumMessage.globalEnum), answer: "number" },
                    { tag: "localMixedEnumMessage.localEnumList is object",
                        field: typeof(localMixedEnumMessage.localEnumList),
                        answer: "object" },
                    { tag: "localMixedEnumMessage.localEnumList[0] is number",
                        field: typeof(localMixedEnumMessage.localEnumList[0]),
                        answer: "number" },
                    { tag: "localMixedEnumMessage.localEnumList[1] is number",
                        field: typeof(localMixedEnumMessage.localEnumList[1]),
                        answer: "number" },
                    { tag: "localRepeatedEnumMessage.localEnumList is object",
                        field: typeof(localRepeatedEnumMessage.localEnumList),
                        answer: "object" },
                    { tag: "localRepeatedEnumMessage.localEnumList[0] is number",
                        field: typeof(localRepeatedEnumMessage.localEnumList[0]),
                        answer: "number" },
                    { tag: "localRepeatedEnumMessage.localEnumList[1] is number",
                        field: typeof(localRepeatedEnumMessage.localEnumList[1]),
                        answer: "number" },
                    { tag: "localRepeatedEnumMessage.localEnumList[2] is number",
                        field: typeof(localRepeatedEnumMessage.localEnumList[2]),
                        answer: "number" },
                    { tag: "globalEnumMessage.globalEnumList is object",
                        field: typeof(globalEnumMessage.globalEnumList),
                        answer: "object" },
                    { tag: "globalEnumMessage.globalEnumList[0] is number",
                        field: typeof(globalEnumMessage.globalEnumList[0]),
                        answer: "number" },
                    { tag: "globalEnumMessage.globalEnumList[1] is number",
                        field: typeof(globalEnumMessage.globalEnumList[1]),
                        answer: "number" },
                    { tag: "localStepChildEnumMessage.localStepChildList is object",
                        field: typeof(localStepChildEnumMessage.localStepChildList),
                        answer: "object" },
                    { tag: "localStepChildEnumMessage.localStepChildList[0] == 0",
                        field: typeof(localStepChildEnumMessage.localStepChildList[0]),
                        answer: "number" },
                    { tag: "localStepChildEnumMessage.localStepChildList[1] == 1",
                        field: typeof(localStepChildEnumMessage.localStepChildList[1]),
                        answer: "number" },
                ]
    }

    function test_checkMessageTypes(data) {
        compare(data.field, data.answer)
    }

}
