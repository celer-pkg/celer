// Copyright (C) 2015 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#! [0]
TEMPLATE = app
QT  += axserver

RC_FILE  = qaxserver.rc
...
#! [0]


#! [1]
TEMPLATE = lib
QT += axserver
CONFIG  += dll

DEF_FILE = qaxserver.def
RC_FILE  = qaxserver.rc
...
#! [1]


#! [2]
TEMPLATE = lib
VERSION = 2.5
...
#! [2]
