// Copyright (C) 2017 Andre Hartmann <aha_1980@gmx.de>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qcanbusdeviceinfo.h"
#include "qcanbusdeviceinfo_p.h"

QT_BEGIN_NAMESPACE

QT_DEFINE_QSDP_SPECIALIZATION_DTOR(QCanBusDeviceInfoPrivate)

/*!
    \class QCanBusDeviceInfo
    \inmodule QtSerialBus
    \since 5.9

    \brief The QCanBusDeviceInfo provides information about CAN bus interfaces.

    Each plugin may support one or more interfaces with different
    capabilities. This class provides information about available functions.
*/

/*!
    Constructs a copy of \a other.
*/
QCanBusDeviceInfo::QCanBusDeviceInfo(const QCanBusDeviceInfo &) = default;

/*!
    \fn QCanBusDeviceInfo::QCanBusDeviceInfo(QCanBusDeviceInfo &&other)

    Move-constructs a CAN bus device info from \a other.
    \since 6.10
*/

/*!
    Constructs a CAN bus device info from QCanBusDeviceInfoPrivate \a dd.
    \internal
*/
QCanBusDeviceInfo::QCanBusDeviceInfo(QCanBusDeviceInfoPrivate &dd) :
    d_ptr(&dd)
{
}

/*!
    Destroys the CAN bus device info.
*/
QCanBusDeviceInfo::~QCanBusDeviceInfo() = default;

/*!
    \fn void QCanBusDeviceInfo::swap(QCanBusDeviceInfo &other)
    Swaps this CAN bus device info with \a other. This operation is very fast
    and never fails.
*/

/*!
    \fn QCanBusDeviceInfo &QCanBusDeviceInfo::operator=(QCanBusDeviceInfo &&other)

    Move-assigns \a other to this QCanBusDeviceInfo instance.
*/

/*!
    Assigns \a other to this CAN bus device info and returns a reference to this
    CAN bus device info.
*/
QCanBusDeviceInfo &QCanBusDeviceInfo::operator=(const QCanBusDeviceInfo &) = default;

 /*!
    \since 6.2
     Returns the plugin name of this CAN bus interface, e.g. "peakcan".

     This corresponds to the \c plugin parameter of QCanBus::createDevice().
 */
QString QCanBusDeviceInfo::plugin() const
{
    return d_ptr->plugin;
}

/*!
    Returns the interface name of this CAN bus interface, e.g. "can0".

    This corresponds to the \c interfaceName parameter of QCanBus::createDevice().
*/
QString QCanBusDeviceInfo::name() const
{
    return d_ptr->name;
}

/*!
    \since 5.11
    Returns a textual description of the CAN bus interface, if available.
    Example output: "PCAN USB Pro FD". If no description is available,
    an empty string is returned.
*/
QString QCanBusDeviceInfo::description() const
{
    return d_ptr->description;
}

/*!
    \since 5.11
    Returns the serial number of the CAN bus interface as string, if available.
    Otherwise, an empty string is returned.

    \sa alias()
*/
QString QCanBusDeviceInfo::serialNumber() const
{
    return d_ptr->serialNumber;
}

/*!
    \since 6.0
    Returns a user defineable alias associated with this CAN bus interface.

    Some CAN bus interfaces can have a user defined alias associated. This is mostly
    done with the CAN hardware vendors tools. The alias allows to identify this
    hardware later, especially when multiple interfaces are connected.

    \note In contrast to serialNumber(), the alias is not guaranteed to be unique.

    If this function is not supported by the CAN plugin, an empty string is returned.
    \sa serialNumber()
*/
QString QCanBusDeviceInfo::alias() const
{
    return d_ptr->alias;
}

/*!
    \since 5.11
    Returns the sequential channel number of the CAN bus interface, starting
    with zero. For example, a two channel CAN interface may have the channels
    0 and 1. If the interface has only one channel or if no information about
    the channel is available, zero is returned.
*/
int QCanBusDeviceInfo::channel() const
{
    return d_ptr->channel;
}

/*!
    Returns true, if the CAN bus interface is CAN FD (flexible data rate) capable.

    If this information is not available, false is returned.
*/
bool QCanBusDeviceInfo::hasFlexibleDataRate() const
{
    return d_ptr->hasFlexibleDataRate;
}

/*!
    Returns true, if the CAN bus interface is virtual (i.e. not connected to real
    CAN hardware).

    If this information is not available, false is returned.
*/
bool QCanBusDeviceInfo::isVirtual() const
{
    return d_ptr->isVirtual;
}

QT_END_NAMESPACE
