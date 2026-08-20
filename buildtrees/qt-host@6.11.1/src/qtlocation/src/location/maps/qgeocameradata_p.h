// Copyright (C) 2015 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default
#ifndef QGEOCAMERADATA_P_H
#define QGEOCAMERADATA_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtLocation/private/qlocationglobal_p.h>

#include <QtCore/QSharedDataPointer>
#include <QtCore/QMetaType>

QT_BEGIN_NAMESPACE

class QGeoCoordinate;
class QGeoCameraDataPrivate;
QT_DECLARE_QSDP_SPECIALIZATION_DTOR_WITH_EXPORT(QGeoCameraDataPrivate, Q_LOCATION_EXPORT)

class Q_LOCATION_EXPORT QGeoCameraData
{
public:
    QGeoCameraData();
    QGeoCameraData(const QGeoCameraData &other) noexcept;
    QGeoCameraData(QGeoCameraData &&other) noexcept = default;
    ~QGeoCameraData();

    QGeoCameraData &operator = (const QGeoCameraData &other) noexcept;
    QT_MOVE_ASSIGNMENT_OPERATOR_IMPL_VIA_MOVE_AND_SWAP(QGeoCameraData)

    void swap(QGeoCameraData &other) noexcept { d.swap(other.d); }

    friend inline bool operator==(const QGeoCameraData &lhs, const QGeoCameraData &rhs) noexcept
    { return lhs.isEqual(rhs); }
    friend inline bool operator!=(const QGeoCameraData &lhs, const QGeoCameraData &rhs) noexcept
    { return !lhs.isEqual(rhs); }

    void setCenter(const QGeoCoordinate &coordinate);
    QGeoCoordinate center() const;

    void setBearing(double bearing);
    double bearing() const;

    void setTilt(double tilt);
    double tilt() const;

    void setRoll(double roll);
    double roll() const;

    void setFieldOfView(double fieldOfView);
    double fieldOfView() const;

    // Zoom level is intended to be relative to a tileSize of 256^2 pixels.
    // E.g., a zoom level of 0 must result in a mapWidth of 256, and so on.
    void setZoomLevel(double zoomLevel);
    double zoomLevel() const;

private:
    QSharedDataPointer<QGeoCameraDataPrivate> d;

    bool isEqual(const QGeoCameraData &other) const;
};

QT_END_NAMESPACE

Q_DECLARE_METATYPE(QGeoCameraData)

#endif // QGEOCAMERADATA_P_H
