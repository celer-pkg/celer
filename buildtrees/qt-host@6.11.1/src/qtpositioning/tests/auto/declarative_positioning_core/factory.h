// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
#ifndef FACTORY_H
#define FACTORY_H

#include <QObject>
#include <QGeoShape>
#include <QGeoCoordinate>
#include <qqml.h>

QT_BEGIN_NAMESPACE

class QGeoShape;
class QGeoCoordinate;

class Factory : public QObject
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit Factory(QObject *parent = nullptr);

    Q_INVOKABLE QGeoShape createShape(const QGeoCoordinate &topLeft, const QGeoCoordinate &bottomRight);
    Q_INVOKABLE QGeoShape createShape(const QGeoCoordinate &center, qreal radius) const;
};

QT_END_NAMESPACE

#endif // FACTORY_H
