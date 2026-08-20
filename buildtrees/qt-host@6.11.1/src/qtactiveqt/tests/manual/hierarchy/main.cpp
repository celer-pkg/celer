// Copyright (C) 2015 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

//! [0]
#include "objects.h"
#include <QAxFactory>

QAXFACTORY_BEGIN("{9e626211-be62-4d18-9483-9419358fbb03}", "{75c276de-1df5-451f-a004-e4fa1a587df1}")
    QAXCLASS(QParentWidget)
    QAXTYPE(QSubWidget)
QAXFACTORY_END()
//! [0]
