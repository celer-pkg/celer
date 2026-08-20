// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qgeocameracapabilities_p.h"

#include <QSharedData>
#include <cmath>

static double zoomLevelTo256(double zoomLevelForTileSize, double tileSize)
{
    return std::log(std::pow(2.0, zoomLevelForTileSize) * tileSize / 256.0) * (1.0 / std::log(2.0));
}

QT_BEGIN_NAMESPACE

class QGeoCameraCapabilitiesPrivate : public QSharedData
{
public:
    bool operator==(const QGeoCameraCapabilitiesPrivate &rhs) const noexcept;

    bool supportsBearing_ = false;
    bool supportsRolling_ = false;
    bool supportsTilting_ = false;

    // this is mutable so that it can be set from accessor functions that are const
    // TODO: remove the mutable here
    mutable bool valid_ = false;

    double minZoom_ = 0.0;
    double maxZoom_ = 0.0;
    double minTilt_ = 0.0;
    double maxTilt_ = 0.0;
    int tileSize_ = 256;
    double minimumFieldOfView_ = 45.0; // Defaulting to a fixed FOV of 45 degrees.
    double maximumFieldOfView_ = 45.0; // Too large FOVs cause the loading of too many tiles.
    bool overzoomEnabled_ = false;
};

QT_DEFINE_QSDP_SPECIALIZATION_DTOR(QGeoCameraCapabilitiesPrivate)

bool QGeoCameraCapabilitiesPrivate::operator==(const QGeoCameraCapabilitiesPrivate &rhs) const noexcept
{
    return ((supportsBearing_ == rhs.supportsBearing_)
            && (supportsRolling_ == rhs.supportsRolling_)
            && (supportsTilting_ == rhs.supportsTilting_)
            && (valid_ == rhs.valid_)
            && (minZoom_ == rhs.minZoom_)
            && (maxZoom_ == rhs.maxZoom_)
            && (minTilt_ == rhs.minTilt_)
            && (maxTilt_ == rhs.maxTilt_)
            && (tileSize_ == rhs.tileSize_)
            && (minimumFieldOfView_ == rhs.minimumFieldOfView_)
            && (maximumFieldOfView_ == rhs.maximumFieldOfView_)
            && (overzoomEnabled_ == rhs.overzoomEnabled_));
}

/*!
    \class QGeoCameraCapabilities
    \inmodule QtLocation
    \ingroup QtLocation-impl
    \since 5.6
    \internal

    \brief The QGeoCameraCapabilities class describes the limitations on camera settings imposed by a mapping plugin.

    Different mapping plugins will support different ranges of zoom levels, and not all mapping plugins will
    be able to support, bearing, tilting and rolling of the camera.

    This class describes what the plugin supports, and is used to restrict changes to the camera information
    associated with a \l QGeoMap such that the camera information stays within these limits.
*/

/*!
    \qmlvaluetype cameraCapabilities
    \inqmlmodule QtLocation
    \ingroup qml-QtLocation5-maps
    \since QtLocation 5.10

    \brief The cameraCapabilities type holds information about the camera capabilities for a specific map type.

    This includes the map minimum and maximum zoom level, minimum and maximum tilt angle and
    minimum and maximum field of view.
*/

/*!
    Constructs a camera capabilities object.
*/
QGeoCameraCapabilities::QGeoCameraCapabilities()
    : d(new QGeoCameraCapabilitiesPrivate()) {}

/*!
    Constructs a camera capabilities object from the contents of \a other.
*/
QGeoCameraCapabilities::QGeoCameraCapabilities(const QGeoCameraCapabilities &other) noexcept = default;

/*!
    Destroys this camera capabilities object.
*/
QGeoCameraCapabilities::~QGeoCameraCapabilities() = default;

/*!
    Assigns the contents of \a other to this camera capabilities object and
    returns a reference to this camera capabilities object.
*/
QGeoCameraCapabilities &QGeoCameraCapabilities::operator=(const QGeoCameraCapabilities &other) noexcept
{
    if (this == &other)
        return *this;

    d = other.d;
    return *this;
}

bool QGeoCameraCapabilities::isEqual(const QGeoCameraCapabilities &other) const
{
    return (*(d.constData()) == *(other.d.constData()));
}

void QGeoCameraCapabilities::setTileSize(int tileSize)
{
    if (tileSize < 1)
        return;
    d->tileSize_ = tileSize;
}

int QGeoCameraCapabilities::tileSize() const
{
    return d->tileSize_;
}

/*!
    Returns whether this instance of the class is considered "valid". To be
    valid, the instance must have had at least one capability set (to either
    true or false) using a set method, or copied from another instance
    (such as by the assignment operator).
*/
bool QGeoCameraCapabilities::isValid() const
{
    return d->valid_;
}

/*!
    \qmlproperty real cameraCapabilities::minimumZoomLevel

    This read-only property holds the minimum available zoom level with this map type.
*/

/*!
    \property QGeoCameraCapabilities::minimumZoomLevel
    \brief the minimum zoom level supported by the associated plugin.

    Larger values of the zoom level correspond to more detailed views of the
    map.
*/
void QGeoCameraCapabilities::setMinimumZoomLevel(double minimumZoomLevel)
{
    d->minZoom_ = minimumZoomLevel;
    d->valid_ = true;
}

double QGeoCameraCapabilities::minimumZoomLevel() const
{
    return d->minZoom_;
}

double QGeoCameraCapabilities::minimumZoomLevelAt256() const
{
    if (d->tileSize_ == 256)
        return d->minZoom_;
    return qMax<double>(0, zoomLevelTo256(d->minZoom_, d->tileSize_));
}

/*!
    \qmlproperty real cameraCapabilities::maximumZoomLevel

    This read-only property holds the maximum available zoom level with this map type.
*/

/*!
    \property QGeoCameraCapabilities::maximumZoomLevel
    \brief the maximum zoom level supported by the associated plugin.

    Larger values of the zoom level correspond to more detailed views of the
    map.
*/
void QGeoCameraCapabilities::setMaximumZoomLevel(double maximumZoomLevel)
{
    d->maxZoom_ = maximumZoomLevel;
    d->valid_ = true;
}

double QGeoCameraCapabilities::maximumZoomLevel() const
{
    return d->maxZoom_;
}

double QGeoCameraCapabilities::maximumZoomLevelAt256() const
{
    if (d->tileSize_ == 256)
        return d->maxZoom_;
    return qMax<double>(0, zoomLevelTo256(d->maxZoom_, d->tileSize_));
}

/*!
    Sets whether the associated plugin can render a map when the camera
    has an arbitrary bearing to \a supportsBearing.
*/
void QGeoCameraCapabilities::setSupportsBearing(bool supportsBearing)
{
    d->supportsBearing_ = supportsBearing;
    d->valid_ = true;
}

/*!
    Returns whether the associated plugin can render a map when the camera
    has an arbitrary bearing.
*/
bool QGeoCameraCapabilities::supportsBearing() const
{
    return d->supportsBearing_;
}

/*!
    Sets whether the associated plugin can render a map when the
    camera is rolled to \a supportsRolling.
*/
void QGeoCameraCapabilities::setSupportsRolling(bool supportsRolling)
{
    d->supportsRolling_ = supportsRolling;
    d->valid_ = true;
}

/*!
    Returns whether the associated plugin can render a map when the
    camera is rolled.
*/
bool QGeoCameraCapabilities::supportsRolling() const
{
    return d->supportsRolling_;
}

/*!
    Sets whether the associated plugin can render a map when the
    camera is tilted to \a supportsTilting.
*/
void QGeoCameraCapabilities::setSupportsTilting(bool supportsTilting)
{
    d->supportsTilting_ = supportsTilting;
    d->valid_ = true;
}

/*!
    Returns whether the associated plugin can render a map when the
    camera is tilted.
*/
bool QGeoCameraCapabilities::supportsTilting() const
{
    return d->supportsTilting_;
}

/*!
    \qmlproperty real cameraCapabilities::minimumTilt

    This read-only property holds the minimum available tilt with this map type.
*/

/*!
    \property QGeoCameraCapabilities::minimumTilt
    \brief the minimum tilt supported by the associated plugin.

    The value is in degrees where 0 is equivalent to 90 degrees between
    the line of view and earth's surface, that is, looking straight down to earth.
*/
void QGeoCameraCapabilities::setMinimumTilt(double minimumTilt)
{
    d->minTilt_ = minimumTilt;
    d->valid_ = true;
}

double QGeoCameraCapabilities::minimumTilt() const
{
    return d->minTilt_;
}

/*!
    \qmlproperty real cameraCapabilities::maximumTilt

    This read-only property holds the maximum available tilt with this map type.
*/

/*!
    \property QGeoCameraCapabilities::maximumTilt
    \brief the maximum tilt supported by the associated plugin.

    The value is in degrees where 0 is equivalent to 90 degrees between
    the line of view and earth's surface, that is, looking straight down to earth.
*/
void QGeoCameraCapabilities::setMaximumTilt(double maximumTilt)
{
    d->maxTilt_ = maximumTilt;
    d->valid_ = true;
}

double QGeoCameraCapabilities::maximumTilt() const
{
    return d->maxTilt_;
}

/*!
    \qmlproperty real cameraCapabilities::minimumFieldOfView

    This read-only property holds the minimum available field of view with this map type.
*/

/*!
    \property QGeoCameraCapabilities::minimumFieldOfView
    \brief the minimum field of view supported by the associated plugin.

    The value is in degrees and is clamped against a [1, 179] range.

    \since 5.9
*/
void QGeoCameraCapabilities::setMinimumFieldOfView(double minimumFieldOfView)
{
    d->minimumFieldOfView_ = qBound(1.0, minimumFieldOfView, 179.0);
    d->valid_ = true;
}

double QGeoCameraCapabilities::minimumFieldOfView() const
{
    return d->minimumFieldOfView_;
}

/*!
    \qmlproperty real cameraCapabilities::maximumFieldOfView

    This read-only property holds the maximum available field of view with this map type.
*/

/*!
    \property QGeoCameraCapabilities::maximumFieldOfView
    \brief the maximum field of view supported by the associated plugin.

    The value is in degrees and is clamped against a [1, 179] range.

    \since 5.9
*/
void QGeoCameraCapabilities::setMaximumFieldOfView(double maximumFieldOfView)
{
    d->maximumFieldOfView_ = qBound(1.0, maximumFieldOfView, 179.0);
    d->valid_ = true;
}

double QGeoCameraCapabilities::maximumFieldOfView() const
{
    return d->maximumFieldOfView_;
}

/*!
    Sets whether overzooming is supported by the associated plugin.

    Overzooming means that zoom levels outside the [minimumZL, maximumZL] range can be set,
    and if tiles aren't available for those zoom levels, either tiles from other zoom levels
    will be used, or nothing will be shown.

    Set this value to false if the plugin is not capable of that. For example if using
    a mapping engine that always clamp the zoomLevel value, which may cause misalignment in case
    of stacked map elements.

    \since 5.9
*/
void QGeoCameraCapabilities::setOverzoomEnabled(bool overzoomEnabled)
{
    d->overzoomEnabled_ = overzoomEnabled;
    d->valid_ = true;
}

/*!
    Returns whether overzooming is supported by the associated plugin.

    \since 5.9
*/
bool QGeoCameraCapabilities::overzoomEnabled() const
{
    return d->overzoomEnabled_;
}


QT_END_NAMESPACE

#include "moc_qgeocameracapabilities_p.cpp"
