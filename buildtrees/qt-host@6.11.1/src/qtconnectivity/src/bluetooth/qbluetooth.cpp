// Copyright (C) 2016 The Qt Company Ltd.
// Copyright (C) 2014 Denis Shienkov <denis.shienkov@gmail.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include <QtCore/QLoggingCategory>
#include <QtBluetooth/qbluetooth.h>

QT_BEGIN_NAMESPACE

namespace QBluetooth {
/*!
    \namespace QBluetooth
    \inmodule QtBluetooth
    \brief The QBluetooth namespace provides classes and functions related to
    Bluetooth.

    \since 5.2
*/

/*!
    \enum QBluetooth::Security

    This enum describe the security requirements of a Bluetooth service.

    \value NoSecurity    The service does not require any security.

    \value Authorization The service requires authorization by the user,
    unless the device is Authorized-Paired.

    \value Authentication The service requires authentication. Device must
    be paired, and the user is prompted on connection unless the device is
    Authorized-Paired.

    \value Encryption The service requires the communication link to be
    encrypted. This requires the device to be paired.

    \value Secure The service requires the communication link to be secure.
    Simple Pairing from Bluetooth 2.1 or greater is required.
    Legacy pairing is not permitted.
*/

/*!
    \enum QBluetooth::AttAccessConstraint
    \since 5.7

    This enum describes the possible requirements for reading or writing an ATT attribute.

    \value AttAuthorizationRequired
        The client needs authorization from the ATT server to access the attribute.
    \value AttAuthenticationRequired
        The client needs to be authenticated to access the attribute.
    \value AttEncryptionRequired
        The attribute can only be accessed if the connection is encrypted.
*/

}

/*!
    \typedef QLowEnergyHandle
    \relates QBluetooth
    \since 5.4

    Typedef for Bluetooth Low Energy ATT attribute handles.
*/

Q_LOGGING_CATEGORY(QT_BT, "qt.bluetooth")
Q_LOGGING_CATEGORY(QT_BT_ANDROID, "qt.bluetooth.android")
Q_LOGGING_CATEGORY(QT_BT_BLUEZ, "qt.bluetooth.bluez")
Q_LOGGING_CATEGORY(QT_BT_WINDOWS, "qt.bluetooth.windows")
Q_LOGGING_CATEGORY(QT_BT_WINDOWS_SERVICE_THREAD, "qt.bluetooth.winrt.service.thread")

QT_END_NAMESPACE

#include "moc_qbluetooth.cpp"
