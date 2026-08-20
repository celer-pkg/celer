// Copyright (C) 2015 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

//! [0]
#include "ax1.h"
#include "ax2.h"
#include <QAxFactory>

QT_USE_NAMESPACE

QAXFACTORY_BEGIN("{98DE28B6-6CD3-4e08-B9FA-3D1DB43F1D2F}", "{05828915-AD1C-47ab-AB96-D6AD1E25F0E2}")
    QAXCLASS(QAxWidget1)
    QAXCLASS(QAxWidget2)
QAXFACTORY_END()
//! [0]
