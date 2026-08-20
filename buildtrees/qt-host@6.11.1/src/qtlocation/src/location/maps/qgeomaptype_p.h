// Copyright (C) 2015 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QGEOMAPTYPE_H
#define QGEOMAPTYPE_H

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
#include <QtCore/QString>
#include <QtCore/QSharedDataPointer>
#include <QtCore/QVariantMap>
#include <QtLocation/private/qlocationglobal_p.h>
#include <QtQml/qqml.h>

QT_BEGIN_NAMESPACE

class QGeoCameraCapabilities;
class QGeoMapTypePrivate;
QT_DECLARE_QSDP_SPECIALIZATION_DTOR_WITH_EXPORT(QGeoMapTypePrivate, Q_LOCATION_EXPORT)

class Q_LOCATION_EXPORT QGeoMapType
{
    Q_GADGET
    QML_VALUE_TYPE(mapType)
    QML_STRUCTURED_VALUE

    Q_PROPERTY(MapStyle style READ style CONSTANT)
    Q_PROPERTY(QString name READ name CONSTANT)
    Q_PROPERTY(QString description READ description CONSTANT)
    Q_PROPERTY(bool mobile READ mobile CONSTANT)
    Q_PROPERTY(bool night READ night CONSTANT)
    Q_PROPERTY(QGeoCameraCapabilities cameraCapabilities READ cameraCapabilities CONSTANT)
    Q_PROPERTY(QVariantMap metadata READ metadata CONSTANT)

    Q_MOC_INCLUDE(<QtLocation/private/qgeocameracapabilities_p.h>)
public:
    enum MapStyle { // ### Qt6: change this to be a QFlags instead, or remove.
        NoMap = 0,
        StreetMap,
        SatelliteMapDay,
        SatelliteMapNight,
        TerrainMap,
        HybridMap,
        TransitMap,
        GrayStreetMap,
        PedestrianMap,
        CarNavigationMap,
        CycleMap,
        CustomMap = 100
    };
    Q_ENUM(MapStyle)

    QGeoMapType();
    QGeoMapType(const QGeoMapType &other) noexcept;
    QGeoMapType(QGeoMapType &&other) noexcept = default;
    QGeoMapType(MapStyle style, const QString &name, const QString &description, bool mobile,
                bool night, int mapId, const QByteArray &pluginName,
                const QGeoCameraCapabilities &cameraCapabilities,
                const QVariantMap &metadata = QVariantMap());
    ~QGeoMapType();

    QGeoMapType &operator=(const QGeoMapType &other) noexcept;
    QT_MOVE_ASSIGNMENT_OPERATOR_IMPL_VIA_MOVE_AND_SWAP(QGeoMapType)

    void swap(QGeoMapType &other) noexcept { d_ptr.swap(other.d_ptr); }

    friend inline bool operator==(const QGeoMapType &lhs, const QGeoMapType &rhs) noexcept
    { return lhs.isEqual(rhs); }
    friend inline bool operator!=(const QGeoMapType &lhs, const QGeoMapType &rhs) noexcept
    { return !lhs.isEqual(rhs); }

    MapStyle style() const;
    QString name() const;
    QString description() const;
    bool mobile() const;
    bool night() const;
    int mapId() const;
    QByteArray pluginName() const;
    QGeoCameraCapabilities cameraCapabilities() const;
    QVariantMap metadata() const;

private:
    QSharedDataPointer<QGeoMapTypePrivate> d_ptr;

    bool isEqual(const QGeoMapType &other) const noexcept;
};

namespace QGeoMapTypeForeignNamespace
{
    Q_NAMESPACE
    QML_FOREIGN_NAMESPACE(QGeoMapType)
    QML_NAMED_ELEMENT(MapType)
};

QT_END_NAMESPACE

#endif // QGEOMAPTYPE_H
