// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qquick3dpickresult_p.h"
#include "qquick3dmodel_p.h"

QT_BEGIN_NAMESPACE

/*!
    \qmlvaluetype pickResult
    \inqmlmodule QtQuick3D
    \brief Contains the results of a pick.

    Created as a return object to View3D::pick.
*/

QQuick3DPickResult::QQuick3DPickResult()
    : m_objectHit(nullptr)
    , m_distance(0.0f)
    , m_instanceIndex(-1)
    , m_itemHit(nullptr)
    , m_hitType(QQuick3DPickResultEnums::HitType::Null)
{

}

QQuick3DPickResult::QQuick3DPickResult(QQuick3DModel *hitObject,
                                       float distanceFromCamera,
                                       const QVector2D &uvPosition,
                                       const QVector3D &scenePosition,
                                       const QVector3D &position,
                                       const QVector3D &normal,
                                       const QVector3D &sceneNormal,
                                       int instanceIndex,
                                       QQuickItem *itemHit)
    : m_objectHit(hitObject)
    , m_distance(distanceFromCamera)
    , m_uvPosition(uvPosition)
    , m_scenePosition(scenePosition)
    , m_position(position)
    , m_normal(normal)
    , m_sceneNormal(sceneNormal)
    , m_instanceIndex(instanceIndex)
    , m_itemHit(itemHit)
    , m_hitType(QQuick3DPickResultEnums::HitType::Model)
{

}

QQuick3DPickResult::QQuick3DPickResult(QQuickItem *itemHit,
                                       float distanceFromCamera,
                                       const QVector2D &uvPosition,
                                       const QVector3D &scenePosition,
                                       const QVector3D &position,
                                       const QVector3D &sceneNormal)
    : m_objectHit(nullptr)
    , m_distance(distanceFromCamera)
    , m_uvPosition(uvPosition)
    , m_scenePosition(scenePosition)
    , m_position(position)
    , m_normal({ 0, 0, 1 })
    , m_sceneNormal(sceneNormal)
    , m_instanceIndex(-1)
    , m_itemHit(itemHit)
    , m_hitType(QQuick3DPickResultEnums::HitType::Item)
{

}

/*!
    \qmlproperty Model pickResult::objectHit
    \readonly

    This property holds the model object hit by the pick. This value will be null if
    \l{pickResult::hitType} {hitType} is not \c pickResult.Model.

    \sa itemHit
*/
QQuick3DModel *QQuick3DPickResult::objectHit() const
{
    return m_objectHit;
}

/*!
    \qmlproperty real pickResult::distance
    \readonly

    This property holds the distance between the pick origin and the hit position
    i.e. the length of the ray. In the case of using viewport coordinates for
    picking the pick origin will be the active camera's position.
*/
float QQuick3DPickResult::distance() const
{
    return m_distance;
}

/*!
    \qmlproperty vector2d pickResult::uvPosition
    \readonly

    This property holds the UV position of the hit. The UV position is calculated as
    the normalized local x and y coordinates of the hit point relative to the bounding volume.
    Useful for further picking against an offscreen-rendered object.

    When \l{pickResult::}{hitType} is \c pickResult.Item this value will represent the position
    of the hit in the coordinate space of \l{pickResult::}{itemHit}.
*/
QVector2D QQuick3DPickResult::uvPosition() const
{
    return m_uvPosition;
}

/*!
    \qmlproperty vector3d pickResult::scenePosition
    \readonly

    This property holds the scene position of the hit.
*/
QVector3D QQuick3DPickResult::scenePosition() const
{
    return m_scenePosition;
}

/*!
    \qmlproperty vector3d pickResult::position
    \readonly

    This property holds the scene position of the hit in local coordinate
    space.
*/
QVector3D QQuick3DPickResult::position() const
{
    return m_position;
}

/*!
    \qmlproperty vector3d pickResult::normal
    \readonly

    This property holds the normal of the face that was hit in local coordinate
    space.

    \note for 2D Items this will always be (0, 0, 1).
*/
QVector3D QQuick3DPickResult::normal() const
{
    return m_normal;
}


/*!
    \qmlproperty vector3d pickResult::sceneNormal
    \readonly

    This property holds the normal of the face that was hit in scene coordinate
    space.
*/
QVector3D QQuick3DPickResult::sceneNormal() const
{
    return m_sceneNormal;
}


/*!
    \qmlproperty int pickResult::instanceIndex
    \readonly
    \since 6.5

    This property holds the index in the instance table for the case
    where the pick hit an instance of an instanced model.
*/
int QQuick3DPickResult::instanceIndex() const
{
    return m_instanceIndex;
}

/*!
    \qmlproperty Item pickResult::itemHit
    \readonly
    \since 6.8

    This property holds the Qt Quick Item hit by the pick. This value will be null if
    \l{pickResult::}{hitType} is not \c pickResult.Item.

    \sa objectHit
*/

QQuickItem *QQuick3DPickResult::itemHit() const
{
    return m_itemHit;
}

/*!
    \qmlproperty enumeration pickResult::hitType
    \readonly
    \since 6.8

    This property holds the hit type of the pick result.

    \value PickResult.Null The pick did not hit anything.
    \value PickResult.Model The pick hit a Model.
    \value PickResult.Item The pick hit a QQuickItem.
*/

QQuick3DPickResultEnums::HitType QQuick3DPickResult::hitType() const
{
    return m_hitType;
}

QT_END_NAMESPACE
