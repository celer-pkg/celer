// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "darwin/btutility_p.h"

#include "qbluetoothuuid.h"

QT_BEGIN_NAMESPACE

/*!
    \brief Constructs a new QBluetoothUuid, containing a copy of the \a cbUuid CBUUID.

    \note this function is only available on Apple platforms.

    \since 6.6
    \ingroup platform-type-conversions
*/
QBluetoothUuid QBluetoothUuid::fromCBUUID(CBUUID *cbUuid)
{
    if (!cbUuid)
        return {};

    return DarwinBluetooth::qt_uuid(cbUuid);
}

/*!
    \brief Creates a CBUUID from a QBluetoothUuid.

    The resulting CBUUID is autoreleased.

    \note this function is only available on Apple platforms.

    \since 6.6
    \ingroup platform-type-conversions
*/

CBUUID *QBluetoothUuid::toCBUUID() const
{
    const auto cbUuidGuard = DarwinBluetooth::cb_uuid(*this);
    // cb_uuid returns a strong reference (RAII object). Let
    // it do its job and release, but we return auto-released
    // CBUUID, as documented.
    return [[cbUuidGuard.data() retain] autorelease];
}

QT_END_NAMESPACE
