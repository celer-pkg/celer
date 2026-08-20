// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtTest

import qtprotobufnamespace.tests

TestCase {
    name: "qtrotobufSyntaxTest"

    property message_Uderscore_name underscore_name;
    property messageUderscorename underscoreName;
    property messageUnderscoreField msgUnderscoreName;
    property followingMessageUnderscoreField fUnderscoreName;
    property combinedMessageUnderscoreField combinedName;
    property messageUpperCaseReserved upperCaseReserved;
    property lowerCaseFieldMessageName lowerCaseFieldName;
    property messageEnumReserved msgEnumReserved;
    property messageUpperCase msgUpperCase;
    property messageReserved msgReserved;
    property priorMessageUnderscoreField underScoreMsg;
    property lowerCaseMessageName lowerCaseMsg;
    property nameClashingMutableGetters mutableGettersMsg;

    function initTestCase() {
        underscore_name.testField = -7
        underscoreName.testField = 100
        msgUnderscoreName.underScoreMessageField = 100
        fUnderscoreName.underScoreMessageField = 1
        combinedName.underScoreMessageField = 200
        upperCaseReserved.import_proto = 123
        upperCaseReserved.property_proto = 456
        upperCaseReserved.id_proto = 789
        lowerCaseFieldName.testField.testField = 7
        msgReserved.id_proto = 34;
        msgReserved.import_proto = 35;
        msgReserved.property_proto = 36;
        underScoreMsg.underScoreMessageField = 123
        msgUpperCase.testField = 34
        lowerCaseMsg.testField = 34
    }

    function test_Names_data() {
        return [
                    { tag: "msgReserved.id_proto == 34",
                        field: msgReserved.id_proto, answer: 34 },
                    { tag: "msgReserved.import_proto == 35",
                        field: msgReserved.import_proto, answer: 35 },
                    { tag: "msgReserved.property_proto == 36",
                        field: msgReserved.property_proto, answer: 36 },
                    { tag: "underscore_name.testField == -7",
                        field: underscore_name.testField, answer: -7 },
                    { tag: "underscoreName.testField == 100",
                        field: underscoreName.testField, answer: 100 },
                    { tag: "msgUnderscoreName.underScoreMessageField == 100",
                        field: msgUnderscoreName.underScoreMessageField, answer: 100 },
                    { tag: "fUnderscoreName.underScoreMessageField == 1",
                        field: fUnderscoreName.underScoreMessageField, answer: 1 },
                    { tag: "combinedName._underScoreMessage_Field_ == 200",
                        field: combinedName.underScoreMessageField, answer: 200 },
                    { tag: "upperCaseReserved.import_proto == 123",
                        field: upperCaseReserved.import_proto, answer: 123 },
                    { tag: "upperCaseReserved.property_proto == 456",
                        field: upperCaseReserved.property_proto, answer: 456 },
                    { tag: "upperCaseReserved.id_proto == 789",
                        field: upperCaseReserved.id_proto, answer: 789 },
                    { tag: "lowerCaseFieldName.testField.testField == 7",
                        field: lowerCaseFieldName.testField.testField, answer: 7 },
                    { tag: "underScoreMsg.underScoreMessageField == 123",
                        field: underScoreMsg.underScoreMessageField, answer: 123 },
                    { tag: "msgUpperCase.testField == 34",
                        field: msgUpperCase.testField, answer: 34 },
                    { tag: "lowerCaseMsg.testField == 34",
                        field: lowerCaseMsg.testField, answer: 34 },
                ]
    }

    function test_Names(data) {
        compare(data.field, data.answer)
    }

    function test_enumValues_data() {
        return [
                    { tag: "MessageEnumReserved.Import == 0",
                        field: MessageEnumReserved.Import, answer: 0 },
                    { tag: "MessageEnumReserved.Property == 1",
                        field: MessageEnumReserved.Property, answer: 1 },
                    { tag: "MessageEnumReserved.Id == 2",
                        field: MessageEnumReserved.Id, answer: 2 },

                    { tag: "MessageEnumReserved.enumValue0 == 0",
                        field: MessageEnumReserved.EnumValue0, answer: 0 },
                    { tag: "MessageEnumReserved.enumValue1 == 1",
                        field: MessageEnumReserved.EnumValue1, answer: 1 },
                    { tag: "MessageEnumReserved.enumValue2 == 2",
                        field: MessageEnumReserved.EnumValue2, answer: 2 },

                    { tag: "MessageEnumReserved._enumUnderscoreValue0 is undefined",
                        field: MessageEnumReserved._enumUnderscoreValue0, answer: undefined },
                    { tag: "MessageEnumReserved._EnumUnderscoreValue1 is undefined",
                        field: MessageEnumReserved._EnumUnderscoreValue1, answer: undefined }
                ]
    }

    function test_enumValues(data) {
        compare(data.field, data.answer)
    }

    function test_mutableGetters() {
        mutableGettersMsg.field.data = 1;
        mutableGettersMsg.mutField.data = 2;

        compare(Number(mutableGettersMsg.field.data), 1);
        expectFailContinue("", "Property getter of 'mutField' accesses non-const getter,"+
            " which clashes to mutable getter of 'field'. See QTBUG-119912.")
        compare(Number(mutableGettersMsg.mutField.data), 2);
    }
}
