// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#! [3]
CONFIG += release
#! [3]

#! [4]
QTPLUGIN.platforms = qminimal
#! [4]

#! [5]
QTPLUGIN += qlibinputplugin
#! [5]

#! [6]
QTPLUGIN.platforms = -
#! [6]

#! [7]
CONFIG -= import_plugins
#! [7]
