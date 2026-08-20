// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QLOCATIONGLOBAL_P_H
#define QLOCATIONGLOBAL_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "qlocationglobal.h"
#include <qqml.h>
#include "private/qglobal_p.h"
#include <QtLocation/qtlocationexports.h>

QT_BEGIN_NAMESPACE

namespace QLocation
{
Q_NAMESPACE_EXPORT(Q_LOCATION_EXPORT)
QML_NAMED_ELEMENT(QtLocation)
QML_ADDED_IN_VERSION(6, 6)
Q_CLASSINFO("RegisterEnumClassesUnscoped", "false")

enum class ReferenceSurface : uint8_t {
    Map,
    Globe
};
Q_ENUM_NS(ReferenceSurface)

}

QT_END_NAMESPACE

#endif // QLOCATIONGLOBAL_P_H
