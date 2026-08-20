// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qdeclarativegeoaddress_p.h"

QT_BEGIN_NAMESPACE

/*!
    \qmltype Address
    \inqmlmodule QtPositioning
    \since 5.2

    \brief The Address QML type represents a specific location as a street address.

    An Address is used as a unit of data for queries such as (Reverse) Geocoding
    or Places searches -- many of these operations either accept an Address
    or return one.

    Not all properties of an Address are necessarily available or relevant
    in all parts of the world and all locales. The \l district, \l state and
    \l county properties are particularly area-specific for many data sources,
    and often only one or two of these are available or useful.

    The Address has a \l text property which holds a formatted string.  It
    is the recommended way to display an address to the user and typically
    takes the format of an address as found on an envelope, but this is not always
    the case.  The \l text may be automatically generated from constituent
    address properties such as \l street, \l city and and so on, but can also
    be explicitly assigned.  See \l text for details.

    \section2 Example Usage

    The following code snippet shows the declaration of an Address object.

    \code
    Address {
        id: address
        street: "53 Brandl St"
        city: "Eight Mile Plains"
        country: "Australia"
        countryCode: "AUS"
    }
    \endcode

    This could then be used, for example, as the value of a geocoding query,
    to get an exact longitude and latitude for the address.

    \sa {QGeoAddress}
*/

QDeclarativeGeoAddress::QDeclarativeGeoAddress(QObject *parent) :
        QObject(parent)
{
}

QDeclarativeGeoAddress::QDeclarativeGeoAddress(const QGeoAddress &address, QObject *parent) :
        QObject(parent), m_address(address)
{
}

/*!
    \qmlproperty QGeoAddress QtPositioning::Address::address

    For details on how to use this property to interface between C++ and QML see
    "\l {Address - QGeoAddress} {Interfaces between C++ and QML Code}".
*/
QGeoAddress QDeclarativeGeoAddress::address() const
{
    return m_address;
}

void QDeclarativeGeoAddress::setAddress(const QGeoAddress &address)
{
    // Elaborate but takes care of emiting needed signals
    setText(address.text());
    setCountry(address.country());
    setCountryCode(address.countryCode());
    setState(address.state());
    setCounty(address.county());
    setCity(address.city());
    setDistrict(address.district());
    setStreet(address.street());
    setStreetNumber(address.streetNumber());
    setPostalCode(address.postalCode());
    m_address = address;
}

/*!
    \qmlproperty string QtPositioning::Address::text

    This property holds the address as a single formatted string. It is the recommended
    string to use to display the address to the user. It typically takes the format of
    an address as found on an envelope, but this is not always necessarily the case.

    The address \c text is either automatically generated or explicitly assigned,
    this can be determined by checking \l isTextGenerated.

    If an empty string is assigned to \c text, then \l isTextGenerated will be set
    to true and \c text will return a string which is locally formatted according to
    \l countryCode and based on the properties of the address. Modifying the address
    properties such as \l street, \l city and so on may cause the contents of \c text to
    change.

    If a non-empty string is assigned to \c text, then \l isTextGenerated will be
    set to false and \c text will always return the explicitly assigned string.
    Modifying address properties will not affect the \c text property.
*/
QString QDeclarativeGeoAddress::text() const
{
    return m_address.text();
}

void QDeclarativeGeoAddress::setText(const QString &address)
{
    QString oldText = m_address.text();
    bool oldIsTextGenerated = m_address.isTextGenerated();
    m_address.setText(address);

    if (oldText != m_address.text())
        emit textChanged();
    if (oldIsTextGenerated != m_address.isTextGenerated())
        emit isTextGeneratedChanged();
}

/*!
  \qmlproperty string QtPositioning::Address::country

  This property holds the country of the address as a single formatted string.
*/
QString QDeclarativeGeoAddress::country() const
{
    return m_address.country();
}

void QDeclarativeGeoAddress::setCountry(const QString &country)
{
    if (m_address.country() == country)
        return;
    QString oldText = m_address.text();
    m_address.setCountry(country);
    emit countryChanged();

    if (m_address.isTextGenerated() && oldText != m_address.text())
        emit textChanged();
}

/*!
  \qmlproperty string QtPositioning::Address::countryCode

  This property holds the country code of the address as a single formatted string.
*/
QString QDeclarativeGeoAddress::countryCode() const
{
    return m_address.countryCode();
}

void QDeclarativeGeoAddress::setCountryCode(const QString &countryCode)
{
    if (m_address.countryCode() == countryCode)
        return;
    QString oldText = m_address.text();
    m_address.setCountryCode(countryCode);
    emit countryCodeChanged();

    if (m_address.isTextGenerated() && oldText != m_address.text())
        emit textChanged();
}

/*!
  \qmlproperty string QtPositioning::Address::state

  This property holds the state of the address as a single formatted string.
*/
QString QDeclarativeGeoAddress::state() const
{
    return m_address.state();
}

void QDeclarativeGeoAddress::setState(const QString &state)
{
    if (m_address.state() == state)
        return;
    QString oldText = m_address.text();
    m_address.setState(state);
    emit stateChanged();

    if (m_address.isTextGenerated() && oldText != m_address.text())
        emit textChanged();
}

/*!
  \qmlproperty string QtPositioning::Address::county

  This property holds the county of the address as a single formatted string.
*/
QString QDeclarativeGeoAddress::county() const
{
    return m_address.county();
}

void QDeclarativeGeoAddress::setCounty(const QString &county)
{
    if (m_address.county() == county)
        return;
    QString oldText = m_address.text();
    m_address.setCounty(county);
    emit countyChanged();

    if (m_address.isTextGenerated() && oldText != m_address.text())
        emit textChanged();
}

/*!
  \qmlproperty string QtPositioning::Address::city

  This property holds the city of the address as a single formatted string.
*/
QString QDeclarativeGeoAddress::city() const
{
    return m_address.city();
}

void QDeclarativeGeoAddress::setCity(const QString &city)
{
    if (m_address.city() == city)
        return;
    QString oldText = m_address.text();
    m_address.setCity(city);
    emit cityChanged();

    if (m_address.isTextGenerated() && oldText != m_address.text())
        emit textChanged();
}

/*!
  \qmlproperty string QtPositioning::Address::district

  This property holds the district of the address as a single formatted string.
*/
QString QDeclarativeGeoAddress::district() const
{
    return m_address.district();
}

void QDeclarativeGeoAddress::setDistrict(const QString &district)
{
    if (m_address.district() == district)
        return;
    QString oldText = m_address.text();
    m_address.setDistrict(district);
    emit districtChanged();

    if (m_address.isTextGenerated() && oldText != m_address.text())
        emit textChanged();
}

/*!
  \qmlproperty string QtPositioning::Address::street

  This property holds the street of the address.

  \note Before Qt6 this property could also contain things like a unit number,
  a building name, or anything else that might be used to distinguish one
  address from another. Since Qt6 use \l{QtPositioning::Address::}{streetNumber}
  property for such information.
*/
QString QDeclarativeGeoAddress::street() const
{
    return m_address.street();
}

void QDeclarativeGeoAddress::setStreet(const QString &street)
{
    if (m_address.street() == street)
        return;
    QString oldText = m_address.text();
    m_address.setStreet(street);
    emit streetChanged();

    if (m_address.isTextGenerated() && oldText != m_address.text())
        emit textChanged();
}

/*!
    \qmlproperty string QtPositioning::Address::streetNumber
    \since QtPositioning 6.2

    This property holds the street number of the address like a unit number,
    a building name, or anything else that might be used to distinguish one
    address from another.
*/
QString QDeclarativeGeoAddress::streetNumber() const
{
    return m_address.streetNumber();
}

void QDeclarativeGeoAddress::setStreetNumber(const QString &streetNumber)
{
    if (m_address.streetNumber() == streetNumber)
        return;
    QString oldText = m_address.text();
    m_address.setStreetNumber(streetNumber);
    emit streetNumberChanged();

    if (m_address.isTextGenerated() && oldText != m_address.text())
        emit textChanged();
}

/*!
  \qmlproperty string QtPositioning::Address::postalCode

  This property holds the postal code of the address as a single formatted string.
*/
QString QDeclarativeGeoAddress::postalCode() const
{
    return m_address.postalCode();
}

void QDeclarativeGeoAddress::setPostalCode(const QString &postalCode)
{
    if (m_address.postalCode() == postalCode)
        return;
    QString oldText = m_address.text();
    m_address.setPostalCode(postalCode);
    emit postalCodeChanged();

    if (m_address.isTextGenerated() && oldText != m_address.text())
        emit textChanged();
}

/*!
  \qmlproperty bool QtPositioning::Address::isTextGenerated

  This property holds a boolean that if true, indicates that \l text is automatically
  generated from address properties.  If false, it indicates that the \l text has been
  explicitly assigned.

*/
bool QDeclarativeGeoAddress::isTextGenerated() const
{
    return m_address.isTextGenerated();
}

QT_END_NAMESPACE

#include "moc_qdeclarativegeoaddress_p.cpp"
