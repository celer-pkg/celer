// Copyright (C) 2015 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QGEOCAMERACAPABILITIES_P_H
#define QGEOCAMERACAPABILITIES_P_H

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

#include <QtCore/QObject>
#include <QtLocation/private/qlocationglobal_p.h>

#include <QtCore/QSharedDataPointer>

QT_BEGIN_NAMESPACE

class QGeoCameraCapabilitiesPrivate;
QT_DECLARE_QSDP_SPECIALIZATION_DTOR_WITH_EXPORT(QGeoCameraCapabilitiesPrivate, Q_LOCATION_EXPORT)

class Q_LOCATION_EXPORT QGeoCameraCapabilities
{
    Q_GADGET

    Q_PROPERTY(qreal minimumZoomLevel READ minimumZoomLevelAt256 CONSTANT)
    Q_PROPERTY(qreal maximumZoomLevel READ maximumZoomLevelAt256 CONSTANT)
    Q_PROPERTY(qreal minimumTilt READ minimumTilt CONSTANT)
    Q_PROPERTY(qreal maximumTilt READ maximumTilt CONSTANT)
    Q_PROPERTY(qreal minimumFieldOfView READ minimumFieldOfView CONSTANT)
    Q_PROPERTY(qreal maximumFieldOfView READ maximumFieldOfView CONSTANT)

public:
    QGeoCameraCapabilities();
    QGeoCameraCapabilities(const QGeoCameraCapabilities &other) noexcept;
    QGeoCameraCapabilities(QGeoCameraCapabilities &&other) noexcept = default;
    ~QGeoCameraCapabilities();

    QGeoCameraCapabilities &operator=(const QGeoCameraCapabilities &other) noexcept;
    QT_MOVE_ASSIGNMENT_OPERATOR_IMPL_VIA_MOVE_AND_SWAP(QGeoCameraCapabilities)

    void swap(QGeoCameraCapabilities &other) noexcept { d.swap(other.d); }

    friend inline bool operator==(const QGeoCameraCapabilities &lhs,
                                  const QGeoCameraCapabilities &rhs) noexcept
    {
        return lhs.isEqual(rhs);
    }
    friend inline bool operator!=(const QGeoCameraCapabilities &lhs,
                                  const QGeoCameraCapabilities &rhs) noexcept
    {
        return !lhs.isEqual(rhs);
    }

    void setTileSize(int tileSize);
    int tileSize() const;

    void setMinimumZoomLevel(double minimumZoomLevel);
    double minimumZoomLevel() const;
    double minimumZoomLevelAt256() const;

    void setMaximumZoomLevel(double maximumZoomLevel);
    double maximumZoomLevel() const;
    double maximumZoomLevelAt256() const;

    void setSupportsBearing(bool supportsBearing);
    bool supportsBearing() const;

    void setSupportsRolling(bool supportsRolling);
    bool supportsRolling() const;

    void setSupportsTilting(bool supportsTilting);
    bool supportsTilting() const;

    void setMinimumTilt(double minimumTilt);
    double minimumTilt() const;

    void setMaximumTilt(double maximumTilt);
    double maximumTilt() const;

    void setMinimumFieldOfView(double minimumFieldOfView);
    double minimumFieldOfView() const;

    void setMaximumFieldOfView(double maximumFieldOfView);
    double maximumFieldOfView() const;

    void setOverzoomEnabled(bool overzoomEnabled);
    bool overzoomEnabled() const;

    bool isValid() const;

private:
    QSharedDataPointer<QGeoCameraCapabilitiesPrivate> d;

    bool isEqual(const QGeoCameraCapabilities &other) const;
};

QT_END_NAMESPACE

#endif // QGEOCAMERACAPABILITIES_P_H
