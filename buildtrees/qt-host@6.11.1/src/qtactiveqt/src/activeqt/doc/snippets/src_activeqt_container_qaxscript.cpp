// Copyright (C) 2015 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

//! [0]
function setNumber(number)
{
    n = number;
}
//! [0]


//! [1]
QValueList args;
args << 5;
script->call("setNumber(const QVariant&)", args);
//! [1]


//! [2]
script->call("setNumber(5)");
//! [2]
