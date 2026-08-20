// Copyright (C) 2015 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QLOCATION_H
#define QLOCATION_H

#if 0
#pragma qt_class(QLocation)
#endif

#include <QtLocation/qlocationglobal.h>

QT_BEGIN_NAMESPACE

namespace QLocation {

enum Visibility {
    UnspecifiedVisibility = 0x00,
    DeviceVisibility = 0x01,
    PrivateVisibility = 0x02,
    PublicVisibility = 0x04
};

Q_DECLARE_FLAGS(VisibilityScope, Visibility)

}

Q_DECLARE_OPERATORS_FOR_FLAGS(QLocation::VisibilityScope)

QT_END_NAMESPACE

#endif // QLOCATION_H
