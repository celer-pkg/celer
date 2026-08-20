// Copyright (C) 2016 BlackBerry Limited. All rights reserved.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qlowenergycharacteristic.h"
#include "qlowenergyserviceprivate_p.h"
#include <QHash>

QT_BEGIN_NAMESPACE

QT_IMPL_METATYPE_EXTERN(QLowEnergyCharacteristic)

/*!
    \class QLowEnergyCharacteristic
    \inmodule QtBluetooth
    \brief The QLowEnergyCharacteristic class stores information about a Bluetooth
    Low Energy service characteristic.

    \since 5.4

    QLowEnergyCharacteristic provides information about a Bluetooth Low Energy
    service characteristic's name(), uuid(), value(), properties(), and
    descriptors(). To obtain the characteristic's specification
    and information, it is necessary to connect to the device using the
    QLowEnergyService and QLowEnergyController classes.

    The characteristic value may be written via the \l QLowEnergyService instance
    that manages the service to which this characteristic belongs. The
    \l {QLowEnergyService::writeCharacteristic()} function writes the new value.
    The \l {QLowEnergyService::characteristicWritten()} signal is emitted upon success.
    The value() of this object is automatically updated accordingly.

    Characteristics may contain none, one or more descriptors. They can be individually
    retrieved using the \l descriptor() function. The \l descriptors() function returns
    all descriptors as a list. The general purpose of a descriptor is to add contextual
    information to the characteristic. For example, the descriptor might provide
    format or range information specifying how the characteristic's value is to be\
    interpreted.

    \sa QLowEnergyService, QLowEnergyDescriptor
*/

/*!
    \enum QLowEnergyCharacteristic::PropertyType

    This enum describes the properties of a characteristic.

    \value Unknown                  The type is not known.
    \value Broadcasting             Allow for the broadcasting of Generic Attributes (GATT) characteristic values.
    \value Read                     Allow the characteristic values to be read.
    \value WriteNoResponse          Allow characteristic values without responses to be written.
    \value Write                    Allow for characteristic values to be written.
    \value Notify                   Permits notification of characteristic values.
    \value Indicate                 Permits indications of characteristic values.
    \value WriteSigned              Permits signed writes of the GATT characteristic values.
    \value ExtendedProperty         Additional characteristic properties are defined in the characteristic's
                                    extended properties descriptor.

    It is not recommended to set both Notify and Indicate properties on the same characteristic
    as the underlying Bluetooth stack behaviors differ from platform to platform. Please see
    \l QLowEnergyCharacteristic::clientCharacteristicConfiguration


    \sa properties()
*/

struct QLowEnergyCharacteristicPrivate
{
    QLowEnergyHandle handle;
};

/*!
    Construct a new QLowEnergyCharacteristic. A default-constructed instance of
    this class is always invalid.

    \sa isValid()
*/
QLowEnergyCharacteristic::QLowEnergyCharacteristic():
    d_ptr(nullptr)
{

}

/*!
    Construct a new QLowEnergyCharacteristic that is a copy of \a other.

    The two copies continue to share the same underlying data which does not detach
    upon write.
*/
QLowEnergyCharacteristic::QLowEnergyCharacteristic(const QLowEnergyCharacteristic &other):
    d_ptr(other.d_ptr)
{
    if (other.data) {
        data = new QLowEnergyCharacteristicPrivate();
        data->handle = other.data->handle;
    }
}

/*!
    \internal
*/
QLowEnergyCharacteristic::QLowEnergyCharacteristic(
        QSharedPointer<QLowEnergyServicePrivate> p, QLowEnergyHandle handle):
    d_ptr(p)
{
    data = new QLowEnergyCharacteristicPrivate();
    data->handle = handle;
}

/*!
    Destroys the QLowEnergyCharacteristic object.
*/
QLowEnergyCharacteristic::~QLowEnergyCharacteristic()
{
    delete data;
}

/*!
    Returns the human-readable name of the characteristic.

    The name is based on the characteristic's \l uuid() which must have been
    standardized. The complete list of characteristic types can be found
    under \l {https://developer.bluetooth.org/gatt/characteristics/Pages/CharacteristicsHome.aspx}{Bluetooth.org Characteristics}.

    The returned string is empty if the \l uuid() is unknown.

    \sa QBluetoothUuid::characteristicToString()
*/
QString QLowEnergyCharacteristic::name() const
{
    return QBluetoothUuid::characteristicToString(
                static_cast<QBluetoothUuid::CharacteristicType>(uuid().toUInt16()));
}

/*!
    Returns the UUID of the characteristic if \l isValid() returns \c true; otherwise a
    \l {QUuid::isNull()}{null} UUID.
*/
QBluetoothUuid QLowEnergyCharacteristic::uuid() const
{
    if (d_ptr.isNull() || !data
        || !d_ptr->characteristicList.contains(data->handle))
        return QBluetoothUuid();

    return d_ptr->characteristicList[data->handle].uuid;
}

/*!
    Returns the properties of the characteristic.

    The properties define the access permissions for the characteristic.
*/
QLowEnergyCharacteristic::PropertyTypes QLowEnergyCharacteristic::properties() const
{
    if (d_ptr.isNull() || !data
        || !d_ptr->characteristicList.contains(data->handle))
        return QLowEnergyCharacteristic::Unknown;

    return d_ptr->characteristicList[data->handle].properties;
}

/*!
    Returns the cached value of the characteristic.

    If the characteristic's \l properties() permit writing of new values,
    the value can be updated using \l QLowEnergyService::writeCharacteristic().

    The cache is updated during the associated service's
    \l {QLowEnergyService::discoverDetails()} {detail discovery}, a successful
    \l {QLowEnergyService::readCharacteristic()}{read}/\l {QLowEnergyService::writeCharacteristic()}{write}
    operation or when an update notification is received.

    The returned \l QByteArray always remains empty if the characteristic does not
    have the \l {QLowEnergyCharacteristic::Read}{read permission}. In such cases only
    the \l QLowEnergyService::characteristicChanged() or
    \l QLowEnergyService::characteristicWritten() may provice information about the
    value of this characteristic.
*/
QByteArray QLowEnergyCharacteristic::value() const
{
    if (d_ptr.isNull() || !data
        || !d_ptr->characteristicList.contains(data->handle))
        return QByteArray();

    return d_ptr->characteristicList[data->handle].value;
}

/*!
    \internal

    Returns the handle of the characteristic's value attribute;
    or \c 0 if the handle cannot be accessed on the platform or
    if the characteristic is invalid.

    \note On \macos and iOS handles can differ from 0, but these
    values have no special meaning outside of internal/private API.
*/
QLowEnergyHandle QLowEnergyCharacteristic::handle() const
{
    if (d_ptr.isNull() || !data
        || !d_ptr->characteristicList.contains(data->handle))
        return 0;

    return d_ptr->characteristicList[data->handle].valueHandle;
}

/*!
    Makes a copy of \a other and assigns it to this \l QLowEnergyCharacteristic object.
    The two copies continue to share the same service and controller details.
*/
QLowEnergyCharacteristic &QLowEnergyCharacteristic::operator=(const QLowEnergyCharacteristic &other)
{
    d_ptr = other.d_ptr;

    if (!other.data) {
        if (data) {
            delete data;
            data = nullptr;
        }
    } else {
        if (!data)
            data = new QLowEnergyCharacteristicPrivate();

        data->handle = other.data->handle;
    }
    return *this;
}

/*!
    \fn bool QLowEnergyCharacteristic::operator==(const QLowEnergyCharacteristic &a,
                                                  const QLowEnergyCharacteristic &b)
    \brief Returns \c true if \a a is equal to \a b, otherwise \c false.

    Two \l QLowEnergyCharacteristic instances are considered to be equal if they refer to
    the same characteristic on the same remote Bluetooth Low Energy device or both instances
    have been default-constructed.
 */

/*!
    \fn bool QLowEnergyCharacteristic::operator!=(const QLowEnergyCharacteristic &a,
                                                  const QLowEnergyCharacteristic &b)
    \brief Returns \c true if \a a and \a b are not equal; otherwise \c
    false.

    Two QLowEnergyCharcteristic instances are considered to be equal if they refer to
    the same characteristic on the same remote Bluetooth Low Energy device or both instances
    have been default-constructed.
 */

/*!
    \brief Returns \c true if \a a is equal to \a b; otherwise \c false.
    \internal

    Two \l QLowEnergyCharacteristic instances are considered to be equal if they refer to
    the same characteristic on the same remote Bluetooth Low Energy device or both instances
    have been default-constructed.
 */
bool QLowEnergyCharacteristic::equals(const QLowEnergyCharacteristic &a,
                                      const QLowEnergyCharacteristic &b)
{
    if (a.d_ptr != b.d_ptr)
        return false;

    if ((a.data && !b.data) || (!a.data && b.data))
        return false;

    if (!a.data)
        return true;

    if (a.data->handle != b.data->handle)
        return false;

    return true;
}

/*!
    Returns \c true if the QLowEnergyCharacteristic object is valid, otherwise returns \c false.

    An invalid characteristic object is not associated with any service (default-constructed)
    or the associated service is no longer valid due to a disconnect from
    the underlying Bluetooth Low Energy device, for example. Once the object is invalid
    it cannot become valid anymore.

    \note If a \l QLowEnergyCharacteristic instance turns invalid due to a disconnect
    from the underlying device, the information encapsulated by the current
    instance remains as it was at the time of the disconnect. Therefore it can be
    retrieved after the disconnect event.
*/
bool QLowEnergyCharacteristic::isValid() const
{
    if (d_ptr.isNull() || !data)
        return false;

    if (d_ptr->state == QLowEnergyService::InvalidService)
        return false;

    return true;
}

/*!
    \internal

    Returns the handle of the characteristic or
    \c 0 if the handle cannot be accessed on the platform or if the
    characteristic is invalid.

    \note On \macos and iOS handles can differ from 0, but these
    values have no special meaning outside of internal/private API.

    \sa isValid()
 */
QLowEnergyHandle QLowEnergyCharacteristic::attributeHandle() const
{
    if (d_ptr.isNull() || !data)
        return 0;

    return data->handle;
}

/*!
    Returns the descriptor for \a uuid or an invalid \l QLowEnergyDescriptor instance.

    \sa descriptors()
*/
QLowEnergyDescriptor QLowEnergyCharacteristic::descriptor(const QBluetoothUuid &uuid) const
{
    if (d_ptr.isNull() || !data)
        return QLowEnergyDescriptor();

    CharacteristicDataMap::const_iterator charIt = d_ptr->characteristicList.constFind(data->handle);
    if (charIt != d_ptr->characteristicList.constEnd()) {
        const QLowEnergyServicePrivate::CharData &charDetails = charIt.value();

        DescriptorDataMap::const_iterator descIt = charDetails.descriptorList.constBegin();
        for ( ; descIt != charDetails.descriptorList.constEnd(); ++descIt) {
            const QLowEnergyHandle descHandle = descIt.key();
            const QLowEnergyServicePrivate::DescData &descDetails = descIt.value();

            if (descDetails.uuid == uuid)
                return QLowEnergyDescriptor(d_ptr, data->handle, descHandle);
        }
    }

    return QLowEnergyDescriptor();
}

/*!
    Returns the Client Characteristic Configuration Descriptor or an
    invalid \l QLowEnergyDescriptor instance if no
    Client Characteristic Configuration Descriptor exists.

    BTLE characteristics can support notifications and/or indications.
    In both cases, the peripheral will inform the central on
    each change of the characteristic's value. In the BTLE
    attribute protocol, notification messages are not confirmed
    by the central, while indications are confirmed.
    Notifications are considered faster, but unreliable, while
    indications are slower and more reliable.

    If a characteristic supports notification or indication,
    these can be enabled by writing special bit patterns to the
    Client Characteristic Configuration Descriptor.
    For convenience, these bit patterns are provided as
    \l QLowEnergyCharacteristic::CCCDDisable,
    \l QLowEnergyCharacteristic::CCCDEnableNotification, and
    \l QLowEnergyCharacteristic::CCCDEnableIndication.

    Enabling e.g. notification for a characteristic named
    \c mycharacteristic in a service called \c myservice
    could be done using the following code.
    \code
    auto cccd = mycharacteristic.clientCharacteristicConfiguration();
    if (!cccd.isValid()) {
        // your error handling
        return error;
    }
    myservice->writeDescriptor(cccd, QLowEnergyCharacteristic::CCCDEnableNotification);
    \endcode

    \note
    Calling \c characteristic.clientCharacteristicConfiguration() is equivalent to calling
    \c characteristic.descriptor(QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration).

    \note
    It is not recommended to use both notifications and indications on the same characteristic.
    This applies to both server-side when setting up the characteristics, as well as client-side
    when enabling them. The bluetooth stack behavior differs from platform to platform and the
    cross-platform behavior will likely be inconsistent. As an example a Bluez Linux client might
    unconditionally try to enable both mechanisms if both are supported, whereas a macOS client
    might unconditionally enable just the notifications. If both are needed consider creating two
    separate characteristics.

    \since 6.2
    \sa descriptor()
*/
QLowEnergyDescriptor QLowEnergyCharacteristic::clientCharacteristicConfiguration() const
{
    return descriptor(QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration);
}

/*!
    Returns the list of descriptors belonging to this characteristic; otherwise
    an empty list.

    \sa descriptor()
*/
QList<QLowEnergyDescriptor> QLowEnergyCharacteristic::descriptors() const
{
    QList<QLowEnergyDescriptor> result;

    if (d_ptr.isNull() || !data
                       || !d_ptr->characteristicList.contains(data->handle))
        return result;

    QList<QLowEnergyHandle> descriptorKeys = d_ptr->characteristicList[data->handle].
                                                    descriptorList.keys();

    std::sort(descriptorKeys.begin(), descriptorKeys.end());

    for (const QLowEnergyHandle descHandle : std::as_const(descriptorKeys)) {
        QLowEnergyDescriptor descriptor(d_ptr, data->handle, descHandle);
        result.append(descriptor);
    }

    return result;
}

/*!
    \variable QLowEnergyCharacteristic::CCCDDisable
    \since 6.2

    Bit pattern to write into Client Characteristic Configuration Descriptor
    to disable both notification and indication.

    \sa QLowEnergyCharacteristic::clientCharacteristicConfiguration
*/
/*!
    \variable QLowEnergyCharacteristic::CCCDEnableNotification
    \since 6.2

    Bit pattern to write into Client Characteristic Configuration Descriptor
    to enable notification.

    \sa QLowEnergyCharacteristic::clientCharacteristicConfiguration
*/
/*!
    \variable QLowEnergyCharacteristic::CCCDEnableIndication
    \since 6.2

    Bit pattern to write into Client Characteristic Configuration Descriptor
    to enable indication.

    \sa QLowEnergyCharacteristic::clientCharacteristicConfiguration
*/
const QByteArray QLowEnergyCharacteristic::CCCDDisable = QByteArray::fromHex("0000");
const QByteArray QLowEnergyCharacteristic::CCCDEnableNotification = QByteArray::fromHex("0100");
const QByteArray QLowEnergyCharacteristic::CCCDEnableIndication = QByteArray::fromHex("0200");

QT_END_NAMESPACE
