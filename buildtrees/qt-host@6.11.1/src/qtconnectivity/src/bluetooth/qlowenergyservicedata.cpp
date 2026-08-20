// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qlowenergyservicedata.h"

#include "qlowenergycharacteristicdata.h"

#include <QtCore/qloggingcategory.h>

QT_BEGIN_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(QT_BT)

struct QLowEnergyServiceDataPrivate : public QSharedData
{
    QLowEnergyServiceDataPrivate() : type(QLowEnergyServiceData::ServiceTypePrimary) {}

    QLowEnergyServiceData::ServiceType type;
    QBluetoothUuid uuid;
    QList<QLowEnergyService *> includedServices;
    QList<QLowEnergyCharacteristicData> characteristics;
};


/*!
    \since 5.7
    \class QLowEnergyServiceData
    \brief The QLowEnergyServiceData class is used to set up GATT service data.
    \inmodule QtBluetooth
    \ingroup shared

    An Object of this class provides a service to be added to a GATT server via
    \l QLowEnergyController::addService().
*/

/*!
   \enum QLowEnergyServiceData::ServiceType
   The type of GATT service.

   \value ServiceTypePrimary
       The service is a primary service.
   \value ServiceTypeSecondary
       The service is a secondary service. Secondary services are included by other services
       to implement some higher-level functionality.
 */

/*! Creates a new invalid object of this class. */
QLowEnergyServiceData::QLowEnergyServiceData() : d(new QLowEnergyServiceDataPrivate)
{
}

/*! Constructs a new object of this class that is a copy of \a other. */
QLowEnergyServiceData::QLowEnergyServiceData(const QLowEnergyServiceData &other) : d(other.d)
{
}

/*! Destroys this object. */
QLowEnergyServiceData::~QLowEnergyServiceData()
{
}

/*! Makes this object a copy of \a other and returns the new value of this object. */
QLowEnergyServiceData &QLowEnergyServiceData::operator=(const QLowEnergyServiceData &other)
{
    d = other.d;
    return *this;
}

/*! Returns the type of this service. */
QLowEnergyServiceData::ServiceType QLowEnergyServiceData::type() const
{
    return d->type;
}

/*! Sets the type of this service to \a type. */
void QLowEnergyServiceData::setType(ServiceType type)
{
    d->type = type;
}

/*! Returns the UUID of this service. */
QBluetoothUuid QLowEnergyServiceData::uuid() const
{
    return d->uuid;
}

/*! Sets the UUID of this service to \a uuid. */
void QLowEnergyServiceData::setUuid(const QBluetoothUuid &uuid)
{
    d->uuid = uuid;
}

/*! Returns the list of included services. */
QList<QLowEnergyService *> QLowEnergyServiceData::includedServices() const
{
    return d->includedServices;
}

/*!
  Sets the list of included services to \a services.
  All objects in this list must have been returned from a call to
  \l QLowEnergyController::addService.
  \sa addIncludedService()
*/
void QLowEnergyServiceData::setIncludedServices(const QList<QLowEnergyService *> &services)
{
    d->includedServices = services;
}

/*!
  Adds \a service to the list of included services.
  The \a service object must have been returned from a call to
  \l QLowEnergyController::addService. This requirement prevents circular includes
  (which are forbidden by the Bluetooth specification), and also helps to support the use case of
  including more than one service of the same type.
  \sa setIncludedServices()
*/
void QLowEnergyServiceData::addIncludedService(QLowEnergyService *service)
{
    d->includedServices << service;
}

/*! Returns the list of characteristics. */
QList<QLowEnergyCharacteristicData> QLowEnergyServiceData::characteristics() const
{
    return d->characteristics;
}

/*!
  Sets the list of characteristics to \a characteristics.
  Only valid characteristics are considered.
  \sa addCharacteristic()
 */
void QLowEnergyServiceData::setCharacteristics(const QList<QLowEnergyCharacteristicData> &characteristics)
{
    d->characteristics.clear();
    for (const QLowEnergyCharacteristicData &cd : characteristics)
        addCharacteristic(cd);
}

/*!
  Adds \a characteristic to the list of characteristics, if it is valid.
  \sa setCharacteristics()
 */
void QLowEnergyServiceData::addCharacteristic(const QLowEnergyCharacteristicData &characteristic)
{
    if (characteristic.isValid())
        d->characteristics << characteristic;
    else
        qCWarning(QT_BT) << "not adding invalid characteristic to service";
}

/*! Returns \c true if this service is has a non-null UUID. */
bool QLowEnergyServiceData::isValid() const
{
    return !uuid().isNull();
}

/*!
   \fn void QLowEnergyServiceData::swap(QLowEnergyServiceData &other)
    Swaps this object with \a other.
 */

/*!
   \fn bool QLowEnergyServiceData::operator==(const QLowEnergyServiceData &a,
                                              const QLowEnergyServiceData &b)

   \brief Returns \c true if \a a and \a b are equal with respect to their public state,
   otherwise returns \c false.
 */

/*!
    \fn bool QLowEnergyServiceData::operator!=(const QLowEnergyServiceData &a,
                                               const QLowEnergyServiceData &b)

    \brief Returns \c true if \a a and \a b are unequal with respect to their public state,
    otherwise returns \c false.
 */

/*!
    \brief Returns \c true if \a a and \a b are equal with respect to their public state,
    otherwise returns \c false.
    \internal
 */
bool QLowEnergyServiceData::equals(const QLowEnergyServiceData &a, const QLowEnergyServiceData &b)
{
    return a.d == b.d || (
                a.type() == b.type()
                && a.uuid() == b.uuid()
                && a.includedServices() == b.includedServices()
                && a.characteristics() == b.characteristics());
}

QT_END_NAMESPACE
