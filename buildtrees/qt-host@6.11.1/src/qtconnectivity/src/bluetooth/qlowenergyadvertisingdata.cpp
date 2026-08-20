// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qlowenergyadvertisingdata.h"

#include <cstring>

QT_BEGIN_NAMESPACE

class QLowEnergyAdvertisingDataPrivate : public QSharedData
{
public:
    QLowEnergyAdvertisingDataPrivate()
        : manufacturerId(QLowEnergyAdvertisingData::invalidManufacturerId())
        , discoverability(QLowEnergyAdvertisingData::DiscoverabilityNone)
        , includePowerLevel(false)
    {
    }

    QString localName;
    QByteArray manufacturerData;
    QByteArray rawData;
    QList<QBluetoothUuid> services;
    quint16 manufacturerId;
    QLowEnergyAdvertisingData::Discoverability discoverability;
    bool includePowerLevel;
};

/*!
    \since 5.7
    \class QLowEnergyAdvertisingData
    \brief The QLowEnergyAdvertisingData class represents the data to be broadcast during
           Bluetooth Low Energy advertising.
    \inmodule QtBluetooth
    \ingroup shared

    This data can include the device name, GATT services offered by the device, and so on.
    The data set via this class will be used when advertising is started by calling
    \l QLowEnergyController::startAdvertising(). Objects of this class can represent an
    Advertising Data packet or a Scan Response packet.

    \section2 Advertising Data Limitations

    The maximum length of the advertisement data depends on the bluetooth
    device and the platform bluetooth stack. For maximum compatibility, it is
    recommended to limit the advertising data size to the legacy advertising
    limit of 31 bytes.

    If the variable-length data set via this class exceeds the supported limit,
    the behavior will depend on the underlying Bluetooth stack implementation.
    The most typical possibilities are:

    \list
        \li the extra data can be truncated or entirely left out;
        \li the advertising fails to start.
    \endlist

    For example, on Android, advertising will fail if the advertising data is
    larger than 31 bytes. On BlueZ DBus backend, the advertising length limit
    and the behavior when it is exceeded is up to BlueZ; it may, for example
    support extended advertising, or may fail to start the advertising.

    \sa QLowEnergyAdvertisingParameters
    \sa QLowEnergyController::startAdvertising()
*/

/*!
   \enum QLowEnergyAdvertisingData::Discoverability

   The discoverability of the advertising device as defined by the Generic Access Profile.

   \value DiscoverabilityNone
       The advertising device does not wish to be discoverable by scanning devices.
   \value DiscoverabilityLimited
       The advertising device wishes to be discoverable with a high priority. Note that this mode
       is not compatible with using a white list. The value of
       \l QLowEnergyAdvertisingParameters::filterPolicy() is always assumed to be
       \l QLowEnergyAdvertisingParameters::IgnoreWhiteList when limited discoverability
       is used.
   \value DiscoverabilityGeneral
       The advertising device wishes to be discoverable by scanning devices.
 */

/*!
   Creates a new object of this class. All values are initialized to their defaults
   according to the Bluetooth Low Energy specification.
 */
QLowEnergyAdvertisingData::QLowEnergyAdvertisingData() : d(new QLowEnergyAdvertisingDataPrivate)
{
}

/*! Constructs a new object of this class that is a copy of \a other. */
QLowEnergyAdvertisingData::QLowEnergyAdvertisingData(const QLowEnergyAdvertisingData &other)
    : d(other.d)
{
}

/*! Destroys this object. */
QLowEnergyAdvertisingData::~QLowEnergyAdvertisingData()
{
}

/*! Makes this object a copy of \a other and returns the new value of this object. */
QLowEnergyAdvertisingData &QLowEnergyAdvertisingData::operator=(const QLowEnergyAdvertisingData &other)
{
    d = other.d;
    return *this;
}

/*!
   Specifies that \a name should be broadcast as the name of the device. If the full name does not
   fit into the advertising data packet, an abbreviated name is sent, as described by the
   Bluetooth Low Energy specification.

   On Android, the local name cannot be changed. Android always uses the device name.
   If this local name is not empty, the Android implementation includes the device name
   in the advertisement packet; otherwise the device name is omitted from the advertisement
   packet.

   \sa localName()
 */
void QLowEnergyAdvertisingData::setLocalName(const QString &name)
{
    d->localName = name;
}

/*!
   Returns the name of the local device that is to be advertised.

   \sa setLocalName()
 */
QString QLowEnergyAdvertisingData::localName() const
{
    return d->localName;
}

/*!
   Sets the manufacturer id and data. The \a id parameter is a company identifier as assigned
   by the Bluetooth SIG. The \a data parameter is an arbitrary value.

    \note \macos and iOS do not support advertising of manufacturer id or data,
    so the provided parameters will be ignored on these platforms.
 */
void QLowEnergyAdvertisingData::setManufacturerData(quint16 id, const QByteArray &data)
{
    d->manufacturerId = id;
    d->manufacturerData = data;
}

/*!
   Returns the manufacturer id.
   The default is \l QLowEnergyAdvertisingData::invalidManufacturerId(), which means
   the data will not be advertised.
 */
quint16 QLowEnergyAdvertisingData::manufacturerId() const
{
    return d->manufacturerId;
}

/*!
   Returns the manufacturer data. The default is an empty byte array.
 */
QByteArray QLowEnergyAdvertisingData::manufacturerData() const
{
    return d->manufacturerData;
}

/*!
   Specifies whether to include the device's transmit power level in the advertising data. If
   \a doInclude is \c true, the data will be included, otherwise it will not.
 */
void QLowEnergyAdvertisingData::setIncludePowerLevel(bool doInclude)
{
    d->includePowerLevel = doInclude;
}

/*!
   Returns whether to include the device's transmit power level in the advertising data.
   The default is \c false.
 */
bool QLowEnergyAdvertisingData::includePowerLevel() const
{
    return d->includePowerLevel;
}

/*!
   Sets the discoverability type of the advertising device to \a mode.
   \note Discoverability information can only appear in an actual advertising data packet. If
         this object acts as scan response data, a call to this function will have no effect
         on the scan response sent.
 */
void QLowEnergyAdvertisingData::setDiscoverability(QLowEnergyAdvertisingData::Discoverability mode)
{
    d->discoverability = mode;
}

/*!
   Returns the discoverability mode of the advertising device.
   The default is \l DiscoverabilityNone.
 */
QLowEnergyAdvertisingData::Discoverability QLowEnergyAdvertisingData::discoverability() const
{
    return d->discoverability;
}

/*!
   Specifies that the service UUIDs in \a services should be advertised.
   If the entire list does not fit into the packet, an incomplete list is sent as specified
   by the Bluetooth Low Energy specification.
 */
void QLowEnergyAdvertisingData::setServices(const QList<QBluetoothUuid> &services)
{
    d->services = services;
}

/*!
   Returns the list of service UUIDs to be advertised.
   By default, this list is empty.
 */
QList<QBluetoothUuid> QLowEnergyAdvertisingData::services() const
{
    return d->services;
}

/*!
  Sets the data to be advertised to \a data. If the value is not an empty byte array, it will
  be sent as-is as the advertising data and all other data in this object will be ignored.
  This can be used to send non-standard data.
  \note If \a data is longer than 31 bytes, it will be truncated. It is the caller's responsibility
        to ensure that \a data is well-formed.

  Setting raw advertising data is only supported on the \l {Linux Specific}
  {Linux Bluetooth Kernel API} backend. Other backends do not allow to specify
  the raw advertising data as a global field.
 */
void QLowEnergyAdvertisingData::setRawData(const QByteArray &data)
{
    d->rawData = data;
}

/*!
  Returns the user-supplied raw data to be advertised. The default is an empty byte array.
 */
QByteArray QLowEnergyAdvertisingData::rawData() const
{
    return d->rawData;
}

/*!
   \fn void QLowEnergyAdvertisingData::swap(QLowEnergyAdvertisingData &other)
    Swaps this object with \a other.
 */

/*!
    \brief Returns \c true if \a a and \a b are equal with respect to their public state,
    otherwise returns \c false.
    \internal
 */
bool QLowEnergyAdvertisingData::equals(const QLowEnergyAdvertisingData &a,
                                       const QLowEnergyAdvertisingData &b)
{
    if (a.d == b.d)
        return true;
    return a.discoverability() == b.discoverability()
            && a.includePowerLevel() == b.includePowerLevel() && a.localName() == b.localName()
            && a.manufacturerData() == b.manufacturerData()
            && a.manufacturerId() == b.manufacturerId() && a.services() == b.services()
            && a.rawData() == b.rawData();
}

/*!
    \fn bool QLowEnergyAdvertisingData::operator!=(const QLowEnergyAdvertisingData &data1,
                                                   const QLowEnergyAdvertisingData &data2)
    \brief Returns \c true if \a data1 and \a data2 are not equal with respect to their
    public state, otherwise returns \c false.
 */

/*!
    \fn bool QLowEnergyAdvertisingData::operator==(const QLowEnergyAdvertisingData &data1,
                                                   const QLowEnergyAdvertisingData &data2)
    \brief Returns \c true if \a data1 and \a data2 are equal with respect to their public
    state, otherwise returns \c false.
 */

/*!
   \fn static quint16 QLowEnergyAdvertisingData::invalidManufacturerId();
   Returns an invalid manufacturer id. If this value is set as the manufacturer id
   (which it is by default), no manufacturer data will be present in the advertising data.
 */

QT_END_NAMESPACE
