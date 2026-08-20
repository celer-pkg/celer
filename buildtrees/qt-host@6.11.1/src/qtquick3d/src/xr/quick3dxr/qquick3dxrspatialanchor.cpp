// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qquick3dxrspatialanchor_p.h"

#if defined(Q_OS_VISIONOS)
#include "visionos/qquick3dxranchormanager_visionos_p.h"
#else
#include "openxr/qquick3dxranchormanager_openxr_p.h"
#endif

QT_BEGIN_NAMESPACE

/*!
    \qmltype XrSpatialAnchor
    \inherits QtObject
    \inqmlmodule QtQuick3D.Xr
    \brief Tracks a specific location or object in real space.

    This type represents a spatial anchor that tracks
    a specific location or object in real space. It provides information about
    the anchor's position, rotation, classification, and bounds.

    Spatial anchors are accessed through an \l XrSpatialAnchorListModel.

    \note The system provides Anchor objects. They cannot be created in QML.

    See the \l{Qt Quick 3D - XR Spatial Anchors Example} for how to use this type.

    \section1 Platform notes

    \section2 Meta Quest Devices

    This API makes use of the Meta Quest Scene and Spatial Anchor APIs.
    You need to add the following permissions to your app's \c AndroidManifest.xml file:

    \badcode
        <uses-permission android:name="com.oculus.permission.USE_ANCHOR_API"/>
        <uses-permission android:name="com.oculus.permission.USE_SCENE"/>
    \endcode

    \note You need to manually set up a Space before running an app, or anchors will
    not be available.
 */

/*!
    \qmlproperty enumeration XrSpatialAnchor::Classification
    \ingroup xr-anchors
    \brief The classification of the spatial anchor.
    \readonly

    The Classification enum provides a set of predefined category types that describe
    the purpose or context of a spatial anchor.

    \value Classification.Unknown The label has not been set or identified.
    \value Classification.Wall The anchor represents a wall.
    \value Classification.Ceiling The anchor represents a ceiling.
    \value Classification.Floor The anchor represents a floor.
    \value Classification.Table The anchor represents a table.
    \value Classification.Seat The anchor represents a seat.
    \value Classification.Window The anchor represents a window.
    \value Classification.Door The anchor represents a door.
    \value Classification.Other The anchor was not identified as any of the above types. See: \l classificationString

    The following table shows the mapping between the classification type in \qxr,
    OpenXR, and VisionOS. If the classification type from the system falls outside of the defined types,
    then the \e Type is set to \c Other, and the system type is provided by the \l classificationString property.

    \note The classification string can also be \c{Other}.

    \table
    \header
        \li Type
        \li OpenXR
        \li VisionOS
        \li Description
    \row
        \li Unknown
        \li -
        \li -
        \li The label has not been set or identified.
    \row
        \li Wall
        \li WALL_FACE
        \li Wall
        \li The anchor represents a wall.
    \row
        \li Ceiling
        \li CEILING
        \li Ceiling
        \li The anchor represents a ceiling.
    \row
        \li Floor
        \li FLOOR
        \li Floor
        \li The anchor represents a floor.
    \row
        \li Table
        \li TABLE
        \li Table
        \li The anchor represents a table.
    \row
        \li Seat
        \li COUCH
        \li Seat
        \li The anchor represents a seat.
    \row
        \li Window
        \li WINDOW_FRAME
        \li Window
        \li The anchor represents a window.
    \row
        \li Door
        \li DOOR_FRAME
        \li Door
        \li The anchor represents a door.
    \row
        \li Other
        \li -
        \li -
        \li The anchor represents something else. See: \l classificationString
    \endtable
 */

QQuick3DXrSpatialAnchor::QQuick3DXrSpatialAnchor(QtQuick3DXr::XrSpaceId space, QUuid &uuid, QObject *parent)
    : QObject(parent)
    , m_space(space)
    , m_uuid(uuid)
{
}

QQuick3DXrSpatialAnchor::~QQuick3DXrSpatialAnchor()
{
}

/*!
    \qmlproperty vector3d XrSpatialAnchor::offset3D
    \brief The 3D offset of the spatial anchor.
    \readonly

    This property provides the 3D offset of the anchor's bounds (in meters) from the anchor's \l position.

    \sa offset3D, has3DBounds
 */

QVector3D QQuick3DXrSpatialAnchor::offset3D() const
{
    return m_offset3D;
}

void QQuick3DXrSpatialAnchor::setOffset3D(const QVector3D &newOffset)
{
    if (m_offset3D == newOffset)
        return;
    m_offset3D = newOffset;
    emit offset3DChanged();
}

/*!
    \qmlproperty vector3d XrSpatialAnchor::extent3D
    \brief The 3D extent of the spatial anchor.
    \readonly

    This property specifies the spatial anchor's volume in three dimensions (width, height, and depth).
    It is valid when \l has3DBounds is \c true.

    \sa offset3D, has3DBounds
 */

QVector3D QQuick3DXrSpatialAnchor::extent3D() const
{
    return m_extent3D;
}

void QQuick3DXrSpatialAnchor::setExtent3D(const QVector3D &newExtent)
{
    if (m_extent3D == newExtent)
        return;
    m_extent3D = newExtent;
    emit extent3DChanged();
}

/*!
    \qmlproperty vector3d XrSpatialAnchor::position
    \brief The position of the spatial anchor.

    \readonly
    This property returns the 3D position (in meters) of the spatial anchor's origin within the
    session's coordinate system.
 */

QVector3D QQuick3DXrSpatialAnchor::position() const
{
    return m_position;
}

void QQuick3DXrSpatialAnchor::setPosition(const QVector3D &newPosition)
{
    if (m_position == newPosition)
        return;
    m_position = newPosition;
    emit positionChanged();
}

/*!
    \qmlproperty quaternion XrSpatialAnchor::rotation
    \brief The orientation of the spatial anchor.
    \readonly

    This property provides the spatial anchor's rotation (as a quaternion).
 */

QQuaternion QQuick3DXrSpatialAnchor::rotation() const
{
    return m_rotation;
}

void QQuick3DXrSpatialAnchor::setRotation(const QQuaternion &newRotation)
{
    if (m_rotation == newRotation)
        return;
    m_rotation = newRotation;
    emit rotationChanged();
}

/*!
    \qmlproperty enumeration XrSpatialAnchor::classification
    \brief The classification type of the spatial anchor.
    \readonly

    This property returns the \l {XrSpatialAnchor::Classification}{classification type} for this anchor
    (for example,\c Table or \c Floor) describing the anchor's purpose or context.

    \note The classification type coming from the system might not be in the set of labels
    defined by the \l Classification enum, in which case the type will be set to \c Other
    and the \l classificationString property will contain the original label.

    \sa classificationString
 */

QQuick3DXrSpatialAnchor::Classification QQuick3DXrSpatialAnchor::classification() const
{
    return m_classification;
}

void QQuick3DXrSpatialAnchor::setClassification(Classification newClassification)
{
    if (m_classification == newClassification)
        return;
    m_classification = newClassification;
    emit classificationChanged();
}

/*!
    \qmlproperty string XrSpatialAnchor::classificationString
    \brief The classification type of the spatial anchor as a string.
    \readonly

    This property returns the classification type as a string if one exists.
    If the classification type is not in the set of types defined by the \l Classification enums, the
    label is set to \c Other, and this property can be used to access the type as it was reported by
    the system.

    \note This string can be empty or change, depending on the system and how the anchor gets classified.

    \sa classification
*/

QString QQuick3DXrSpatialAnchor::classificationString() const
{
    return m_classificationString;
}

void QQuick3DXrSpatialAnchor::setClassificationString(const QString &newClassificationString)
{
    if (m_classificationString == newClassificationString)
        return;
    m_classificationString = newClassificationString;
    emit classificationStringChanged();
}

/*!
    \qmlproperty bool XrSpatialAnchor::has2DBounds
    \brief Indicates whether the spatial anchor has 2D bounds.
    \readonly

    This property holds \c true if the spatial anchor has 2D bounds,
    described by \l offset2D and \l extent2D, indicating that it
    represents a flat surface (for example, a floor or wall).

    Otherwise, it returns false.
    \sa offset2D, extent2D, has3DBounds
 */

bool QQuick3DXrSpatialAnchor::has2DBounds() const
{
    return m_has2DBounds;
}

void QQuick3DXrSpatialAnchor::setBounds2D(const QVector2D &offset, const QVector2D &extent)
{
    if (qFuzzyCompare(m_offset2D, offset) && qFuzzyCompare(m_extent2D, extent))
        return;

    m_offset2D = offset;
    m_extent2D = extent;

    // FIXME: verify
    m_has2DBounds = true;

    emit has2DBoundsChanged();
}

/*!
    \qmlproperty bool XrSpatialAnchor::has3DBounds
    \brief Indicates whether the spatial anchor has 3D bounds.
    \readonly

    This property returns \c true if the spatial anchor has 3D bounds, indicating
    that it represents a volume (for example, a table or a cupboard).
    The bounds are described by \l offset3D and \l extent3D.

    Otherwise, it returns \c false.
    \sa offset3D, extent3D, has2DBounds
 */


bool QQuick3DXrSpatialAnchor::has3DBounds() const
{
    return m_has3DBounds;
}

void QQuick3DXrSpatialAnchor::setBounds3D(const QVector3D &offset, const QVector3D &extent)
{
    if (qFuzzyCompare(m_offset3D, offset) && qFuzzyCompare(m_extent3D, extent))
        return;

    m_offset3D = offset;
    m_extent3D = extent;

    // FIXME: Store the 3D bounds and verify
    m_has3DBounds = true;

    emit has3DBoundsChanged();
}

/*!
    \qmlproperty vector2d XrSpatialAnchor::offset2D
    \brief The 2D offset of the spatial anchor.

    \readonly
    This property holds the offset of the anchor's bounds within
    the X/Z plane. It is valid when \l has2DBounds is \c true.

    \sa has2DBounds, extent2D
*/

QVector2D QQuick3DXrSpatialAnchor::offset2D() const
{
    return m_offset2D;
}

/*!
    \qmlproperty vector2d XrSpatialAnchor::extent2D
    \brief The 2D extent of the spatial anchor.
    \readonly

    This property holds the spatial anchor's size in two dimensions (width and height) within
    the X/Z plane. It is valid when \l has2DBounds is \c true.

    \sa has2DBounds, offset2D
 */

QVector2D QQuick3DXrSpatialAnchor::extent2D() const
{
    return m_extent2D;
}

/*!
    \qmlproperty string XrSpatialAnchor::identifier
    \brief A unique identifier for this spatial anchor.
    \readonly

    This property holds a unique identifier associated with the
    spatial anchor. This is the same identifier referenced by a \l XrSpatialAnchorListModel.
 */

QString QQuick3DXrSpatialAnchor::identifier() const
{
    return QString::fromLatin1(m_uuid.toRfc4122());
}

QSet<QUuid> QQuick3DXrSpatialAnchor::roomLayoutUuids() const
{
    return m_roomLayoutUuids;
}

void QQuick3DXrSpatialAnchor::setRoomLayoutUuids(const QSet<QUuid> &newRoomLayoutUuids)
{
    m_roomLayoutUuids = newRoomLayoutUuids;
}

QSet<QUuid> QQuick3DXrSpatialAnchor::spaceContainerUuids() const
{
    return m_spaceContainerUuids;
}

void QQuick3DXrSpatialAnchor::setSpaceContainerUuids(const QSet<QUuid> &newSpaceContainerUuids)
{
    m_spaceContainerUuids = newSpaceContainerUuids;
}

QT_END_NAMESPACE
