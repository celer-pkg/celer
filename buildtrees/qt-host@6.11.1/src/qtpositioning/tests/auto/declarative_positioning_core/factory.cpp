// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "factory.h"
#include <QGeoRectangle>
#include <QGeoCircle>

QT_BEGIN_NAMESPACE

Factory::Factory(QObject *parent) : QObject(parent)
{

}

QGeoShape Factory::createShape(const QGeoCoordinate &topLeft, const QGeoCoordinate &bottomRight)
{
    return QGeoRectangle(topLeft, bottomRight);
}

QGeoShape Factory::createShape(const QGeoCoordinate &center, qreal radius) const
{
    return QGeoCircle(center, radius);
}

QT_END_NAMESPACE
