// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef PROPERTIES_H
#define PROPERTIES_H

#include <QObject>
#include <QList>
#include <QGeoCoordinate>
#include <QGeoRectangle>
#include <qqml.h>

QT_BEGIN_NAMESPACE

class Properties : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QGeoCoordinate coordinate READ coordinate WRITE setCoordinate NOTIFY coordinateChanged)
    Q_PROPERTY(QList<QGeoCoordinate> coordinates READ coordinates WRITE setCoordinates NOTIFY coordinatesChanged)
    Q_PROPERTY(QGeoRectangle rectangle READ rectangle WRITE setRectangle NOTIFY rectangleChanged)
    Q_PROPERTY(QList<QGeoRectangle> region READ region WRITE setRegion NOTIFY regionChanged)
    QML_ELEMENT

public:
    explicit Properties(QObject *parent = nullptr);

    QGeoCoordinate coordinate() const { return m_coordinate; }
    void setCoordinate(const QGeoCoordinate &c);

    QList<QGeoCoordinate> coordinates() const { return m_coordinates; }
    void setCoordinates(const QList<QGeoCoordinate> &values);

    QGeoRectangle rectangle() const { return m_rectangle; }
    void setRectangle(const QGeoRectangle &r);

    QList<QGeoRectangle> region() const { return m_region; }
    void setRegion(const QList<QGeoRectangle> &region);

signals:
    void coordinateChanged(const QGeoCoordinate &);
    void coordinatesChanged(const QList<QGeoCoordinate> &);
    void rectangleChanged(const QGeoRectangle &);
    void regionChanged(const QList<QGeoRectangle> &);

private:
    QGeoCoordinate m_coordinate;
    QList<QGeoCoordinate> m_coordinates;
    QGeoRectangle m_rectangle;
    QList<QGeoRectangle> m_region;
};

QT_END_NAMESPACE

#endif // FACTORY_H
