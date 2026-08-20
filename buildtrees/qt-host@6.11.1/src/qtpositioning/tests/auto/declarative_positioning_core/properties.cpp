// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "properties.h"

Properties::Properties(QObject *parent)
    : QObject(parent)
{
}

void Properties::setCoordinate(const QGeoCoordinate &c)
{
    if (m_coordinate == c)
        return;
    m_coordinate = c;
    emit coordinateChanged(m_coordinate);
}

void Properties::setCoordinates(const QList<QGeoCoordinate> &values)
{
    if (m_coordinates == values)
        return;
    m_coordinates = values;
    emit coordinatesChanged(m_coordinates);
}

void Properties::setRectangle(const QGeoRectangle &r)
{
    if (m_rectangle == r)
        return;
    m_rectangle = r;
    emit rectangleChanged(m_rectangle);
}

void Properties::setRegion(const QList<QGeoRectangle> &region)
{
    if (m_region == region)
        return;
    m_region = region;
    emit regionChanged(m_region);
}

#include "moc_properties.cpp"
