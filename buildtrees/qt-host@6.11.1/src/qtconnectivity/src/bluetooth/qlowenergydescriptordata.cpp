// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qlowenergydescriptordata.h"

#include <QtCore/qbytearray.h>

QT_BEGIN_NAMESPACE

struct QLowEnergyDescriptorDataPrivate : public QSharedData
{
    QLowEnergyDescriptorDataPrivate() : readable(true), writable(true) {}

    QBluetoothUuid uuid;
    QByteArray value;
    QBluetooth::AttAccessConstraints readConstraints;
    QBluetooth::AttAccessConstraints writeConstraints;
    bool readable;
    bool writable;
};

/*!
    \since 5.7
    \class QLowEnergyDescriptorData
    \brief The QLowEnergyDescriptorData class is used to create GATT service data.
    \inmodule QtBluetooth
    \ingroup shared

    An object of this class provides a descriptor to be added to a
    \l QLowEnergyCharacteristicData object via \l QLowEnergyCharacteristicData::addDescriptor().

    \note The member functions related to access permissions are only applicable to those
          types of descriptors for which the Bluetooth specification does not prescribe if
          and how their values can be accessed.

    \sa QLowEnergyCharacteristicData
    \sa QLowEnergyServiceData
    \sa QLowEnergyController::addService
*/

/*! Creates a new invalid object of this class. */
QLowEnergyDescriptorData::QLowEnergyDescriptorData() : d(new QLowEnergyDescriptorDataPrivate)
{
}

/*!
  Creates a new object of this class with UUID and value being provided by \a uuid and \a value,
  respectively.
 */
QLowEnergyDescriptorData::QLowEnergyDescriptorData(const QBluetoothUuid &uuid,
                                                   const QByteArray &value)
    : d(new QLowEnergyDescriptorDataPrivate)
{
    setUuid(uuid);
    setValue(value);
}

/*! Constructs a new object of this class that is a copy of \a other. */
QLowEnergyDescriptorData::QLowEnergyDescriptorData(const QLowEnergyDescriptorData &other)
    : d(other.d)
{
}

/*! Destroys this object. */
QLowEnergyDescriptorData::~QLowEnergyDescriptorData()
{
}

/*! Makes this object a copy of \a other and returns the new value of this object. */
QLowEnergyDescriptorData &QLowEnergyDescriptorData::operator=(const QLowEnergyDescriptorData &other)
{
    d = other.d;
    return *this;
}

/*! Returns the value of this descriptor. */
QByteArray QLowEnergyDescriptorData::value() const
{
    return d->value;
}

/*!
  Sets the value of this descriptor to \a value. It will be sent to a peer device
  exactly the way it is provided here, so callers need to take care of things such as endianness.
 */
void QLowEnergyDescriptorData::setValue(const QByteArray &value)
{
    d->value = value;
}

/*! Returns the UUID of this descriptor. */
QBluetoothUuid QLowEnergyDescriptorData::uuid() const
{
    return d->uuid;
}

/*! Sets the UUID of this descriptor to \a uuid. */
void QLowEnergyDescriptorData::setUuid(const QBluetoothUuid &uuid)
{
    d->uuid = uuid;
}

/*! Returns true if and only if this object has a non-null UUID. */
bool QLowEnergyDescriptorData::isValid() const
{
    return !uuid().isNull();
}

/*!
  Specifies whether the value of this descriptor is \a readable and if so, under which
  \a constraints.
  \sa setWritePermissions()
 */
void QLowEnergyDescriptorData::setReadPermissions(bool readable,
                                                  QBluetooth::AttAccessConstraints constraints)
{
    d->readable = readable;
    d->readConstraints = constraints;
}

/*! Returns \c true if the value of this descriptor is readable and \c false otherwise. */
bool QLowEnergyDescriptorData::isReadable() const
{
    return d->readable;
}

/*!
  Returns the constraints under which the value of this descriptor can be read. This value
  is only relevant if \l isReadable() returns \c true.
 */
QBluetooth::AttAccessConstraints QLowEnergyDescriptorData::readConstraints() const
{
    return d->readConstraints;
}

/*!
  Specifies whether the value of this descriptor is \a writable and if so, under which
  \a constraints.
  \sa setReadPermissions()
 */
void QLowEnergyDescriptorData::setWritePermissions(bool writable,
                                                   QBluetooth::AttAccessConstraints constraints)
{
    d->writable = writable;
    d->writeConstraints = constraints;
}

/*! Returns \c true if the value of this descriptor is writable and \c false otherwise. */
bool QLowEnergyDescriptorData::isWritable() const
{
    return d->writable;
}

/*!
  Returns the constraints under which the value of this descriptor can be written. This value
  is only relevant if \l isWritable() returns \c true.
 */
QBluetooth::AttAccessConstraints QLowEnergyDescriptorData::writeConstraints() const
{
    return d->writeConstraints;
}

/*!
   \fn void QLowEnergyDescriptorData::swap(QLowEnergyDescriptorData &other)
    Swaps this object with \a other.
 */

/*!
   \fn bool QLowEnergyDescriptorData::operator==(const QLowEnergyDescriptorData &a,
                                                 const QLowEnergyDescriptorData &b)

   \brief Returns \c true if \a a and \a b are equal with respect to their public state,
   otherwise returns \c false.
 */

/*!
    \fn bool QLowEnergyDescriptorData::operator!=(const QLowEnergyDescriptorData &a,
                                                  const QLowEnergyDescriptorData &b)

    \brief Returns \c true if \a a and \a b are unequal with respect to their public state,
    otherwise returns \c false.
 */

/*!
    \brief Returns \c true if \a a and \a b are equal with respect to their public state,
    otherwise returns \c false.
    \internal
 */
bool QLowEnergyDescriptorData::equals(const QLowEnergyDescriptorData &a,
                                      const QLowEnergyDescriptorData &b)
{
    return a.d == b.d || (
                a.uuid() == b.uuid()
                && a.value() == b.value()
                && a.isReadable() == b.isReadable()
                && a.isWritable() == b.isWritable()
                && a.readConstraints() == b.readConstraints()
                && a.writeConstraints() == b.writeConstraints());
}

QT_END_NAMESPACE
