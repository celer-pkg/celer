// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qgeocoordinateobject_p.h"

QT_BEGIN_NAMESPACE

QT_IMPL_METATYPE_EXTERN_TAGGED(QGeoCoordinateObject*, QGeoCoordinateObject_ptr)

/*

  Note: This class only purpose is to enable conversion between QGeoCoordinate and QDeclarativeGeoWaypoint.
  Since QGeoCoordinate lives in the QtPositioning module, this class acts as a base for QDeclarativeGeoWaypoint,
  and contains the bare minimum to convert/compare to a QGeoCoordinate

*/

QGeoCoordinateObject::QGeoCoordinateObject(QObject *parent) : QObject(parent)
{
}

QGeoCoordinateObject::QGeoCoordinateObject(const QGeoCoordinate &c, QObject *parent) : QObject(parent)
{
    setCoordinate(c);
}

QGeoCoordinateObject::~QGeoCoordinateObject()
{

}

bool QGeoCoordinateObject::operator==(const QGeoCoordinateObject &other) const
{
    return m_coordinate.value() == other.m_coordinate.value();
}

bool QGeoCoordinateObject::operator==(const QGeoCoordinate &other) const
{
    return m_coordinate.value() == other;
}

QGeoCoordinate QGeoCoordinateObject::coordinate() const
{
    return m_coordinate;
}

void QGeoCoordinateObject::setCoordinate(const QGeoCoordinate &c)
{
    m_coordinate = c; // The signal is emitted automatically if needed
}

QBindable<QGeoCoordinate> QGeoCoordinateObject::bindableCoordinate()
{
    return QBindable<QGeoCoordinate>(&m_coordinate);
}

QT_END_NAMESPACE

#include "moc_qgeocoordinateobject_p.cpp"

