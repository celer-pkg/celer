// Copyright (C) 2015 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QGEOTILESPEC_H
#define QGEOTILESPEC_H

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
#include <QtCore/QMetaType>
#include <QtCore/QList>
#include <QtCore/QSet>
#include <QString>

#include <QSharedDataPointer>

QT_BEGIN_NAMESPACE

class QGeoTileSpecPrivate;
QT_DECLARE_QSDP_SPECIALIZATION_DTOR_WITH_EXPORT(QGeoTileSpecPrivate, Q_LOCATION_EXPORT)

class Q_LOCATION_EXPORT QGeoTileSpec
{
public:
    QGeoTileSpec();
    QGeoTileSpec(const QGeoTileSpec &other) noexcept;
    QGeoTileSpec(QGeoTileSpec &&other) noexcept = default;
    QGeoTileSpec(const QString &plugin, int mapId, int zoom, int x, int y, int version = -1);
    ~QGeoTileSpec();

    QGeoTileSpec &operator=(const QGeoTileSpec &other) noexcept;
    QT_MOVE_ASSIGNMENT_OPERATOR_IMPL_VIA_MOVE_AND_SWAP(QGeoTileSpec)

    void swap(QGeoTileSpec &other) noexcept { d.swap(other.d); }

    QString plugin() const;

    void setZoom(int zoom);
    int zoom() const;

    void setX(int x);
    int x() const;

    void setY(int y);
    int y() const;

    void setMapId(int mapId);
    int mapId() const;

    void setVersion(int version);
    int version() const;

    friend inline bool operator==(const QGeoTileSpec &lhs, const QGeoTileSpec &rhs) noexcept
    { return lhs.isEqual(rhs); }
    friend inline bool operator!=(const QGeoTileSpec &lhs, const QGeoTileSpec &rhs) noexcept
    { return !lhs.isEqual(rhs); }
    friend inline bool operator < (const QGeoTileSpec &lhs, const QGeoTileSpec &rhs) noexcept
    { return lhs.isLess(rhs); }

private:
    QSharedDataPointer<QGeoTileSpecPrivate> d;

    bool isEqual(const QGeoTileSpec &rhs) const noexcept;
    bool isLess(const QGeoTileSpec &rhs) const noexcept;
    size_t hash(size_t seed) const noexcept;
    friend size_t qHash(const QGeoTileSpec &key, size_t seed = 0) noexcept
    { return key.hash(seed); }
};


Q_LOCATION_EXPORT QDebug operator<<(QDebug, const QGeoTileSpec &);

QT_END_NAMESPACE

Q_DECLARE_METATYPE(QGeoTileSpec)
Q_DECLARE_METATYPE(QList<QGeoTileSpec>)
Q_DECLARE_METATYPE(QSet<QGeoTileSpec>)

#endif // QGEOTILESPEC_H
