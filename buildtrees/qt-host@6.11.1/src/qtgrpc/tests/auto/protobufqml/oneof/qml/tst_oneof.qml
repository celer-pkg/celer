// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtTest

import QmlTestUri

TestCase {
    name: "qtprotobufBasicTest"
    property oneofSimpleMessage simpleOneofMsg;
    property oneofComplexMessage complesOneofMsg;
    function test_simpleOneofFields() {
        verify(!simpleOneofMsg.hasTestOneofFieldInt)
        verify(!simpleOneofMsg.hasTestOneofFieldSecondInt)
        verify(simpleOneofMsg.testOneofFieldInt == 0)
        verify(simpleOneofMsg.testOneofFieldSecondInt == 0)

        simpleOneofMsg.testOneofFieldInt = 10024242;
        verify(simpleOneofMsg.hasTestOneofFieldInt)
        verify(!simpleOneofMsg.hasTestOneofFieldSecondInt)
        verify(simpleOneofMsg.testOneofFieldInt == 10024242)
        verify(simpleOneofMsg.testOneofFieldSecondInt == 0)

        simpleOneofMsg.testOneofFieldSecondInt = 42421002;
        verify(simpleOneofMsg.hasTestOneofFieldSecondInt)
        verify(!simpleOneofMsg.hasTestOneofFieldInt)
        verify(simpleOneofMsg.testOneofFieldSecondInt == 42421002)
        verify(simpleOneofMsg.testOneofFieldInt == 0)

        simpleOneofMsg.clearTestOneof()
        verify(!simpleOneofMsg.hasTestOneofFieldSecondInt)
        verify(!simpleOneofMsg.hasTestOneofFieldInt)

        verify(simpleOneofMsg.testOneofFieldInt == 0)
        verify(simpleOneofMsg.testOneofFieldSecondInt == 0)
    }
}
