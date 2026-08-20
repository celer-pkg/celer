// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtTest

import QmlTestUri

pragma ValueTypeBehavior: Addressable

Item {
    id: root

    property var usLocale: Qt.locale("en_US")

    TestCase {
        name: "qtprotobufBasicTest"

        property simpleBoolMessage boolMsg
        property simpleIntMessage int32Msg
        property simpleSIntMessage sint32Msg
        property simpleUIntMessage uint32Msg
        property simpleFixedInt32Message fixed32Msg
        property simpleSFixedInt32Message sfixed32Msg
        property simpleStringMessage stringMsg
        property complexMessage complexMsg
        property simpleStringMessage outerMessage
        property simpleStringMessage innerMessage
        property complexMessage complexMsgNoInit
        property simpleInt64Message int64Msg
        property simpleSInt64Message sint64Msg
        property simpleUInt64Message uint64Msg
        property simpleFixedInt64Message fixed64Msg
        property simpleSFixedInt64Message sfixed64Msg

        property complexMessage clearComplexMsg

        // Messages with fields not compatible with QML
        property simpleBytesMessage bytesMessage

        function initTestCase() {
            // 32 size
            int32Msg.testFieldInt = 2147483647
            sint32Msg.testFieldInt = 2147483647
            uint32Msg.testFieldInt = 4294967295
            fixed32Msg.testFieldFixedInt32 = 4294967295
            sfixed32Msg.testFieldFixedInt32 = 2147483647

            // 64 size
            int64Msg.testFieldInt = 2147483647
            sint64Msg.testFieldInt = 2147483647
            uint64Msg.testFieldInt = 4294967295
            fixed64Msg.testFieldFixedInt64 = 4294967295
            sfixed64Msg.testFieldFixedInt64 = 2147483647

            // strings
            stringMsg.testFieldString = "Test string"
            outerMessage.testFieldString = "outer"
            innerMessage.testFieldString = "inner"
            complexMsg.testComplexField = innerMessage
            complexMsg.testComplexField.testFieldString = "inner"
        }

        function test_protobufTypesTypeCheck_data() {
            return [
                        {
                            tag: "SimpleBoolMessage testFieldBool type",
                            field: typeof boolMsg.testFieldBool,
                            answer: "boolean"
                        },
                        {
                            tag: "SimpleIntMessage testFieldInt type",
                            field: typeof int32Msg.testFieldInt,
                            answer: "object"
                        },
                        {
                            tag: "SimpleSIntMessage testFieldInt type",
                            field: typeof sint32Msg.testFieldInt,
                            answer: "number"
                        },
                        {
                            tag: "SimpleUIntMessage testFieldInt type",
                            field: typeof uint32Msg.testFieldInt,
                            answer: "number"
                        },
                        {
                            tag: "SimpleFixedInt32Message testFieldInt type",
                            field: typeof fixed32Msg.testFieldFixedInt32,
                            answer: "object"
                        },
                        {
                            tag: "SimpleSFixedInt32Message testFieldInt type",
                            field: typeof sfixed32Msg.testFieldFixedInt32,
                            answer: "object"
                        },
                        {
                            tag: "SimpleStringMessage testFieldString type",
                            field: typeof stringMsg.testFieldString,
                            answer: "string"
                        },
                        {
                            tag: "SimpleSInt64Message testFieldInt type",
                            field: typeof sint64Msg.testFieldInt,
                            answer: "number"
                        },
                        {
                            tag: "SimpleUInt64Message testFieldInt type",
                            field: typeof uint64Msg.testFieldInt,
                            answer: "number"
                        },
                        {
                            tag: "SimpleInt64Message testFieldInt type",
                            field: typeof int64Msg.testFieldInt,
                            answer: "object"
                        },
                        {
                            tag: "SimpleFixedInt64Message testFieldInt type",
                            field: typeof fixed64Msg.testFieldFixedInt64,
                            answer: "object"
                        },
                        {
                            tag: "SimpleSFixedInt64Message testFieldInt type",
                            field: typeof sfixed64Msg.testFieldFixedInt64,
                            answer: "object"
                        }
                    ]
        }

        function test_protobufTypesTypeCheck(data) {
            compare(data.field, data.answer)
        }

        function test_unsupportedProtobufTypesTypeCheck_data() {
            // The below types contain fields that are not supported by QML so the property is not
            // accessible but we can still check that it's valid. typeof check returns 'object' for
            // these fields.
            return [
                        {
                            tag: "SimpleBytesMessage testFieldBytes type",
                            field: typeof bytesMessage.testFieldBytes,
                            answer: "object"
                        },
                    ]
        }

        function test_unsupportedProtobufTypesTypeCheck(data) {
            compare(data.field, data.answer)
        }

        function test_complexMessageTypeCheck_data() {
            return [
                        {
                            tag: "ComplexMessage testComplexField type",
                            field: typeof complexMsgNoInit.testComplexField,
                            answer: "object"
                        },
                        {
                            tag: "ComplexMessage testComplexField exact type",
                            field: complexMsgNoInit.testComplexField as simpleStringMessage,
                            answer: "qtprotobufnamespace::tests::SimpleStringMessage()"
                        },
                        {
                            tag: "ComplexMessage testComplexField exact invalid type",
                            field: complexMsgNoInit.testComplexField as simpleSIntMessage == undefined,
                            answer: true
                        },
                        {
                            tag: "ComplexMessage testComplexField.testFieldString type",
                            field: typeof complexMsgNoInit.testComplexField.testFieldString,
                            answer: "string"
                        },
                        {
                            tag: "ComplexMessage testFieldInt type",
                            field: typeof complexMsgNoInit.testFieldInt,
                            answer: "object"
                        },
                    ]
        }

        function test_complexMessageTypeCheck(data) {
            compare(data.field, data.answer)
        }

        function test_1initializationCheck_data() {
            return [
                        {tag: "SimpleIntMessage initialization",
                            field: int32Msg.testFieldInt, answer: 2147483647 },
                        {tag: "SimpleSIntMessage initialization",
                            field: sint32Msg.testFieldInt, answer: 2147483647 },
                        {tag: "SimpleUIntMessage initialization",
                            field: uint32Msg.testFieldInt, answer: 4294967295 },
                        {tag: "SimpleFixedInt32Message initialization",
                            field: fixed32Msg.testFieldFixedInt32, answer: 4294967295 },
                        {tag: "SimpleSFixedInt32Message initialization",
                            field: sfixed32Msg.testFieldFixedInt32, answer: 2147483647 },
                        {tag: "SimpleInt64Message initialization",
                            field: int64Msg.testFieldInt, answer: 2147483647 },
                        {tag: "SimpleSInt64Message initialization",
                            field: sint64Msg.testFieldInt, answer: 2147483647 },
                        {tag: "SimpleUInt64Message initialization",
                            field: uint64Msg.testFieldInt, answer: 4294967295 },
                        {tag: "SimpleFixedInt64Message initialization",
                            field: fixed64Msg.testFieldFixedInt64, answer: 4294967295 },
                        {tag: "SimpleSFixedInt64Message initialization",
                            field: sfixed64Msg.testFieldFixedInt64, answer: 2147483647 }
                    ]
        }

        function test_1initializationCheck(data) {
            verify(data.field == data.answer)
        }

        function test_simplesstringmessage() {
            compare(stringMsg.testFieldString, "Test string", "SimpleStringMessage")
        }

        function test_int32LocaleStringConversion() {
            var field = int32Msg.testFieldInt.toLocaleString(root.usLocale)
            var answer = Number(int32Msg.testFieldInt).toLocaleString(root.usLocale)
            expectFail("", "int32Msg number is not match" + field + " not equal " + answer)
            compare(field, answer)
        }

        function test_fixed32LocaleStringConversion(data) {
            var field = fixed32Msg.testFieldFixedInt32.toLocaleString(root.usLocale)
            var answer = Number(fixed32Msg.testFieldFixedInt32).toLocaleString(root.usLocale)
            expectFail("", "fixed32LocaleString number is not match"
                       + field + " not equal "  + answer)
            compare(field, answer)
        }

        function test_sint32LocaleStringConversion() {
            compare(sint32Msg.testFieldInt.toLocaleString(root.usLocale),
                    Number(sint32Msg.testFieldInt).toLocaleString(root.usLocale),
                    "Locale number string is not match "
                    + sint32Msg.testFieldInt.toLocaleString(root.usLocale)
                    + " != " + Number(sint32Msg.testFieldInt).toLocaleString(root.usLocale))
        }

        function test_sfixed32LocaleStringConversion(data) {
            var field = sfixed32Msg.testFieldFixedInt32.toLocaleString(root.usLocale)
            var answer = Number(sfixed32Msg.testFieldFixedInt32).toLocaleString(root.usLocale)

            expectFail("", "sfixed32LocaleString number is not match" +
                       field + " not equal " + answer)
            compare(field, answer)
        }

        function test_complexMessage() {
            compare(complexMsg.testComplexField.testFieldString, "inner")
            complexMsg.testComplexField.testFieldString = "My test"
            compare(complexMsg.testComplexField.testFieldString, "My test")
            complexMsg.testComplexField = outerMessage
            compare(complexMsg.testComplexField, outerMessage)
            complexMsg.testComplexField = innerMessage
            compare(complexMsg.testComplexField, innerMessage)
            compare(complexMsg.testComplexField.testFieldString, "inner")
        }

        function test_clearMessageMethod() {
            // strings
            outerMessage.testFieldString = "outer"
            clearComplexMsg.testComplexField = outerMessage
            compare(clearComplexMsg.testComplexField.testFieldString, "outer")
            compare(outerMessage.testFieldString, "outer")

            clearComplexMsg.testComplexField.testFieldString = "inner"
            compare(clearComplexMsg.testComplexField.testFieldString, "inner")
            compare(outerMessage.testFieldString, "outer")

            clearComplexMsg.clearTestComplexField()
            compare(clearComplexMsg.testComplexField.testFieldString, "")
            compare(outerMessage.testFieldString, "outer")
        }
    }

    Repeater {
        id: simpleSInt32

        property var intArray: [0, -128, 127, -256, 255,
            -32768, 32767, -65536, 65535, -2147483648, 2147483647]
        model: simpleSInt32.intArray

        TestCase {
            name: "sint32Msg_testFieldInt"
            property simpleSIntMessage sint32Msg
            required property int modelData

            function initTestCase() {
                sint32Msg.testFieldInt = modelData
            }

            function test_simpleSIntMessageValidValueAssignment() {
                compare(sint32Msg.testFieldInt, modelData, "SimpleSIntMessage != " + modelData)
            }

            function cleanupTestCase() {
                sint32Msg.testFieldInt = 0
            }
        }
    }

    Repeater {
        id: simpleUint32
        property var intArray: [0, 127, 255, 32767, 65535, 2147483647, 4294967295]
        model: simpleUint32.intArray
        TestCase {
            name: "uint32Msg_testFieldInt"
            property simpleUIntMessage uint32Msg
            required property var modelData
            function initTestCase() {
                uint32Msg.testFieldInt = modelData
            }

            function test_simpleUIntMessageValidValueAssignment() {
                compare(uint32Msg.testFieldInt, modelData, "simpleUIntMessage != " + modelData)
            }

            function cleanupTestCase() {
                uint32Msg.testFieldInt = 0
            }
        }
    }

    Repeater {
        id: simpleFixed32
        property var intArray: [0, 127, 255, 32767, 65535, 2147483647, 4294967295]
        model: simpleFixed32.intArray
        TestCase {
            name: "fixed32Msg_testFieldFixedInt32"
            property simpleFixedInt32Message fixed32Msg
            required property var modelData
            function initTestCase() {
                fixed32Msg.testFieldFixedInt32 = modelData
            }

            function test_simpleFixedInt32MessageValidValueAssignment() {
                verify(fixed32Msg.testFieldFixedInt32
                       == modelData, "SimpleFixedInt32Message != " + modelData)
            }

            function cleanupTestCase() {
                fixed32Msg.testFieldFixedInt32 = 0
            }
        }
    }

    Repeater {
        id: simpleSFixed32
        property var intArray: [0, -128, 127, -256, 255,
            -32768, 32767, -65536, 65535, -2147483648, 2147483647]
        model: simpleSFixed32.intArray
        TestCase {
            name: "sfixed32Msg_testFieldFixedInt32"
            property simpleSFixedInt32Message sfixed32Msg
            required property int modelData
            function initTestCase() {
                sfixed32Msg.testFieldFixedInt32 = modelData
            }

            function test_simpleSFixedInt32MessageValidValueAssignment() {
                verify(sfixed32Msg.testFieldFixedInt32 == modelData,
                       "SimpleSFixedInt32Message != " + modelData)
            }

            function cleanupTestCase() {
                sfixed32Msg.testFieldFixedInt32 = 0
            }
        }
    }

    Repeater {
        id: simpleInt32

        property var intArray: [0, -128, 127, -256, 255,
            -32768, 32767, -65536, 65535, -2147483648, 2147483647]
        model: simpleInt32.intArray
        TestCase {
            name: "int32Msg_testFieldInt"
            property simpleIntMessage int32Msg
            required property int modelData
            function initTestCase() {
                int32Msg.testFieldInt = modelData
            }

            function test_simpleIntMessageValidValueAssignment() {
                verify(int32Msg.testFieldInt == modelData, "SimpleIntMessage != " + modelData)
            }

            function cleanupTestCase() {
                int32Msg.testFieldInt = 0
            }
        }
    }

    TestCase {
        name: "int32ImplicitConversionToBool"
        property simpleIntMessage int32Msg

        function test_int32ImplicitConversion_data() {
            return [
                        { tag: "true", field: 1, answer: true },
                        { tag: "false", field: 0, answer: false },
                    ]
        }

        function test_int32ImplicitConversion(data) {
            int32Msg.testFieldInt = data.field
            expectFail("false", "This is failing, because field type is an object")
            compare(int32Msg.testFieldInt ? true : false, data.answer,
                    "Invalid implicit conversion: " + int32Msg.testFieldInt
                    + " should be" + data.answer)
            // Faulty positive
            compare(int32Msg.testFieldInt == true ? true : false, data.answer,
                    "Invalid implicit conversion: " + int32Msg.testFieldInt
                    + " should be" + data.answer)
        }
    }

    TestCase {
        name: "fixed32ImplicitConversionToBool"
        property simpleFixedInt32Message fixed32Msg

        function test_fixed32ImplicitConversion_data() {
            return [
                        { tag: "true", field: 1, answer: true },
                        { tag: "false", field: 0, answer: false },
                    ]
        }

        function test_fixed32ImplicitConversion(data) {
            fixed32Msg.testFieldFixedInt32 = data.field
            expectFail("false", "This is failing, because field type is an object")
            compare(fixed32Msg.testFieldFixedInt32 ? true : false, data.answer,
                    "Invalid implicit conversion: " + fixed32Msg.testFieldFixedInt32
                    + " should be" + data.answer)
            // Faulty positive
            compare(fixed32Msg.testFieldFixedInt32 == true ? true : false, data.answer,
                    "Invalid implicit conversion: " + fixed32Msg.testFieldFixedInt32
                    + " should be" + data.answer)
        }
    }

    TestCase {
        name: "sfixed32ImplicitConversionToBool"
        property simpleSFixedInt32Message sfixed32Msg

        function test_sfixed32ImplicitConversion_data() {
            return [
                        { tag: "true", field: 1, answer: true },
                        { tag: "false", field: 0, answer: false },
                    ]
        }

        function test_sfixed32ImplicitConversion(data) {
            sfixed32Msg.testFieldFixedInt32 = data.field
            expectFail("false", "This is failing, because field type is an object")
            compare(sfixed32Msg.testFieldFixedInt32 ? true : false, data.answer,
                    "Invalid implicit conversion: " + sfixed32Msg.testFieldFixedInt32
                    + " should be" + data.answer)
            // Faulty positive
            compare(sfixed32Msg.testFieldFixedInt32 == true ? true : false, data.answer,
                    "Invalid implicit conversion: " + sfixed32Msg.testFieldFixedInt32
                    + " should be" + data.answer)
        }
    }

    TestCase {
        name: "sint32ImplicitConversionToBool"
        property simpleSIntMessage sint32Msg

        function test_sint32ImplicitConversion_data() {
            return [
                        { tag: "true", field: 1, answer: true },
                        { tag: "false", field: 0, answer: false },
                    ]
        }

        // Test passes normally because simpleSIntMessage is a number type
        function test_sint32ImplicitConversion(data) {
            sint32Msg.testFieldInt = data.field
            compare(sint32Msg.testFieldInt ? true : false, data.answer,
                    "Invalid implicit conversion: " + sint32Msg.testFieldInt
                    + " should be" + data.answer)
            compare(sint32Msg.testFieldInt == true ? true : false, data.answer,
                    "Invalid implicit conversion: " + sint32Msg.testFieldInt
                    + " should be" + data.answer)
        }
    }

    TestCase {
        name: "basicTypesPlusTest"

        property simpleIntMessage int32Msg
        property simpleSIntMessage sint32Msg
        property simpleUIntMessage uint32Msg
        property simpleFixedInt32Message fixed32Msg
        property simpleSFixedInt32Message sfixed32Msg
        property simpleStringMessage stringMsg
        property complexMessage complexMsg
        property simpleStringMessage outerMessage
        property simpleStringMessage innerMessage
        property complexMessage complexMsgNoInit
        property simpleInt64Message int64Msg
        property simpleSInt64Message sint64Msg
        property simpleUInt64Message uint64Msg
        property simpleFixedInt64Message fixed64Msg
        property simpleSFixedInt64Message sfixed64Msg

        function initTestCase() {
            int32Msg.testFieldInt = 10
            sint32Msg.testFieldInt = 100
            uint32Msg.testFieldInt = 100
            fixed32Msg.testFieldFixedInt32 = 1000
            sfixed32Msg.testFieldFixedInt32 = 1000
            int64Msg.testFieldInt = 1000
            sint64Msg.testFieldInt = 100
            uint64Msg.testFieldInt = 100
            fixed64Msg.testFieldFixedInt64 = 1000
            sfixed64Msg.testFieldFixedInt64 = 1000

            int32Msg.testFieldInt = int32Msg.testFieldInt + 10
            sint32Msg.testFieldInt = sint32Msg.testFieldInt + 100
            uint32Msg.testFieldInt = uint32Msg.testFieldInt + 100
            fixed32Msg.testFieldFixedInt32 = fixed32Msg.testFieldFixedInt32 + 1000
            sfixed32Msg.testFieldFixedInt32 = sfixed32Msg.testFieldFixedInt32 + 1000
            int64Msg.testFieldInt = int64Msg.testFieldInt + 1010
            sint64Msg.testFieldInt = sint64Msg.testFieldInt + 100
            uint64Msg.testFieldInt = uint64Msg.testFieldInt + 100
            fixed64Msg.testFieldFixedInt64 = fixed64Msg.testFieldFixedInt64 + 1000
            sfixed64Msg.testFieldFixedInt64 = sfixed64Msg.testFieldFixedInt64 + 1000
        }

        function test_basicTypesPlus_data() {
            return [
                        {
                            tag: "int32 '+' operation",
                            field: int32Msg.testFieldInt,
                            answer: 20
                        },
                        {
                            tag: "sint32 '+' operation",
                            field: sint32Msg.testFieldInt,
                            answer: 200
                        },
                        {
                            tag: "uint32 '+' operation",
                            field: uint32Msg.testFieldInt,
                            answer: 200
                        },
                        {
                            tag: "fixedInt32 '+' operation",
                            field: fixed32Msg.testFieldFixedInt32,
                            answer: 2000
                        },
                        {
                            tag: "sfixed32 '+' operation",
                            field: sfixed32Msg.testFieldFixedInt32,
                            answer: 2000
                        },
                        {
                            tag: "int64 '+' operation",
                            field: int64Msg.testFieldInt ,
                            answer: 2010
                        },
                        {
                            tag: "sint64 '+' operation",
                            field: sint64Msg.testFieldInt,
                            answer: 200
                        },
                        {
                            tag: "uint64 '+' operation",
                            field: uint64Msg.testFieldInt,
                            answer: 200
                        },
                        {
                            tag: "fixedInt64 '+' operation",
                            field: fixed64Msg.testFieldFixedInt64,
                            answer: 2000
                        },
                        {
                            tag: "sfixed64 '+' operation",
                            field: sfixed64Msg.testFieldFixedInt64,
                            answer: 2000
                        }
                    ]
        }

        function test_basicTypesPlus(data) {
            verify(data.field == data.answer,
                   "Plus operation error: " + data.field + " != " + data.answer)
        }

        function test_basicTypesPlus_ImplicitJSValueConversion_data() {
            return [
                        {
                            tag: "int32 '+' operation",
                            field: int32Msg.testFieldInt + 10,
                            answer: 30
                        },
                        {
                            tag: "sint32 '+' operation",
                            field: sint32Msg.testFieldInt + 111,
                            answer: 311
                        },
                        {
                            tag: "uint32 '+' operation",
                            field: uint32Msg.testFieldInt + 99,
                            answer: 299
                        },
                        {
                            tag: "fixedInt32 '+' operation",
                            field: fixed32Msg.testFieldFixedInt32 + 87,
                            answer: 2087
                        },
                        {
                            tag: "sfixed32 '+' operation",
                            field: sfixed32Msg.testFieldFixedInt32 + 9,
                            answer: 2009
                        },
                        {
                            tag: "int64 '+' operation",
                            field: int64Msg.testFieldInt + 1,
                            answer: 2011
                        },
                        {
                            tag: "sint64 '+' operation",
                            field: sint64Msg.testFieldInt + 0,
                            answer: 200
                        },
                        {
                            tag: "uint64 '+' operation",
                            field: uint64Msg.testFieldInt + 99,
                            answer: 299
                        },
                        {
                            tag: "fixedInt64 '+' operation",
                            field: fixed64Msg.testFieldFixedInt64 + 11,
                            answer: 2011
                        },
                        {
                            tag: "sfixed64 '+' operation",
                            field: sfixed64Msg.testFieldFixedInt64 + 10,
                            answer: 2010
                        }
                    ]
        }

        function test_basicTypesPlus_ImplicitJSValueConversion(data) {
            compare(data.field, data.answer,
                    "Plus operation error: (JS)" + data.field + " != " + data.answer)
        }
    }

    TestCase {
        name: "basicTypesAssignTest"

        property simpleIntMessage int32Msg
        property simpleSIntMessage sint32Msg
        property simpleUIntMessage uint32Msg
        property simpleFixedInt32Message fixed32Msg
        property simpleSFixedInt32Message sfixed32Msg
        property simpleStringMessage stringMsg
        property complexMessage complexMsg
        property simpleStringMessage outerMessage
        property simpleStringMessage innerMessage
        property complexMessage complexMsgNoInit
        property simpleInt64Message int64Msg
        property simpleSInt64Message sint64Msg
        property simpleUInt64Message uint64Msg
        property simpleFixedInt64Message fixed64Msg
        property simpleSFixedInt64Message sfixed64Msg

        function initTestCase() {
            int32Msg.testFieldInt = 10
            sint32Msg.testFieldInt = 100
            uint32Msg.testFieldInt = 100
            fixed32Msg.testFieldFixedInt32 = 1000
            sfixed32Msg.testFieldFixedInt32 = 1000
            int64Msg.testFieldInt = 1000
            sint64Msg.testFieldInt = 100
            uint64Msg.testFieldInt = 100
            fixed64Msg.testFieldFixedInt64 = 1000
            sfixed64Msg.testFieldFixedInt64 = 1000
        }

        function test_basicTypesAssign_data() {
            return [
                        // int32
                        {
                            tag: "int32 '=' operation",
                            field: int32Msg.testFieldInt,
                            answer: 10
                        },
                        // sint32
                        {
                            tag: "sint32 '=' operation",
                            field: sint32Msg.testFieldInt,
                            answer: 100
                        },
                        // uint32
                        {
                            tag: "uint32 '=' operation",
                            field: uint32Msg.testFieldInt,
                            answer: 100
                        },
                        // fixedInt32
                        {
                            tag: "fixedInt32 '=' operation",
                            field: fixed32Msg.testFieldFixedInt32,
                            answer: 1000
                        },
                        // sfixed32
                        {
                            tag: "sfixed32 '=' operation",
                            field: sfixed32Msg.testFieldFixedInt32,
                            answer: 1000
                        },
                        // int64
                        {
                            tag: "int64 '=' operation",
                            field: int64Msg.testFieldInt,
                            answer: 1000
                        },
                        // sint64
                        {
                            tag: "sint64 '=' operation",
                            field: sint64Msg.testFieldInt,
                            answer: 100
                        },
                        // uint64
                        {
                            tag: "uint64 '=' operation",
                            field: uint64Msg.testFieldInt,
                            answer: 100
                        },
                        // fixedInt64
                        {
                            tag: "fixedInt64 '=' operation",
                            field: fixed64Msg.testFieldFixedInt64,
                            answer: 1000
                        },
                        // sfixed64
                        {
                            tag: "sfixed64 '=' operation",
                            field: sfixed64Msg.testFieldFixedInt64,
                            answer: 1000
                        }
                    ]
        }

        function test_basicTypesAssign(data) {
            verify(data.field == data.answer,
                   "Assignment operation error: " + data.field + " != " + data.answer)
        }
    }

    TestCase {
        name: "basicTypesMinusTest"

        property simpleIntMessage int32Msg
        property simpleSIntMessage sint32Msg
        property simpleUIntMessage uint32Msg
        property simpleFixedInt32Message fixed32Msg
        property simpleSFixedInt32Message sfixed32Msg
        property simpleStringMessage stringMsg
        property complexMessage complexMsg
        property simpleStringMessage outerMessage
        property simpleStringMessage innerMessage
        property complexMessage complexMsgNoInit
        property simpleInt64Message int64Msg
        property simpleSInt64Message sint64Msg
        property simpleUInt64Message uint64Msg
        property simpleFixedInt64Message fixed64Msg
        property simpleSFixedInt64Message sfixed64Msg

        function initTestCase() {
            int32Msg.testFieldInt = 10
            sint32Msg.testFieldInt = 100
            uint32Msg.testFieldInt = 100
            fixed32Msg.testFieldFixedInt32 = 1000
            sfixed32Msg.testFieldFixedInt32 = 1000
            int64Msg.testFieldInt = 100
            sint64Msg.testFieldInt = 100
            uint64Msg.testFieldInt = 100
            fixed64Msg.testFieldFixedInt64 = 1000
            sfixed64Msg.testFieldFixedInt64 = 1000

            int32Msg.testFieldInt = int32Msg.testFieldInt - int32Msg.testFieldInt
            sint32Msg.testFieldInt = sint32Msg.testFieldInt - sint32Msg.testFieldInt
            uint32Msg.testFieldInt = uint32Msg.testFieldInt - 10
            fixed32Msg.testFieldFixedInt32
                    = fixed32Msg.testFieldFixedInt32 - 100
            sfixed32Msg.testFieldFixedInt32
                    = sfixed32Msg.testFieldFixedInt32 - 100
            int64Msg.testFieldInt = int64Msg.testFieldInt - 10
            sint64Msg.testFieldInt = sint64Msg.testFieldInt - 10
            uint64Msg.testFieldInt = uint64Msg.testFieldInt - 10
            fixed64Msg.testFieldFixedInt64
                    = fixed64Msg.testFieldFixedInt64 - 100
            sfixed64Msg.testFieldFixedInt64
                    = sfixed64Msg.testFieldFixedInt64 - 100
        }

        function test_basicTypesMinus_data() {
            return [
                        {
                            tag: "int32 '-' operation",
                            field: int32Msg.testFieldInt,
                            answer: 0
                        },
                        {
                            tag: "sint32 '-' operation",
                            field: sint32Msg.testFieldInt,
                            answer: 0
                        },
                        {
                            tag: "uint32 '-' operation",
                            field: uint32Msg.testFieldInt,
                            answer: 90
                        },
                        {
                            tag: "fixedInt32 '-' operation",
                            field: fixed32Msg.testFieldFixedInt32,
                            answer: 900
                        },
                        {
                            tag: "sfixed32 '-' operation",
                            field: sfixed32Msg.testFieldFixedInt32,
                            answer: 900
                        },
                        {
                            tag: "int64 '-' operation",
                            field: int64Msg.testFieldInt,
                            answer: 90
                        },
                        {
                            tag: "sint64 '-' operation",
                            field: sint64Msg.testFieldInt,
                            answer: 90
                        },
                        {
                            tag: "uint64 '-' operation",
                            field: uint64Msg.testFieldInt,
                            answer: 90
                        },
                        {
                            tag: "fixedInt64 '-' operation",
                            field: fixed64Msg.testFieldFixedInt64,
                            answer: 900
                        },
                        {
                            tag: "sfixed64 '-' operation",
                            field: sfixed64Msg.testFieldFixedInt64,
                            answer: 900
                        }
                    ]
        }

        function test_basicTypesMinus(data) {
            verify(data.field == data.answer,
                   "Minus operation error: " + data.field + " != " + data.answer)
        }

        function test_basicTypesMinus_ImplicitJSValueConversion_data() {
            return [
                        {
                            tag: "int32 '-' operation",
                            field: int32Msg.testFieldInt - 10,
                            answer: -10
                        },
                        {
                            tag: "sint32 '-' operation",
                            field: sint32Msg.testFieldInt - 1,
                            answer: -1
                        },
                        {
                            tag: "uint32 '-' operation",
                            field: uint32Msg.testFieldInt - 10,
                            answer: 80
                        },
                        {
                            tag: "fixedInt32 '-' operation",
                            field: fixed32Msg.testFieldFixedInt32 - 100,
                            answer: 800
                        },
                        {
                            tag: "sfixed32 '-' operation",
                            field: sfixed32Msg.testFieldFixedInt32 - 100,
                            answer: 800
                        },
                        {
                            tag: "int64 '-' operation",
                            field: int64Msg.testFieldInt - 10,
                            answer: 80
                        },
                        {
                            tag: "sint64 '-' operation",
                            field: sint64Msg.testFieldInt - 10,
                            answer: 80
                        },
                        {
                            tag: "uint64 '-' operation",
                            field: uint64Msg.testFieldInt - 10,
                            answer: 80
                        },
                        {
                            tag: "fixedInt64 '-' operation",
                            field: fixed64Msg.testFieldFixedInt64 - 100,
                            answer: 800
                        },
                        {
                            tag: "sfixed64 '-' operation",
                            field: sfixed64Msg.testFieldFixedInt64 - 100,
                            answer: 800
                        }
                    ]
        }

        function test_basicTypesMinus_ImplicitJSValueConversion(data) {
            verify(data.field == data.answer,
                   "Minus operation error (JS): " + data.field + " != " + data.answer)
        }
    }

    TestCase {
        name: "basicTypesDivisionTest"

        property simpleIntMessage int32Msg
        property simpleSIntMessage sint32Msg
        property simpleUIntMessage uint32Msg
        property simpleFixedInt32Message fixed32Msg
        property simpleSFixedInt32Message sfixed32Msg
        property simpleStringMessage stringMsg
        property complexMessage complexMsg
        property simpleStringMessage outerMessage
        property simpleStringMessage innerMessage
        property complexMessage complexMsgNoInit
        property simpleInt64Message int64Msg
        property simpleSInt64Message sint64Msg
        property simpleUInt64Message uint64Msg
        property simpleFixedInt64Message fixed64Msg
        property simpleSFixedInt64Message sfixed64Msg

        function initTestCase() {
            int32Msg.testFieldInt = 10
            sint32Msg.testFieldInt = 100
            uint32Msg.testFieldInt = 100
            fixed32Msg.testFieldFixedInt32 = 1000
            sfixed32Msg.testFieldFixedInt32 = 1000
            int64Msg.testFieldInt = 1000
            sint64Msg.testFieldInt = 100
            uint64Msg.testFieldInt = 100
            fixed64Msg.testFieldFixedInt64 = 1000
            sfixed64Msg.testFieldFixedInt64 = 1000

            int32Msg.testFieldInt
                    = int32Msg.testFieldInt / int32Msg.testFieldInt
            sint32Msg.testFieldInt
                    = sint32Msg.testFieldInt / sint32Msg.testFieldInt
            uint32Msg.testFieldInt
                    = uint32Msg.testFieldInt / uint32Msg.testFieldInt
            fixed32Msg.testFieldFixedInt32
                    = fixed32Msg.testFieldFixedInt32 / fixed32Msg.testFieldFixedInt32
            sfixed32Msg.testFieldFixedInt32
                    = sfixed32Msg.testFieldFixedInt32 / sfixed32Msg.testFieldFixedInt32
            int64Msg.testFieldInt
                    = int64Msg.testFieldInt / int64Msg.testFieldInt
            sint64Msg.testFieldInt
                    = sint64Msg.testFieldInt / sint64Msg.testFieldInt
            uint64Msg.testFieldInt
                    = uint64Msg.testFieldInt / uint64Msg.testFieldInt
            fixed64Msg.testFieldFixedInt64
                    = fixed64Msg.testFieldFixedInt64 / fixed64Msg.testFieldFixedInt64
            sfixed64Msg.testFieldFixedInt64
                    = sfixed64Msg.testFieldFixedInt64 / sfixed64Msg.testFieldFixedInt64
        }

        function test_basicTypesDivision_data() {
            return [
                        {
                            tag: "int32 '/' operation",
                            field: int32Msg.testFieldInt,
                            answer: 1
                        },
                        {
                            tag: "sint32 '/' operation",
                            field: sint32Msg.testFieldInt,
                            answer: 1
                        },
                        {
                            tag: "uint32 '/' operation",
                            field: uint32Msg.testFieldInt,
                            answer: 1
                        },
                        {
                            tag: "fixedInt32 '/' operation",
                            field: fixed32Msg.testFieldFixedInt32,
                            answer: 1
                        },
                        {
                            tag: "sfixed32 '/' operation",
                            field: sfixed32Msg.testFieldFixedInt32,
                            answer: 1
                        },
                        {
                            tag: "int64 '/' operation",
                            field: int64Msg.testFieldInt,
                            answer: 1
                        },
                        {
                            tag: "sint64 '/' operation",
                            field: sint64Msg.testFieldInt,
                            answer: 1
                        },
                        {
                            tag: "uint64 '/' operation",
                            field: uint64Msg.testFieldInt,
                            answer: 1
                        },
                        {
                            tag: "fixedInt64 '/' operation",
                            field: fixed64Msg.testFieldFixedInt64,
                            answer: 1
                        },
                        {
                            tag: "sfixed64 '/' operation",
                            field: sfixed64Msg.testFieldFixedInt64,
                            answer: 1
                        }
                    ]
        }

        function test_basicTypesDivision(data) {
            verify(data.field == data.answer,
                   "Divide operation error: " + data.field + " != " + data.answer)
        }

        function test_basicTypesMinus_ImplicitJSValueConversion_data() {
            return [
                        {
                            tag: "int32 '/' operation",
                            field: int32Msg.testFieldInt / 1,
                            answer: 1
                        },
                        {
                            tag: "sint32 '/' operation",
                            field: sint32Msg.testFieldInt / 1,
                            answer: 1
                        },
                        {
                            tag: "uint32 '/' operation",
                            field: uint32Msg.testFieldInt / 1,
                            answer: 1
                        },
                        {
                            tag: "fixedInt32 '/' operation",
                            field: fixed32Msg.testFieldFixedInt32 / 1,
                            answer: 1
                        },
                        {
                            tag: "sfixed32 '/' operation",
                            field: sfixed32Msg.testFieldFixedInt32 / 1,
                            answer: 1
                        },
                        {
                            tag: "int64 '/' operation",
                            field: int64Msg.testFieldInt / 1,
                            answer: 1
                        },
                        {
                            tag: "sint64 '/' operation",
                            field: sint64Msg.testFieldInt / 1,
                            answer: 1
                        },
                        {
                            tag: "uint64 '/' operation",
                            field: uint64Msg.testFieldInt / 1,
                            answer: 1
                        },
                        {
                            tag: "fixedInt64 '/' operation",
                            field: fixed64Msg.testFieldFixedInt64 / 1,
                            answer: 1
                        },
                        {
                            tag: "sfixed64 '/' operation",
                            field: sfixed64Msg.testFieldFixedInt64 / 1,
                            answer: 1
                        }
                    ]
        }

        function test_basicTypesMinus_ImplicitJSValueConversion(data) {
            verify(data.field == data.answer,
                   "Divide operation error (JS): " + data.field + " != " + data.answer)
        }
    }

    TestCase {
        name: "basicTypesMultiTest"

        property simpleIntMessage int32Msg
        property simpleSIntMessage sint32Msg
        property simpleUIntMessage uint32Msg
        property simpleFixedInt32Message fixed32Msg
        property simpleSFixedInt32Message sfixed32Msg
        property simpleStringMessage stringMsg
        property complexMessage complexMsg
        property simpleStringMessage outerMessage
        property simpleStringMessage innerMessage
        property complexMessage complexMsgNoInit
        property simpleInt64Message int64Msg
        property simpleSInt64Message sint64Msg
        property simpleUInt64Message uint64Msg
        property simpleFixedInt64Message fixed64Msg
        property simpleSFixedInt64Message sfixed64Msg

        function initTestCase() {
            int32Msg.testFieldInt = 10
            sint32Msg.testFieldInt = 100
            uint32Msg.testFieldInt = 100
            fixed32Msg.testFieldFixedInt32 = 100
            sfixed32Msg.testFieldFixedInt32 = 100
            int64Msg.testFieldInt = 100
            sint64Msg.testFieldInt = 100
            uint64Msg.testFieldInt = 100
            fixed64Msg.testFieldFixedInt64 = 100
            sfixed64Msg.testFieldFixedInt64 = 100

            int32Msg.testFieldInt = int32Msg.testFieldInt * 10
            sint32Msg.testFieldInt = sint32Msg.testFieldInt * 10
            uint32Msg.testFieldInt = uint32Msg.testFieldInt * 10
            fixed32Msg.testFieldFixedInt32 = fixed32Msg.testFieldFixedInt32 * 10000
            sfixed32Msg.testFieldFixedInt32 = sfixed32Msg.testFieldFixedInt32 * 10000
            int64Msg.testFieldInt = int64Msg.testFieldInt * 10
            sint64Msg.testFieldInt = sint64Msg.testFieldInt * 10
            uint64Msg.testFieldInt = uint64Msg.testFieldInt * 10
            fixed64Msg.testFieldFixedInt64 = fixed64Msg.testFieldFixedInt64 * 10000
            sfixed64Msg.testFieldFixedInt64 = sfixed64Msg.testFieldFixedInt64 * 10000
        }

        function test_basicTypesMulti_data() {
            return [
                        {
                            tag: "int32 '*' operation",
                            field: int32Msg.testFieldInt,
                            answer: 100
                        },
                        {
                            tag: "sint32 '*' operation",
                            field: sint32Msg.testFieldInt,
                            answer: 1000
                        },
                        {
                            tag: "uint32 '*' operation",
                            field: uint32Msg.testFieldInt,
                            answer: 1000
                        },
                        {
                            tag: "fixedInt32 '*' operation",
                            field: fixed32Msg.testFieldFixedInt32,
                            answer: 1000000
                        },
                        {
                            tag: "sfixed32 '*' operation",
                            field: sfixed32Msg.testFieldFixedInt32,
                            answer: 1000000
                        },
                        {
                            tag: "int64 '*' operation",
                            field: int64Msg.testFieldInt,
                            answer: 1000
                        },
                        {
                            tag: "sint64 '*' operation",
                            field: sint64Msg.testFieldInt,
                            answer: 1000
                        },
                        {
                            tag: "uint64 '*' operation",
                            field: uint64Msg.testFieldInt,
                            answer: 1000
                        },
                        {
                            tag: "fixedInt64 '*' operation",
                            field: fixed64Msg.testFieldFixedInt64,
                            answer: 1000000
                        },
                        {
                            tag: "sfixed64 '*' operation",
                            field: sfixed64Msg.testFieldFixedInt64,
                            answer: 1000000
                        }
                    ]
        }

        function test_basicTypesMulti(data) {
            verify(data.field == data.answer,
                   "Multiplication operation error: " + data.field + " != " + data.answer)
        }

        function test_basicTypesMulti_ImplicitJSValueConversion_data() {
            return [
                        {
                            tag: "int32 '*' operation",
                            field: int32Msg.testFieldInt * 1,
                            answer: 100
                        },
                        {
                            tag: "sint32 '*' operation",
                            field: sint32Msg.testFieldInt * (-1),
                            answer: -1000
                        },
                        {
                            tag: "uint32 '*' operation",
                            field: uint32Msg.testFieldInt * 1,
                            answer: 1000
                        },
                        {
                            tag: "fixedInt32 '*' operation",
                            field: fixed32Msg.testFieldFixedInt32 * 1,
                            answer: 1000000
                        },
                        {
                            tag: "sfixed32 '*' operation",
                            field: sfixed32Msg.testFieldFixedInt32 * 1,
                            answer: 1000000
                        },
                        {
                            tag: "int64 '*' operation",
                            field: int64Msg.testFieldInt * 1,
                            answer: 1000
                        },
                        {
                            tag: "sint64 '*' operation",
                            field: sint64Msg.testFieldInt * (-1),
                            answer: -1000
                        },
                        {
                            tag: "uint64 '*' operation",
                            field: uint64Msg.testFieldInt * 1,
                            answer: 1000
                        },
                        {
                            tag: "fixedInt64 '*' operation",
                            field: fixed64Msg.testFieldFixedInt64 * 1,
                            answer: 1000000
                        },
                        {
                            tag: "sfixed64 '*' operation",
                            field: sfixed64Msg.testFieldFixedInt64 * (-1),
                            answer: -1000000
                        }
                    ]
        }

        function test_basicTypesMulti_ImplicitJSValueConversion(data) {
            verify(data.field == data.answer,
                   "Multiplication operation error (JS): "
                   + data.field + " != " + data.answer)
        }
    }

    TestCase {
        name: "basicTypesIncTest"

        property simpleIntMessage int32Msg
        property simpleSIntMessage sint32Msg
        property simpleUIntMessage uint32Msg
        property simpleFixedInt32Message fixed32Msg
        property simpleSFixedInt32Message sfixed32Msg
        property simpleStringMessage stringMsg
        property complexMessage complexMsg
        property simpleStringMessage outerMessage
        property simpleStringMessage innerMessage
        property complexMessage complexMsgNoInit
        property simpleInt64Message int64Msg
        property simpleSInt64Message sint64Msg
        property simpleUInt64Message uint64Msg
        property simpleFixedInt64Message fixed64Msg
        property simpleSFixedInt64Message sfixed64Msg

        function initTestCase() {
            int32Msg.testFieldInt = 10
            sint32Msg.testFieldInt = 100
            uint32Msg.testFieldInt = 100
            fixed32Msg.testFieldFixedInt32 = 1000
            sfixed32Msg.testFieldFixedInt32 = 1000
            int64Msg.testFieldInt = 1000
            sint64Msg.testFieldInt = 100
            uint64Msg.testFieldInt = 100
            fixed64Msg.testFieldFixedInt64 = 1000
            sfixed64Msg.testFieldFixedInt64 = 1000

            int32Msg.testFieldInt++
            sint32Msg.testFieldInt++
            uint32Msg.testFieldInt++
            fixed32Msg.testFieldFixedInt32++
            sfixed32Msg.testFieldFixedInt32++
            int64Msg.testFieldInt++
            sint64Msg.testFieldInt++
            uint64Msg.testFieldInt++
            fixed64Msg.testFieldFixedInt64++
            sfixed64Msg.testFieldFixedInt64++
        }

        function test_basicTypesInc_data() {
            return [
                        {
                            tag: "int32 '++' operation",
                            field: int32Msg.testFieldInt,
                            answer: 11
                        },
                        {
                            tag: "sint32 '++' operation",
                            field: sint32Msg.testFieldInt,
                            answer: 101
                        },
                        {
                            tag: "uint32 '++' operation",
                            field: uint32Msg.testFieldInt,
                            answer: 101
                        },
                        {
                            tag: "fixedInt32 '++' operation",
                            field: fixed32Msg.testFieldFixedInt32,
                            answer: 1001
                        },
                        {
                            tag: "sfixed32 '++' operation",
                            field: sfixed32Msg.testFieldFixedInt32,
                            answer: 1001
                        },
                        {
                            tag: "int64 '++' operation",
                            field: int64Msg.testFieldInt,
                            answer: 1001
                        },
                        {
                            tag: "sint64 '++' operation",
                            field: sint64Msg.testFieldInt,
                            answer: 101
                        },
                        {
                            tag: "uint64 '++' operation",
                            field: uint64Msg.testFieldInt,
                            answer: 101
                        },
                        {
                            tag: "fixedInt64 '++' operation",
                            field: fixed64Msg.testFieldFixedInt64,
                            answer: 1001
                        },
                        {
                            tag: "sfixed64 '++' operation",
                            field: sfixed64Msg.testFieldFixedInt64,
                            answer: 1001
                        }
                    ]
        }

        function test_basicTypesInc(data) {
            verify(data.field == data.answer, "Incrementation error: "
                   + data.field + " != " + data.answer)
        }
    }

    TestCase {
        name: "basicTypesIncTest_JS"

        property simpleIntMessage int32Msg
        property simpleSIntMessage sint32Msg
        property simpleUIntMessage uint32Msg
        property simpleFixedInt32Message fixed32Msg
        property simpleSFixedInt32Message sfixed32Msg
        property simpleStringMessage stringMsg
        property complexMessage complexMsg
        property simpleStringMessage outerMessage
        property simpleStringMessage innerMessage
        property complexMessage complexMsgNoInit
        property simpleInt64Message int64Msg
        property simpleSInt64Message sint64Msg
        property simpleUInt64Message uint64Msg
        property simpleFixedInt64Message fixed64Msg
        property simpleSFixedInt64Message sfixed64Msg

        function initTestCase() {
            int32Msg.testFieldInt = 10
            sint32Msg.testFieldInt = 100
            uint32Msg.testFieldInt = 100
            fixed32Msg.testFieldFixedInt32 = 1000
            sfixed32Msg.testFieldFixedInt32 = 1000
            int64Msg.testFieldInt = 1000
            sint64Msg.testFieldInt = 100
            uint64Msg.testFieldInt = 100
            fixed64Msg.testFieldFixedInt64 = 1000
            sfixed64Msg.testFieldFixedInt64 = 1000
        }


        function test_basicTypesInc_ImplicitJSValueConversion_data() {
            return [
                        {
                            tag: "int32 '++' operation",
                            field: ++int32Msg.testFieldInt,
                            answer: 11
                        },
                        {
                            tag: "sint32 '++' operation",
                            field: ++sint32Msg.testFieldInt,
                            answer: 101
                        },
                        {
                            tag: "uint32 '++' operation",
                            field: ++uint32Msg.testFieldInt,
                            answer: 101
                        },
                        {
                            tag: "fixedInt32 '++' operation",
                            field: ++fixed32Msg.testFieldFixedInt32,
                            answer: 1001
                        },
                        {
                            tag: "sfixed32 '++' operation",
                            field: ++sfixed32Msg.testFieldFixedInt32,
                            answer: 1001
                        },
                        {
                            tag: "int64 '++' operation",
                            field: ++int64Msg.testFieldInt,
                            answer: 1001
                        },
                        {
                            tag: "sint64 '++' operation",
                            field: ++sint64Msg.testFieldInt,
                            answer: 101
                        },
                        {
                            tag: "uint64 '++' operation",
                            field: ++uint64Msg.testFieldInt,
                            answer: 101
                        },
                        {
                            tag: "fixedInt64 '++' operation",
                            field: ++fixed64Msg.testFieldFixedInt64,
                            answer: 1001
                        },
                        {
                            tag: "sfixed64 '++' operation",
                            field: ++sfixed64Msg.testFieldFixedInt64,
                            answer: 1001
                        }
                    ]
        }

        function test_basicTypesInc_ImplicitJSValueConversion(data) {
                verify(data.field == data.answer, "Incrementation error: "
                       + data.field + " != " + data.answer)
        }
    }

    TestCase {
        name: "basicTypesDecTest"

        property simpleIntMessage int32Msg
        property simpleSIntMessage sint32Msg
        property simpleUIntMessage uint32Msg
        property simpleFixedInt32Message fixed32Msg
        property simpleSFixedInt32Message sfixed32Msg
        property simpleStringMessage stringMsg
        property complexMessage complexMsg
        property simpleStringMessage outerMessage
        property simpleStringMessage innerMessage
        property complexMessage complexMsgNoInit
        property simpleInt64Message int64Msg
        property simpleSInt64Message sint64Msg
        property simpleUInt64Message uint64Msg
        property simpleFixedInt64Message fixed64Msg
        property simpleSFixedInt64Message sfixed64Msg

        function initTestCase() {
            int32Msg.testFieldInt = 10
            sint32Msg.testFieldInt = 10
            uint32Msg.testFieldInt = 10
            fixed32Msg.testFieldFixedInt32 = 1000
            sfixed32Msg.testFieldFixedInt32 = 1000
            int64Msg.testFieldInt = 10
            sint64Msg.testFieldInt = 10
            uint64Msg.testFieldInt = 10
            fixed64Msg.testFieldFixedInt64 = 1000
            sfixed64Msg.testFieldFixedInt64 = 1000

            int32Msg.testFieldInt--
            sint32Msg.testFieldInt--
            uint32Msg.testFieldInt--
            fixed32Msg.testFieldFixedInt32--
            sfixed32Msg.testFieldFixedInt32--
            int64Msg.testFieldInt--
            sint64Msg.testFieldInt--
            uint64Msg.testFieldInt--
            fixed64Msg.testFieldFixedInt64--
            sfixed64Msg.testFieldFixedInt64--
        }

        function test_basicTypesDec_data() {
            return [
                        {
                            tag: "int32 '--' operation",
                            field: int32Msg.testFieldInt,
                            answer: 9
                        },
                        {
                            tag: "sint32 '--' operation",
                            field: sint32Msg.testFieldInt,
                            answer: 9
                        },
                        {
                            tag: "uint32 '--' operation",
                            field: uint32Msg.testFieldInt,
                            answer: 9
                        },
                        {
                            tag: "fixedInt32 '--' operation",
                            field: fixed32Msg.testFieldFixedInt32,
                            answer: 999
                        },
                        {
                            tag: "sfixed32 '--' operation",
                            field: sfixed32Msg.testFieldFixedInt32,
                            answer: 999
                        },
                        {
                            tag: "int64 '--' operation",
                            field: int64Msg.testFieldInt,
                            answer: 9
                        },
                        {
                            tag: "sint64 '--' operation",
                            field: sint64Msg.testFieldInt,
                            answer: 9
                        },
                        {
                            tag: "uint64 '--' operation",
                            field: uint64Msg.testFieldInt,
                            answer: 9
                        },
                        {
                            tag: "fixedInt64 '--' operation",
                            field: fixed64Msg.testFieldFixedInt64,
                            answer: 999
                        },
                        {
                            tag: "sfixed64 '--' operation",
                            field: sfixed64Msg.testFieldFixedInt64,
                            answer: 999
                        }
                    ]
        }

        function test_basicTypesDec(data) {
            verify(data.field == data.answer, "Decrementation error: "
                   + data.field + " != " + data.answer)
        }
    }

    TestCase {
        name: "basicTypesDecTest_JS"

        property simpleIntMessage int32Msg
        property simpleSIntMessage sint32Msg
        property simpleUIntMessage uint32Msg
        property simpleFixedInt32Message fixed32Msg
        property simpleSFixedInt32Message sfixed32Msg
        property simpleStringMessage stringMsg
        property complexMessage complexMsg
        property simpleStringMessage outerMessage
        property simpleStringMessage innerMessage
        property complexMessage complexMsgNoInit
        property simpleInt64Message int64Msg
        property simpleSInt64Message sint64Msg
        property simpleUInt64Message uint64Msg
        property simpleFixedInt64Message fixed64Msg
        property simpleSFixedInt64Message sfixed64Msg

        function initTestCase() {
            int32Msg.testFieldInt = 10
            sint32Msg.testFieldInt = 10
            uint32Msg.testFieldInt = 10
            fixed32Msg.testFieldFixedInt32 = 1000
            sfixed32Msg.testFieldFixedInt32 = 1000
            int64Msg.testFieldInt = 10
            sint64Msg.testFieldInt = 10
            uint64Msg.testFieldInt = 10
            fixed64Msg.testFieldFixedInt64 = 1000
            sfixed64Msg.testFieldFixedInt64 = 1000
        }

        function test_basicTypesDec_ImplicitJSValueConversion_data() {
            return [
                        {
                            tag: "int32 '--' operation",
                            field: --int32Msg.testFieldInt,
                            answer: 9
                        },
                        {
                            tag: "sint32 '--' operation",
                            field: --sint32Msg.testFieldInt,
                            answer: 9
                        },
                        {
                            tag: "uint32 '--' operation",
                            field: --uint32Msg.testFieldInt,
                            answer: 9
                        },
                        {
                            tag: "fixedInt32 '--' operation",
                            field: --fixed32Msg.testFieldFixedInt32,
                            answer: 999
                        },
                        {
                            tag: "sfixed32 '--' operation",
                            field: --sfixed32Msg.testFieldFixedInt32,
                            answer: 999
                        },
                        {
                            tag: "int64 '--' operation",
                            field: --int64Msg.testFieldInt,
                            answer: 9
                        },
                        {
                            tag: "sint64 '--' operation",
                            field: --sint64Msg.testFieldInt,
                            answer: 9
                        },
                        {
                            tag: "uint64 '--' operation",
                            field: --uint64Msg.testFieldInt,
                            answer: 9
                        },
                        {
                            tag: "fixedInt64 '--' operation",
                            field: --fixed64Msg.testFieldFixedInt64,
                            answer: 999
                        },
                        {
                            tag: "sfixed64 '--' operation",
                            field: --sfixed64Msg.testFieldFixedInt64,
                            answer: 999
                        }
                    ]
        }

        function test_basicTypesDec_ImplicitJSValueConversion(data) {
            verify(data.field == data.answer, "Decrementation error: "
                   + data.field + " != " + data.answer)
        }
    }

    TestCase {
        name: "simpleBoolTest"
        property simpleBoolMessage boolMsg

        function initTestCase() {
            boolMsg.testFieldBool = true
        }

        function test_simpleboolmessage_data() {
            return [
                        {tag: "SimpleBoolMessage == true",
                            field: boolMsg.testFieldBool, answer: true },
                        {tag: "SimpleBoolMessage == false",
                            field: !boolMsg.testFieldBool, answer: false },
                    ]
        }

        function test_simpleboolmessage(data) {
            compare(data.field, data.answer)
        }
    }
}
