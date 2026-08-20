// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

pragma Strict

import QtQml
import TestTypes as T

T.UsingUserObject {
    id: self
    property int valA: val.a
    property int valB: val.getB()
    property int myA: a
    property int myB: getB()
    property int myA2: self.a
    property int myB2: self.getB()

    property var valU: val.u
    property var myU: u
    property var myU2: self.u

    property var huge: 4294967295
    property var impossible: "impossible"

    function twiddle() {
        val.a = 55
        val.setB(56)
        a = 57
        setB(58)
        self.a = 59
        self.setB(60)

        val.u = 61
        u = 62
        self.u = 63
    }

    function burn() {
        val.u = huge
        u = huge
        u = 64
        self.u = huge
    }

    function impossibleValA() {
        val.a = impossible
    }

    function impossibleA() {
        a = impossible
    }

    function impossibleSelfA() {
        self.a = impossible
    }

    function impossibleValU() {
        val.u = impossible
    }

    function impossibleU() {
        u = impossible
    }

    function impossibleSelfU() {
        self.u = impossible
    }
}
