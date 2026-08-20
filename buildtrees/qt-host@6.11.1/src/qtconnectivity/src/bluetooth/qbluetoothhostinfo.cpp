// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qbluetoothhostinfo.h"
#include "qbluetoothhostinfo_p.h"

QT_BEGIN_NAMESPACE

QT_IMPL_METATYPE_EXTERN(QBluetoothHostInfo)

/*!
    \class QBluetoothHostInfo
    \inmodule QtBluetooth
    \brief The QBluetoothHostInfo class encapsulates the details of a local
    QBluetooth device.

    \since 5.2

    This class holds the name and address of a local Bluetooth device.
*/

/*!
    Constructs a null QBluetoothHostInfo object.
*/
QBluetoothHostInfo::QBluetoothHostInfo() :
    d_ptr(new QBluetoothHostInfoPrivate)
{
}

/*!
    Constructs a new QBluetoothHostInfo which is a copy of \a other.
*/
QBluetoothHostInfo::QBluetoothHostInfo(const QBluetoothHostInfo &other) :
    d_ptr(new QBluetoothHostInfoPrivate)
{
    Q_D(QBluetoothHostInfo);

    d->m_address = other.d_func()->m_address;
    d->m_name = other.d_func()->m_name;
}

/*!
    Destroys the QBluetoothHostInfo.
*/
QBluetoothHostInfo::~QBluetoothHostInfo()
{
    delete d_ptr;
}

/*!
    Assigns \a other to this QBluetoothHostInfo instance.
*/
QBluetoothHostInfo &QBluetoothHostInfo::operator=(const QBluetoothHostInfo &other)
{
    Q_D(QBluetoothHostInfo);

    d->m_address = other.d_func()->m_address;
    d->m_name = other.d_func()->m_name;

    return *this;
}

/*!
    \fn bool QBluetoothHostInfo::operator==(const QBluetoothHostInfo &a,
                                            const QBluetoothHostInfo &b)
    \brief Returns \c true if \a a and \a b are equal, otherwise \c false.
*/

/*!
    \fn bool QBluetoothHostInfo::operator!=(const QBluetoothHostInfo &a,
                                            const QBluetoothHostInfo &b)
    \brief Returns \c true if \a a and \a b are not equal, otherwise \c false.
*/

/*!
    \brief Returns \c true if \a a and \a b are equal, otherwise \c false.
    \internal
*/
bool QBluetoothHostInfo::equals(const QBluetoothHostInfo &a, const QBluetoothHostInfo &b)
{
    if (a.d_ptr == b.d_ptr)
        return true;

    return a.d_ptr->m_address == b.d_ptr->m_address && a.d_ptr->m_name == b.d_ptr->m_name;
}

/*!
    Returns the Bluetooth address as a QBluetoothAddress.
*/
QBluetoothAddress QBluetoothHostInfo::address() const
{
    Q_D(const QBluetoothHostInfo);
    return d->m_address;
}

/*!
    Sets the Bluetooth \a address for this Bluetooth host info object.
*/
void QBluetoothHostInfo::setAddress(const QBluetoothAddress &address)
{
    Q_D(QBluetoothHostInfo);
    d->m_address = address;
}

/*!
    Returns the user visible name of the host info object.
*/
QString QBluetoothHostInfo::name() const
{
    Q_D(const QBluetoothHostInfo);
    return d->m_name;
}

/*!
    Sets the \a name of the host info object.
*/
void QBluetoothHostInfo::setName(const QString &name)
{
    Q_D(QBluetoothHostInfo);
    d->m_name = name;
}

QT_END_NAMESPACE
