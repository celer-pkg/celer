// Copyright (C) 2018 Denis Shienkov <denis.shienkov@gmail.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef GEOCLUETYPES_H
#define GEOCLUETYPES_H

#include <QtDBus/QDBusArgument>

class Timestamp
{
public:
    quint64 m_seconds = 0;
    quint64 m_microseconds = 0;
};

QT_BEGIN_NAMESPACE

Q_DECLARE_TYPEINFO(Timestamp, Q_RELOCATABLE_TYPE);

QDBusArgument &operator<<(QDBusArgument &arg, const Timestamp &ts);
const QDBusArgument &operator>>(const QDBusArgument &arg, Timestamp &ts);

QT_END_NAMESPACE

QT_DECL_METATYPE_EXTERN(Timestamp, /* not exported */)

#endif // GEOCLUETYPES_H
