// Copyright (C) 2018 Denis Shienkov <denis.shienkov@gmail.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "geocluetypes.h"

QT_BEGIN_NAMESPACE

QT_IMPL_METATYPE_EXTERN(Timestamp)

QDBusArgument &operator<<(QDBusArgument &arg, const Timestamp &ts)
{
    arg.beginStructure();
    arg << ts.m_seconds;
    arg << ts.m_microseconds;
    arg.endStructure();
    return arg;
}

const QDBusArgument &operator>>(const QDBusArgument &arg, Timestamp &ts)
{
    arg.beginStructure();
    arg >> ts.m_seconds;
    arg >> ts.m_microseconds;
    arg.endStructure();
    return arg;
}

QT_END_NAMESPACE
