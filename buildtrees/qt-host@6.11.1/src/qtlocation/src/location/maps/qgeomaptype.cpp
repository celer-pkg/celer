// Copyright (C) 2015 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qgeomaptype_p.h"
#include "qgeomaptype_p_p.h"

QT_BEGIN_NAMESPACE

QT_DEFINE_QSDP_SPECIALIZATION_DTOR(QGeoMapTypePrivate)

/*!
    \qmltype mapType
    \inqmlmodule QtLocation
    \ingroup qml-QtLocation5-maps
    \since QtLocation 5.5

    \brief The mapType type holds information about a map type.

    This includes the map type's \l name and \l description, the \l style and
    a flag to indicate if the map type is optimized for mobile devices (\l mobile).
*/

QGeoMapType::QGeoMapType()
    : d_ptr(new QGeoMapTypePrivate()) {}

QGeoMapType::QGeoMapType(const QGeoMapType &other) noexcept = default;

QGeoMapType::QGeoMapType(QGeoMapType::MapStyle style, const QString &name,
                         const QString &description, bool mobile, bool night, int mapId,
                         const QByteArray &pluginName,
                         const QGeoCameraCapabilities &cameraCapabilities,
                         const QVariantMap &metadata)
:   d_ptr(new QGeoMapTypePrivate(style, name, description, mobile, night, mapId, pluginName, cameraCapabilities, metadata))
{
}

QGeoMapType::~QGeoMapType() = default;

QGeoMapType &QGeoMapType::operator=(const QGeoMapType &other) noexcept
{
    if (this == &other)
        return *this;

    d_ptr = other.d_ptr;
    return *this;
}

bool QGeoMapType::isEqual(const QGeoMapType &other) const noexcept
{
    return (*d_ptr.constData() == *other.d_ptr.constData());
}

/*!
    \qmlproperty enumeration mapType::style

    This read-only property gives access to the style of the map type.

    \list
    \li MapType.NoMap - No map.
    \li MapType.StreetMap - A street map.
    \li MapType.SatelliteMapDay - A map with day-time satellite imagery.
    \li MapType.SatelliteMapNight - A map with night-time satellite imagery.
    \li MapType.TerrainMap - A terrain map.
    \li MapType.HybridMap - A map with satellite imagery and street information.
    \li MapType.GrayStreetMap - A gray-shaded street map.
    \li MapType.PedestrianMap - A street map suitable for pedestriants.
    \li MapType.CarNavigationMap - A street map suitable for car navigation.
    \li MapType.CycleMap - A street map suitable for cyclists.
    \li MapType.CustomMap - A custom map type.
    \endlist
*/

QGeoMapType::MapStyle QGeoMapType::style() const
{
    return d_ptr->style_;
}

/*!
    \qmlproperty string mapType::name

    This read-only property holds the name of the map type as a single formatted string.
*/

QString QGeoMapType::name() const
{
    return d_ptr->name_;
}

/*!
    \qmlproperty string mapType::description

    This read-only property holds the description of the map type as a single formatted string.
*/

QString QGeoMapType::description() const
{
    return d_ptr->description_;
}

/*!
    \qmlproperty bool mapType::mobile

    \brief Whether the map type is optimized for the use on a mobile device.

    Map types for mobile devices usually have higher constrast to counteract the
    effects of sunlight and a reduced color for improved readability.
*/
bool QGeoMapType::mobile() const
{
    return d_ptr->mobile_;
}

/*!
    \qmlproperty bool mapType::night
    \since QtLocation 5.4

    \brief Whether the map type is optimized for use at night.

    Map types suitable for use at night usually have a dark background.
*/
bool QGeoMapType::night() const
{
    return d_ptr->night_;
}

int QGeoMapType::mapId() const
{
    return d_ptr->mapId_;
}

QByteArray QGeoMapType::pluginName() const
{
    return d_ptr->pluginName_;
}

/*!
    \qmlproperty CameraCapabilities mapType::cameraCapabilities
    \since QtLocation 5.10

    This property holds the camera capabilities for this map type.
*/

QGeoCameraCapabilities QGeoMapType::cameraCapabilities() const
{
    return d_ptr->cameraCapabilities_;
}

/*!
    \qmlproperty VariantMap mapType::metadata
    \since QtLocation 5.10

    This property holds optional, extra metadata related to a specific map type.
    The content of this property is entirely plugin-specific.
*/
QVariantMap QGeoMapType::metadata() const
{
    return d_ptr->metadata_;
}

QGeoMapTypePrivate::QGeoMapTypePrivate() = default;

QGeoMapTypePrivate::QGeoMapTypePrivate(QGeoMapType::MapStyle style, const QString &name,
                                       const QString &description, bool mobile, bool night,
                                       int mapId, const QByteArray &pluginName,
                                       const QGeoCameraCapabilities &cameraCapabilities,
                                       const QVariantMap &metadata)
:   name_(name), description_(description), pluginName_(pluginName),
    cameraCapabilities_(cameraCapabilities), metadata_(metadata),
    style_(style), mapId_(mapId), mobile_(mobile), night_(night)
{
}

bool QGeoMapTypePrivate::operator==(const QGeoMapTypePrivate &other) const
{
    return pluginName_ == other.pluginName_ && style_ == other.style_ && name_ == other.name_ &&
           description_ == other.description_ && mobile_ == other.mobile_ && night_ == other.night_ &&
           mapId_ == other.mapId_ && cameraCapabilities_ == other.cameraCapabilities_ &&
           metadata_ == other.metadata_;
}

QT_END_NAMESPACE

#include "moc_qgeomaptype_p.cpp"
