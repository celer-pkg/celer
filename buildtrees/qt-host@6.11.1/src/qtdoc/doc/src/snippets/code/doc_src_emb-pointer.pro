// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#! [7]
....
QMAKE_CFLAGS += -I<path to tslib headers>
QMAKE_LFLAGS += -L<path to tslib library> -Wl,-rpath-link=<path to tslib library>
....
#! [7]
