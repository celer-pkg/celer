// Copyright (C) 2015 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qgeocameradata_p.h"

#include <QtCore/QVariant>
#include <QtCore/QVariantAnimation>

#include <QtPositioning/private/qgeocoordinate_p.h>
#include <QtPositioning/private/qwebmercator_p.h>

QT_BEGIN_NAMESPACE

class QGeoCameraDataPrivate : public QSharedData
{
public:
    bool operator==(const QGeoCameraDataPrivate &rhs) const noexcept;

    QGeoCoordinate m_center = {0, 0};
    double m_bearing = 0.0;
    double m_tilt = 0.0;
    double m_roll = 0.0;
    double m_fieldOfView = 45.0;
    double m_zoomLevel = 0.0;
};

QT_DEFINE_QSDP_SPECIALIZATION_DTOR(QGeoCameraDataPrivate)

bool QGeoCameraDataPrivate::operator==(const QGeoCameraDataPrivate &rhs) const noexcept
{
    return ((m_center == rhs.m_center)
            && (m_bearing == rhs.m_bearing)
            && (m_tilt == rhs.m_tilt)
            && (m_roll == rhs.m_roll)
            && (m_fieldOfView == rhs.m_fieldOfView)
            && (m_zoomLevel == rhs.m_zoomLevel));
}

QVariant cameraInterpolator(const QGeoCameraData &start,
                            const QGeoCameraData &end,
                            qreal progress)
{
    QGeoCameraData result = start;
    QGeoCoordinate from = start.center();
    QGeoCoordinate to = end.center();

    if (from == to) {
        if (progress < 0.5) {
            result.setCenter(from);
        } else {
            result.setCenter(to);
        }
    }
    else {
        QGeoCoordinate coordinateResult = QWebMercator::coordinateInterpolation(from, to, progress);
        result.setCenter(coordinateResult);
    }

    double sf = 1.0 - progress;
    double ef = progress;

    result.setBearing(sf * start.bearing() + ef * end.bearing());
    result.setTilt(sf * start.tilt() + ef * end.tilt());
    result.setRoll(sf * start.roll() + ef * end.roll());
    result.setFieldOfView(sf * start.fieldOfView() + ef * end.fieldOfView());
    result.setZoomLevel(sf * start.zoomLevel() + ef * end.zoomLevel());

    return QVariant::fromValue(result);
}

QGeoCameraData::QGeoCameraData()
    : d(new QGeoCameraDataPrivate())
{
    qRegisterMetaType<QGeoCameraData>();
    qRegisterAnimationInterpolator<QGeoCameraData>(cameraInterpolator);
}

QGeoCameraData::QGeoCameraData(const QGeoCameraData &other) noexcept = default;

QGeoCameraData::~QGeoCameraData() = default;

QGeoCameraData &QGeoCameraData::operator=(const QGeoCameraData &other) noexcept
{
    if (this == &other)
        return *this;

    d = other.d;
    return *this;
}

bool QGeoCameraData::isEqual(const QGeoCameraData &other) const
{
    return (*(d.constData()) == *(other.d.constData()));
}

void QGeoCameraData::setCenter(const QGeoCoordinate &center)
{
    d->m_center = center;
}

QGeoCoordinate QGeoCameraData::center() const
{
    return d->m_center;
}

void QGeoCameraData::setBearing(double bearing)
{
    d->m_bearing = bearing;
}

double QGeoCameraData::bearing() const
{
    return d->m_bearing;
}

void QGeoCameraData::setTilt(double tilt)
{
    d->m_tilt = tilt;
}

double QGeoCameraData::tilt() const
{
    return d->m_tilt;
}

void QGeoCameraData::setRoll(double roll)
{
    d->m_roll = roll;
}

double QGeoCameraData::roll() const
{
    return d->m_roll;
}

void QGeoCameraData::setFieldOfView(double fieldOfView)
{
    d->m_fieldOfView = fieldOfView;
}

double QGeoCameraData::fieldOfView() const
{
    return d->m_fieldOfView;
}

void QGeoCameraData::setZoomLevel(double zoomFactor)
{
    d->m_zoomLevel = zoomFactor;
}

double QGeoCameraData::zoomLevel() const
{
    return d->m_zoomLevel;
}

QT_END_NAMESPACE
