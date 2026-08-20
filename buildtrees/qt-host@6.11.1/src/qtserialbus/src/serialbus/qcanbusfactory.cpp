// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qcanbusfactory.h"

QT_BEGIN_NAMESPACE

/*!
    \class QCanBusFactory
    \inmodule QtSerialBus
    \since 5.9

    \brief The QCanBusFactory class is a factory class used as the
    plugin interface for CAN bus plugins.

    All plugins must implement the functions provided by this factory class.
*/

/*!
    \fn QCanBusDevice *QCanBusFactory::createDevice(const QString &interfaceName,
                                                    QString *errorMessage) const

    Creates a new QCanBusDevice. The caller must take ownership of the returned pointer.

    \a interfaceName is the CAN interface name and
    \a errorMessage contains an error description in case of failure.

    If the factory cannot create a plugin, it returns \c nullptr.
*/

/*!
    \fn QList<QCanBusDeviceInfo> QCanBusFactory::availableDevices(QString *errorMessage) const

    Returns the list of available devices and their capabilities for the QCanBusDevice.

    \a errorMessage contains an error description in case of failure.
*/

/*!
 * \internal
 */
QCanBusFactory::~QCanBusFactory()
{
}

QT_END_NAMESPACE
