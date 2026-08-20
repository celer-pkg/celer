// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qdeclarativegeolocation_p.h"

QT_BEGIN_NAMESPACE

/*!
    \qmltype Location
    \inqmlmodule QtPositioning
    \since 5.2

    \brief The Location type holds location data.

    Location types represent a geographic "location", in a human sense. This
    consists of a specific \l {coordinate}, an \l {address} and a
    \l {boundingShape}{bounding shape}.
    The \l {boundingShape}{bounding shape} represents the recommended region
    to display when viewing this location.

    The Location type is most commonly seen as the contents of a search
    model such as the GeocodeModel. When a GeocodeModel returns the list of
    locations found for a given query, it represents these as Location objects.

    \section2 Example Usage

    The following example shows a simple Location object being declared:

    \code
    Location {
        coordinate {
            latitude: -27.3
            longitude: 153.1
        }
        address: Address {
            ...
        }
    }
    \endcode
*/

/*!
    \qmlproperty VariantMap QtPositioning::Location::extendedAttributes

    This property holds the extended attributes for this Location.
    Extended attributes are backend-dependent and can be location-dependent.

    \since 5.13
*/

QDeclarativeGeoLocation::QDeclarativeGeoLocation(QObject *parent)
:   QObject(parent)

{
    setLocationInternal(QGeoLocation());
}

QDeclarativeGeoLocation::QDeclarativeGeoLocation(const QGeoLocation &src, QObject *parent)
:   QObject(parent)
{
    setLocationInternal(src);
}

QDeclarativeGeoLocation::~QDeclarativeGeoLocation()
{
}

/*!
    \internal

    This method is supposed to be used in the constructors, when we know that
    the properties have no bindings, and we do not want to introduce any.
*/
void QDeclarativeGeoLocation::setLocationInternal(const QGeoLocation &src)
{
    m_address.setValueBypassingBindings(new QDeclarativeGeoAddress(src.address(), this));
    m_coordinate.setValueBypassingBindings(src.coordinate());
    m_boundingShape.setValueBypassingBindings(src.boundingShape());
    m_extendedAttributes.setValueBypassingBindings(src.extendedAttributes());
}

/*!
    \qmlproperty QGeoLocation QtPositioning::Location::location

    For details on how to use this property to interface between C++ and QML see
    "\l {Location - QGeoLocation} {Interfaces between C++ and QML Code}".

    \note This property updates the whole geo location information, so using it
    will result in breaking of all the bindings for all other properties.
*/
void QDeclarativeGeoLocation::setLocation(const QGeoLocation &src)
{
    if (m_address && m_address->parent() == this) {
        m_address->setAddress(src.address());
    } else if (!m_address || m_address->parent() != this) {
        m_address.setValue(new QDeclarativeGeoAddress(src.address(), this));
        m_address.notify();
    }

    setCoordinate(src.coordinate());
    setBoundingShape(src.boundingShape());
    setExtendedAttributes(src.extendedAttributes());
}

QGeoLocation QDeclarativeGeoLocation::location() const
{
    QGeoLocation retValue;
    retValue.setAddress(m_address ? m_address->address() : QGeoAddress());
    retValue.setCoordinate(m_coordinate);
    retValue.setBoundingShape(m_boundingShape);
    retValue.setExtendedAttributes(m_extendedAttributes);
    return retValue;
}

/*!
    \qmlproperty Address QtPositioning::Location::address

    This property holds the address of the location which can be use to retrieve address details of the location.
*/
void QDeclarativeGeoLocation::setAddress(QDeclarativeGeoAddress *address)
{
    m_address.removeBindingUnlessInWrapper();

    const QDeclarativeGeoAddress *oldAddress = m_address.valueBypassingBindings();
    if (oldAddress == address)
        return;

    // implicitly deleting m_address.value() will force the QML bindings to
    // be reevaluated by the QML engine. So we defer the deletion of the old
    // address until a new value is assigned to the m_address.
    m_address.setValueBypassingBindings(address);
    m_address.notify();
    if (oldAddress && (oldAddress->parent() == this))
        delete oldAddress;
}

QBindable<QDeclarativeGeoAddress *> QDeclarativeGeoLocation::bindableAddress()
{
    return QBindable<QDeclarativeGeoAddress *>(&m_address);
}

QDeclarativeGeoAddress *QDeclarativeGeoLocation::address() const
{
    return m_address;
}

/*!
    \qmlproperty coordinate QtPositioning::Location::coordinate

    This property holds the exact geographical coordinate of the location which can be used to retrieve the latitude, longitude and altitude of the location.
*/
void QDeclarativeGeoLocation::setCoordinate(const QGeoCoordinate coordinate)
{
    m_coordinate = coordinate;
}

QBindable<QGeoCoordinate> QDeclarativeGeoLocation::bindableCoordinate()
{
    return QBindable<QGeoCoordinate>(&m_coordinate);
}

QGeoCoordinate QDeclarativeGeoLocation::coordinate() const
{
    return m_coordinate;
}

/*!
    \since QtPositioning 6.2

    \qmlproperty geoShape QtPositioning::Location::boundingShape

    This property holds the recommended region to use when displaying the location.
    For example, a building's location may have a region centered around the building,
    but the region is large enough to show it's immediate surrounding geographical
    context.

    \note This property was introduced in Qt6 instead of boundingBox property.
    It returns a \l geoShape instead of a \l geoRectangle.
    Use \l QGeoShape::boundingGeoRectangle() to obtain a bounding
    \l geoRectangle for the shape.

    If you need to convert the returned shape to a specific type, use the
    \c type property of \l geoShape together with convenience
    methods from \l [QML]{QtPositioning} like
    \l {QtPositioning::shapeToRectangle}{QtPositioning.shapeToRectangle()}.
*/
void QDeclarativeGeoLocation::setBoundingShape(const QGeoShape &boundingShape)
{
    m_boundingShape = boundingShape;
}

QBindable<QGeoShape> QDeclarativeGeoLocation::bindableBoundingShape()
{
    return QBindable<QGeoShape>(&m_boundingShape);
}

QVariantMap QDeclarativeGeoLocation::extendedAttributes() const
{
    return m_extendedAttributes;
}

void QDeclarativeGeoLocation::setExtendedAttributes(const QVariantMap &attributes)
{
    m_extendedAttributes = attributes;
}

QBindable<QVariantMap> QDeclarativeGeoLocation::bindableExtendedAttributes()
{
    return QBindable<QVariantMap>(&m_extendedAttributes);
}

QGeoShape QDeclarativeGeoLocation::boundingShape() const
{
    return m_boundingShape;
}

QT_END_NAMESPACE

#include "moc_qdeclarativegeolocation_p.cpp"
