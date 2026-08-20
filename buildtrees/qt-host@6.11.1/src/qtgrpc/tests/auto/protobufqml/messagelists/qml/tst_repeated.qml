// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtTest

import qtprotobufnamespace.tests

TestCase {
    name: "qtprotobufRepeatedTest"

    property repeatedStringMessage rStrMsg;
    property repeatedStringMessage rComparisonStrMsg;
    property repeatedDoubleMessage rDoubleMsg;
    property repeatedBytesMessage rByteMsg;
    property repeatedFloatMessage rFloatMsg;
    property repeatedComplexMessage rComplexMsg;
    property repeatedSIntMessage rSIntMsg;
    property repeatedIntMessage rIntMsg;
    property repeatedUIntMessage rUIntMsg;
    property repeatedSInt64Message rSInt64Msg;
    property repeatedInt64Message rInt64Msg;
    property repeatedUInt64Message rUInt64Msg;
    property repeatedFixedIntMessage rFixedIntMsg;
    property repeatedSFixedIntMessage rSFixedIntMsg;
    property repeatedFixedInt64Message rFixedInt64Msg;
    property repeatedSFixedInt64Message rSFixedInt64Msg;
    property repeatedBoolMessage rBoolMsg;
    property complexMessage msg1;
    property complexMessage msg2;
    property complexMessage msg3;

    function initTestCase() {
        msg1.testComplexField.testFieldString = "complexMessage 1"
        msg2.testComplexField.testFieldString = "complexMessage 2"

        //TODO: see QTBUG-113690
        rStrMsg.testRepeatedString = ["String", "Second String",
                                      "123 third string", "Ping ***"]
        rComparisonStrMsg.testRepeatedString = ["String", "Second String",
                                                "123 third string", "Ping ***"]
        rDoubleMsg.testRepeatedDouble = [0, 0.0, 1.23456, 2147483648.09, -17.111]
        rFloatMsg.testRepeatedFloat = [0, 0.0, 1.23456, 214748.09375, -17.111]
        rComplexMsg.testRepeatedComplexData = [msg1, msg2]
        rSIntMsg.testRepeatedInt = [0, 65536, -65536, 2]
        rUIntMsg.testRepeatedInt = [0, 65536, 200, 2]
        rSInt64Msg.testRepeatedInt = [0, 65536, -65536, 2]
        rUInt64Msg.testRepeatedInt = [0, 65536, 200, 2]
        rBoolMsg.testRepeatedBool = [true, false, true, false, true, false]

        rByteMsg.testRepeatedBytes = ["\x02hi!\x03\x00", "\x00"]
        rSFixedIntMsg.testRepeatedInt = [0, 65536, -65536, 2]
        rFixedInt64Msg.testRepeatedInt = [0, 65536, 200, 2]
        rSFixedInt64Msg.testRepeatedInt = [0, 65536, -65536, 2]
        rFixedIntMsg.testRepeatedInt = [0, 65536, 2147483647, 2]
        rIntMsg.testRepeatedInt = [0, 65536, -65536, 2]
        rInt64Msg.testRepeatedInt = [0, 65536, -65536, 2]

        rDoubleMsg.testRepeatedDouble[4] = -4567.111
        rFloatMsg.testRepeatedFloat[4] = -4567.11083984375
        rBoolMsg.testRepeatedBool[4] = false
        // Assignment to a exact list element causes assert in qmlcachgen.
        // To be investigated. Skip assginment by now. See QTBUG-131930.
        // rSIntMsg.testRepeatedInt[3] = 5
        // rSInt64Msg.testRepeatedInt[3] = 5
        // rFixedIntMsg.testRepeatedInt[3] = 5
        // rUIntMsg.testRepeatedInt[3] = 5
        // rUInt64Msg.testRepeatedInt[3] = 5
        // rIntMsg.testRepeatedInt[3] = 5
        // rInt64Msg.testRepeatedInt[3] = 5
    }

    function test_repeatedTypes_data() {
        return [
                    // repeatedComplexMessage
                    { tag: "rComplexMsg.testRepeatedComplex is an object",
                        field: typeof(rComplexMsg.testRepeatedComplexData), answer: "object" },
                    { tag: "rComplexMsg.testRepeatedComplex[i] is a string",
                        field: typeof(rComplexMsg.testRepeatedComplexData[0]), answer: "object" },

                    // repeatedStringMessage
                    { tag: "rStrMsg.testRepeatedString is an object",
                        field: typeof(rStrMsg.testRepeatedString), answer: "object" },
                    { tag: "rStrMsg.testRepeatedString[i] is a string",
                        field: typeof(rStrMsg.testRepeatedString[0]), answer: "string" },

                    // repeatedBytesMessage
                    { tag: "rByteMsg.testRepeatedBytes is an object",
                        field: typeof(rByteMsg.testRepeatedBytes), answer: "object" },
                    { tag: "rByteMsg.testRepeatedBytes[i] is a object",
                        field: typeof(rByteMsg.testRepeatedBytes[0]), answer: "object" },

                    // repeatedDoubleMessage
                    { tag: "rDoubleMsg.testRepeatedDouble is an object",
                        field: typeof(rDoubleMsg.testRepeatedDouble), answer: "object" },
                    { tag: "rDoubleMsg.testRepeatedDouble[i] is a number",
                        field: typeof(rDoubleMsg.testRepeatedDouble[0]), answer: "number" },

                    // repeatedSIntMessage
                    { tag: "rSIntMsg.testRepeatedInt is an object",
                        field: typeof(rSIntMsg.testRepeatedInt), answer: "object" },
                    { tag: "rSIntMsg.testRepeatedInt[i] is a number",
                        field: typeof(rSIntMsg.testRepeatedInt[0]), answer: "number" },

                    // repeatedSInt64Message
                    { tag: "rSInt64Msg.testRepeatedInt is an object",
                        field: typeof(rSInt64Msg.testRepeatedInt), answer: "object" },
                    { tag: "rSInt64Msg.testRepeatedInt[i] is a number",
                        field: typeof(rSInt64Msg.testRepeatedInt[0]), answer: "number" },

                    // repeatedUIntMessage
                    { tag: "rUIntMsg.testRepeatedInt is an object",
                        field: typeof(rUIntMsg.testRepeatedInt), answer: "object" },
                    { tag: "rUIntMsg.testRepeatedInt[i] is a number",
                        field: typeof(rUIntMsg.testRepeatedInt[0]), answer: "number" },

                    // repeatedUInt64Message
                    { tag: "rUInt64Msg.testRepeatedInt is an object",
                        field: typeof(rUInt64Msg.testRepeatedInt), answer: "object" },
                    { tag: "rUInt64Msg.testRepeatedInt[i] is a number",
                        field: typeof(rUInt64Msg.testRepeatedInt[0]), answer: "number" },

                    // repeatedFloatMessage
                    { tag: "rFloatMsg.testRepeatedFloat is an object",
                        field: typeof(rFloatMsg.testRepeatedFloat), answer: "object" },
                    { tag: "rFloatMsg.testRepeatedFloat[i] is a number",
                        field: typeof(rFloatMsg.testRepeatedFloat[0]), answer: "number" },

                    // repeatedBoolMessage
                    { tag: "rBoolMsg.testRepeatedBool is an object",
                        field: typeof(rBoolMsg.testRepeatedBool), answer: "object" },
                    { tag: "rBoolMsg.testRepeatedBool[i] is a number",
                        field: typeof(rBoolMsg.testRepeatedBool[0]), answer: "boolean" },

                    // repeatedIntMessage
                    { tag: "rIntMsg.testRepeatedInt is an object",
                        field: typeof(rIntMsg.testRepeatedInt), answer: "object" },
                    { tag: "rIntMsg.testRepeatedInt[i] is a object",
                        field: typeof(rIntMsg.testRepeatedInt[0]), answer: "object" },

                    // repeatedIntMessage
                    { tag: "rInt64Msg.testRepeatedInt is an object",
                        field: typeof(rInt64Msg.testRepeatedInt), answer: "object" },
                    { tag: "rInt64Msg.testRepeatedInt[i] is a object",
                        field: typeof(rInt64Msg.testRepeatedInt[0]), answer: "object" },
                ]
    }

    function test_repeatedTypes(data) {
        compare(data.field, data.answer)
    }

    function test_repeatedValues_data() {
        return [
                    // repeatedComplexMessage
                    { tag: "rComplexMsg.testRepeatedComplexData size == 2",
                        field: rComplexMsg.testRepeatedComplexData.length, answer: 2 },

                    // repeatedStringMessage
                    { tag: "rStrMsg.testRepeatedString size == 4",
                        field: rStrMsg.testRepeatedString.length, answer: 4 },
                    { tag: "rStrMsg.testRepeatedString[0] == String",
                        field: rStrMsg.testRepeatedString[0], answer: "String" },
                    { tag: "rStrMsg.testRepeatedString[3] == String",
                        field: rStrMsg.testRepeatedString[3], answer: "Ping ***" },
                    { tag: "rStrMsg.testRepeatedString[2] assigned to Qwerty!",
                        field: rStrMsg.testRepeatedString,
                        answer: rComparisonStrMsg.testRepeatedString },

                    // repeatedDoubleMessage
                    { tag: "rDoubleMsg.testRepeatedDouble size == 5",
                        field: rDoubleMsg.testRepeatedDouble.length, answer: 5 },
                    { tag: "rDoubleMsg.testRepeatedDouble[0] == 0",
                        field: rDoubleMsg.testRepeatedDouble[0], answer: 0 },
                    { tag: "rDoubleMsg.testRepeatedDouble[3] == 2147483648.09",
                        field: rDoubleMsg.testRepeatedDouble[3], answer: 2147483648.09 },
                    { tag: "rDoubleMsg.testRepeatedDouble[4] assigned to -4567.111",
                        field: rDoubleMsg.testRepeatedDouble[4], answer: -4567.111 },

                    // repeatedSIntMessage
                    { tag: "rSIntMsg.testRepeatedInt size == 4",
                        field: rSIntMsg.testRepeatedInt.length, answer: 4 },
                    { tag: "rSIntMsg.testRepeatedInt[0] == 0",
                        field: rSIntMsg.testRepeatedInt[0], answer: 0 },
                    { tag: "rSIntMsg.testRepeatedInt[2] == -65536",
                        field: rSIntMsg.testRepeatedInt[2], answer: -65536 },

                    // repeatedSInt64Message
                    { tag: "rSInt64Msg.testRepeatedInt size == 4",
                        field: rSInt64Msg.testRepeatedInt.length, answer: 4 },
                    { tag: "rSInt64Msg.testRepeatedInt[0] == 0",
                        field: rSInt64Msg.testRepeatedInt[0], answer: 0 },
                    { tag: "rSInt64Msg.testRepeatedInt[2] == -65536",
                        field: rSInt64Msg.testRepeatedInt[2], answer: -65536 },

                    // repeatedUIntMessage
                    { tag: "rUIntMsg.testRepeatedInt size == 4",
                        field: rUIntMsg.testRepeatedInt.length, answer: 4 },
                    { tag: "rUIntMsg.testRepeatedInt[0] == 0",
                        field: rUIntMsg.testRepeatedInt[0], answer: 0 },
                    { tag: "rUIntMsg.testRepeatedInt[2] == 200",
                        field: rUIntMsg.testRepeatedInt[2], answer: 200 },

                    // repeatedUInt64Message
                    { tag: "rUInt64Msg.testRepeatedInt size == 4",
                        field: rUInt64Msg.testRepeatedInt.length, answer: 4 },
                    { tag: "rUInt64Msg.testRepeatedInt[0] == 0",
                        field: rUInt64Msg.testRepeatedInt[0], answer: 0 },
                    { tag: "rUInt64Msg.testRepeatedInt[2] == 200",
                        field: rUInt64Msg.testRepeatedInt[2], answer: 200 },

                    // repeatedFloatMessage
                    { tag: "rFloatMsg.testRepeatedFloat size == 5",
                        field: rFloatMsg.testRepeatedFloat.length, answer: 5 },
                    { tag: "rFloatMsg.testRepeatedFloat[0] == 0",
                        field: rFloatMsg.testRepeatedFloat[0], answer: 0 },
                    { tag: "rFloatMsg.testRepeatedFloat[3] == 214748.09375",
                        field: rFloatMsg.testRepeatedFloat[3], answer: 214748.09375 },
                    { tag: "rFloatMsg.testRepeatedFloat[4] assigned to -4567.11083984375",
                        field: rFloatMsg.testRepeatedFloat[4], answer: -4567.11083984375 },

                    // repeatedBoolMessage
                    { tag: "rBoolMsg.testRepeatedBool size == 6",
                        field: rBoolMsg.testRepeatedBool.length, answer: 6 },
                    { tag: "rBoolMsg.testRepeatedBool[0] == true",
                        field: rBoolMsg.testRepeatedBool[0], answer: true },
                    { tag: "rBoolMsg.testRepeatedBool[4] assigned to false",
                        field: rBoolMsg.testRepeatedBool[4], answer: false },
                ]
    }

    function test_repeatedValues(data) {
        compare(data.field, data.answer)
    }

    function test_repeatedValuesProtobufTypes_data() {
        return [
                    // repeatedInt64Message
                    { tag: "rInt64Msg.testRepeatedInt size == 4",
                        field: rInt64Msg.testRepeatedInt.length, answer: 4 },
                    { tag: "rInt64Msg.testRepeatedInt[0] == 0",
                        field: rInt64Msg.testRepeatedInt[0], answer: 0 },
                    { tag: "rInt64Msg.testRepeatedInt[2] == -65536",
                        field: rInt64Msg.testRepeatedInt[2], answer: -65536 },

                    // repeatedIntMessage
                    { tag: "rIntMsg.testRepeatedInt size == 4",
                        field: rIntMsg.testRepeatedInt.length, answer: 4 },
                    { tag: "rIntMsg.testRepeatedInt[0] == 0",
                        field: rIntMsg.testRepeatedInt[0], answer: 0 },
                    { tag: "rIntMsg.testRepeatedInt[2] == -65536",
                        field: rIntMsg.testRepeatedInt[2], answer: -65536 },

                    // repeatedFixedIntMessage
                    { tag: "rFixedIntMsg.testRepeatedInt size == 4",
                        field: rFixedIntMsg.testRepeatedInt.length, answer: 4 },
                    { tag: "rFixedIntMsg.testRepeatedInt[0] == 0",
                        field: rFixedIntMsg.testRepeatedInt[0], answer: 0 },
                    { tag: "rFixedIntMsg.testRepeatedInt[2] == 2147483647",
                        field: rFixedIntMsg.testRepeatedInt[2], answer: 2147483647 }
                ]
    }

    function test_repeatedValuesProtobufTypes(data) {
        verify(data.field == data.answer) // those values don't handle "===" operation
    }
}
