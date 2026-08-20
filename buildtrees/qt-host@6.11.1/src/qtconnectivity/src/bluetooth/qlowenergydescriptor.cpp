// Copyright (C) 2016 The Qt Company Ltd.
// Copyright (C) 2016 BlackBerry Limited. All rights reserved.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include <QtBluetooth/QLowEnergyService>
#include "qlowenergyserviceprivate_p.h"
#include "qlowenergydescriptor.h"

QT_BEGIN_NAMESPACE

QT_IMPL_METATYPE_EXTERN(QLowEnergyDescriptor)

/*!
    \class QLowEnergyDescriptor
    \inmodule QtBluetooth
    \brief The QLowEnergyDescriptor class stores information about the Bluetooth
    Low Energy descriptor.
    \since 5.4

    QLowEnergyDescriptor provides information about a Bluetooth Low Energy
    descriptor's name(), uuid(), and value(). Descriptors are
    encapsulated by Bluetooth Low Energy characteristics and provide additional
    contextual information about the characteristic (data format, notification activation
    and so on).

    The descriptor value may be written via the QLowEnergyService instance
    that manages the service to which this descriptor belongs. The
    \l {QLowEnergyService::writeDescriptor()} function writes the new value.
    The \l {QLowEnergyService::descriptorWritten()} signal
    is emitted upon success. The cached value() of this object is updated accordingly.

    \sa QLowEnergyService, QLowEnergyCharacteristic
*/

struct QLowEnergyDescriptorPrivate
{
    QLowEnergyHandle charHandle;
    QLowEnergyHandle descHandle;
};

/*!
    Construct a new QLowEnergyDescriptor. A default-constructed instance
    of this class is always invalid.
*/
QLowEnergyDescriptor::QLowEnergyDescriptor():
    d_ptr(nullptr)
{
}

/*!
    Construct a new QLowEnergyDescriptor that is a copy of \a other.

    The two copies continue to share the same underlying data which does not detach
    upon write.
*/
QLowEnergyDescriptor::QLowEnergyDescriptor(const QLowEnergyDescriptor &other):
    d_ptr(other.d_ptr)
{
    if (other.data) {
        data = new QLowEnergyDescriptorPrivate();
        data->charHandle = other.data->charHandle;
        data->descHandle = other.data->descHandle;
    }
}

/*!
    \internal

*/
QLowEnergyDescriptor::QLowEnergyDescriptor(QSharedPointer<QLowEnergyServicePrivate> p,
                                           QLowEnergyHandle charHandle,
                                           QLowEnergyHandle descHandle):
    d_ptr(p)
{
    data = new QLowEnergyDescriptorPrivate();
    data->charHandle = charHandle;
    data->descHandle = descHandle;

}

/*!
    Destroys the QLowEnergyDescriptor object.
*/
QLowEnergyDescriptor::~QLowEnergyDescriptor()
{
    delete data;
}

/*!
    Makes a copy of \a other and assigns it to this QLowEnergyDescriptor object.
    The two copies continue to share the same service and controller details.
*/
QLowEnergyDescriptor &QLowEnergyDescriptor::operator=(const QLowEnergyDescriptor &other)
{
    d_ptr = other.d_ptr;

    if (!other.data) {
        if (data) {
            delete data;
            data = nullptr;
        }
    } else {
        if (!data)
            data = new QLowEnergyDescriptorPrivate();

        data->charHandle = other.data->charHandle;
        data->descHandle = other.data->descHandle;
    }

    return *this;
}

/*!
    \fn bool QLowEnergyDescriptor::operator==(const QLowEnergyDescriptor &a,
                                              const QLowEnergyDescriptor &b)
    \brief Returns \c true if \a a is equal to \a b; otherwise \c false.

    Two QLowEnergyDescriptor instances are considered to be equal if they refer to
    the same descriptor on the same remote Bluetooth Low Energy device  or both
    instances have been default-constructed.
 */

/*!
    \fn bool QLowEnergyDescriptor::operator!=(const QLowEnergyDescriptor &a,
                                              const QLowEnergyDescriptor &b)
   \brief Returns \c true if \a a is not equal to \a b; otherwise \c false.

    Two QLowEnergyDescriptor instances are considered to be equal if they refer to
    the same descriptor on the same remote Bluetooth Low Energy device  or both
    instances have been default-constructed.
 */

/*!
    \brief Returns \c true if \a other is equal to this QLowEnergyCharacteristic,
    otherwise \c false.
    \internal

    Two QLowEnergyDescriptor instances are considered to be equal if they refer to
    the same descriptor on the same remote Bluetooth Low Energy device or both
    instances have been default-constructed.
 */
bool QLowEnergyDescriptor::equals(const QLowEnergyDescriptor &a, const QLowEnergyDescriptor &b)
{
    if (a.d_ptr != b.d_ptr)
        return false;

    if ((a.data && !b.data) || (!a.data && b.data))
        return false;

    if (!a.data)
        return true;

    if (a.data->charHandle != b.data->charHandle || a.data->descHandle != b.data->descHandle) {
        return false;
    }

    return true;
}

/*!
    Returns \c true if the QLowEnergyDescriptor object is valid, otherwise returns \c false.

    An invalid descriptor instance is not associated with any service (default-constructed)
    or the associated service is no longer valid due to a disconnect from
    the underlying Bluetooth Low Energy device, for example. Once the object is invalid
    it cannot become valid anymore.

    \note If a QLowEnergyDescriptor instance turns invalid due to a disconnect
    from the underlying device, the information encapsulated by the current
    instance remains as it was at the time of the disconnect. Therefore it can be
    retrieved after the disconnect event.
*/
bool QLowEnergyDescriptor::isValid() const
{
    if (d_ptr.isNull() || !data)
        return false;

    if (d_ptr->state == QLowEnergyService::InvalidService)
        return false;

    return true;
}

/*!
    Returns the UUID of this descriptor if \l isValid() returns \c true; otherwise a
    \l {QUuid::isNull()}{null} UUID.
*/
QBluetoothUuid QLowEnergyDescriptor::uuid() const
{
    if (d_ptr.isNull() || !data
                       || !d_ptr->characteristicList.contains(data->charHandle)
                       || !d_ptr->characteristicList[data->charHandle].
                                        descriptorList.contains(data->descHandle)) {
        return QBluetoothUuid();
    }

    return d_ptr->characteristicList[data->charHandle].descriptorList[data->descHandle].uuid;
}

/*!
    \internal

    Returns the handle of the descriptor or \c 0 if the handle
    cannot be accessed on the platform or the descriptor is invalid.

    \note On \macos and iOS handles can differ from 0, but these
    values have no special meaning outside of internal/private API.
*/
QLowEnergyHandle QLowEnergyDescriptor::handle() const
{
    if (!data)
        return 0;

    return data->descHandle;
}

/*!
    Returns the cached value of the descriptor.

    The cached descriptor value may be updated using
    \l QLowEnergyService::writeDescriptor() or \l QLowEnergyService::readDescriptor().
*/
QByteArray QLowEnergyDescriptor::value() const
{
    if (d_ptr.isNull() || !data
                       || !d_ptr->characteristicList.contains(data->charHandle)
                       || !d_ptr->characteristicList[data->charHandle].
                                        descriptorList.contains(data->descHandle)) {
        return QByteArray();
    }

    return d_ptr->characteristicList[data->charHandle].descriptorList[data->descHandle].value;
}

/*!
    Returns the human-readable name of the descriptor.

    The name is based on the descriptor's \l type(). The complete list
    of descriptor types can be found under
    \l {https://developer.bluetooth.org/gatt/descriptors/Pages/DescriptorsHomePage.aspx}{Bluetooth.org Descriptors}.

    The returned string is empty if the \l type() is unknown.

    \sa type(), QBluetoothUuid::descriptorToString()
*/

QString QLowEnergyDescriptor::name() const
{
    return QBluetoothUuid::descriptorToString(type());
}

/*!
    Returns the type of the descriptor.

    \sa name()
 */
QBluetoothUuid::DescriptorType QLowEnergyDescriptor::type() const
{
    const QBluetoothUuid u = uuid();
    bool ok = false;
    QBluetoothUuid::DescriptorType shortUuid = static_cast<QBluetoothUuid::DescriptorType>(u.toUInt16(&ok));

    if (!ok)
        return QBluetoothUuid::DescriptorType::UnknownDescriptorType;

    switch (shortUuid) {
        case QBluetoothUuid::DescriptorType::CharacteristicExtendedProperties:
    case QBluetoothUuid::DescriptorType::CharacteristicUserDescription:
    case QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration:
    case QBluetoothUuid::DescriptorType::ServerCharacteristicConfiguration:
    case QBluetoothUuid::DescriptorType::CharacteristicPresentationFormat:
    case QBluetoothUuid::DescriptorType::CharacteristicAggregateFormat:
    case QBluetoothUuid::DescriptorType::ValidRange:
    case QBluetoothUuid::DescriptorType::ExternalReportReference:
    case QBluetoothUuid::DescriptorType::ReportReference:
        return (QBluetoothUuid::DescriptorType) shortUuid;
    default:
        break;
    }

    return QBluetoothUuid::DescriptorType::UnknownDescriptorType;
}

/*!
    \internal

    Returns the handle of the characteristic to which this descriptor belongs
 */
QLowEnergyHandle QLowEnergyDescriptor::characteristicHandle() const
{
    if (d_ptr.isNull() || !data)
        return 0;

    return data->charHandle;
}

QT_END_NAMESPACE
