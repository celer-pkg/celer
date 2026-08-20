// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QBLUETOOTH_H
#define QBLUETOOTH_H

#include <QtBluetooth/qtbluetoothglobal.h>
#include <QtCore/qglobal.h>
#include <QtCore/qtmetamacros.h>
#include <QtCore/qobject.h>

QT_BEGIN_NAMESPACE

namespace QBluetooth {
Q_NAMESPACE_EXPORT(Q_BLUETOOTH_EXPORT)
// TODO Qt 6: Merge these two enums? But note that ATT Authorization has no equivalent
//            on the socket security level.

enum class Security {
    NoSecurity = 0x00,
    Authorization = 0x01,
    Authentication = 0x02,
    Encryption = 0x04,
    Secure = 0x08
};
Q_ENUM_NS(Security)
Q_DECLARE_FLAGS(SecurityFlags, Security)
Q_DECLARE_OPERATORS_FOR_FLAGS(SecurityFlags)

enum class AttAccessConstraint {
    AttAuthorizationRequired = 0x1,
    AttAuthenticationRequired = 0x2,
    AttEncryptionRequired = 0x4,
};
Q_ENUM_NS(AttAccessConstraint)

Q_DECLARE_FLAGS(AttAccessConstraints, AttAccessConstraint)
Q_DECLARE_OPERATORS_FOR_FLAGS(AttAccessConstraints)

}

typedef quint16 QLowEnergyHandle;

QT_END_NAMESPACE

#endif // QBLUETOOTH_H
