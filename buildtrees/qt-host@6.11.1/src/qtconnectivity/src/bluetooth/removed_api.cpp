// Copyright (C) 2022 Intel Corporation.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#define QT_BLUETOOTH_BUILD_REMOVED_API

// Undefine Qt 128-bit int types
#ifndef QT_NO_INT128
#  define QT_NO_INT128
#endif

#include "qtbluetoothglobal.h"

QT_USE_NAMESPACE

#if QT_BLUETOOTH_REMOVED_SINCE(6, 6)

#include "qbluetoothaddress.h" // inlined API

#include "qbluetoothuuid.h"

static_assert(std::is_aggregate_v<quint128>);
static_assert(std::is_trivial_v<quint128>);

// These legacy functions can't just call the new (quint128, Endian) overloads,
// as the latter may see either QtCore's quint128 (__int128 built-in) _or_ our
// fake version from qbluetoothuuid.h. This TU must _always_ see the fake ones
// (as that is what we had in 6.5):
#ifdef QT_SUPPORTS_INT128
#  error This TU requires QT_NO_INT128 to be defined.
#endif

QBluetoothUuid::QBluetoothUuid(quint128 uuid)
    : QUuid(fromBytes(uuid.data))
{}

quint128 QBluetoothUuid::toUInt128() const
{
    quint128 uuid;
    const auto bytes = toBytes();
    memcpy(uuid.data, bytes.data, sizeof(uuid.data));
    return uuid;
}

// END quint128 functions

QBluetoothUuid::QBluetoothUuid(const QString &uuid)
    : QUuid(qToAnyStringViewIgnoringNull(uuid))
{
}

#ifndef QT_NO_DEBUG_STREAM
QDebug QBluetoothUuid::streamingOperator(QDebug debug, const QBluetoothUuid &uuid)
{
    debug << uuid.toString();
    return debug;
}
#endif // QT_NO_DEBUG_STREAM

#endif // QT_BLUETOOTH_REMOVED_SINCE(6, 6)
