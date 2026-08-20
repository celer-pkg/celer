// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qquick3ddirectionallight_p.h"
#include "qquick3dobject_p.h"

#include <QtQuick3DRuntimeRender/private/qssgrenderlight_p.h>

#include "qquick3dnode_p_p.h"

QT_BEGIN_NAMESPACE

/*!
    \qmltype DirectionalLight
    \inherits Light
    \inqmlmodule QtQuick3D
    \brief Defines a directional light in the scene.

    The directional light emits light in one direction from an unidentifiable source located
    infinitely far away. This is similar to the way sunlight works in real life. A directional
    light has infinite range and does not diminish.

    If \l {Light::castsShadow}{castsShadow} is enabled, shadows will be parallel to the light
    direction.

    A directional light effectively have no position, so moving it does not
    have any effect. The light will always be emitted in the direction of the
    light's Z axis.

    Rotating the light along its X or Y axis will change the direction of the light emission.

    Scaling a directional light will only have an effect in the following cases:
    \list
    \li If Z scale is set to a negative number, the light will be emitted in the opposite direction.
    \li If the scale of any axis is set to 0, the light will be emitted along the world's Z axis.
    \note Rotating the light will then have no effect.
    \endlist

    Let's look at a simple example:

    \qml
    import QtQuick
    import QtQuick3D
    View3D {
        anchors.fill: parent

        PerspectiveCamera { z: 600 }

        DirectionalLight {
        }

        Model {
            source: "#Sphere"
            scale: Qt.vector3d(4, 4, 4)
            materials: PrincipledMaterial {
                baseColor: "#40c060"
                roughness: 0.1 // make specular highlight visible
            }
        }
    }
    \endqml

    Here the DirectionalLight uses the default \c white color, emitting in the
    direction of the DirectionalLight node's Z axis.

    \image directionallight-1.png
           {Green circle showing directional light emitting along Z axis}

    Rotating 60 degrees around the X axis would lead to the following. Instead
    of emitting straight in the direction of the Z axis, the light is now
    pointing 60 degrees "down":

    \qml
    DirectionalLight {
        eulerRotation.x: 60
    }
    \endqml

    \image directionallight-2.png
           {Green circle with shading showing rotated directional light}

    For further usage examples, see \l{Qt Quick 3D - Lights Example}.

    \sa PointLight, SpotLight, {Shadow Mapping}
*/

/*!
    \qmlproperty real DirectionalLight::csmSplit1
    \since 6.8

    This property defines where the first cascade of the shadow map split will occur when
    CSM is active.

    Range: \c{[0.0, 1.0]}

    Default value: \c{0.1}

    \sa csmSplit2, csmSplit3
    \note This property is only used when DirectionalLight::csmNumSplits is greater than \c{0}.
*/

/*!
    \qmlproperty real DirectionalLight::csmSplit2
    \since 6.8

    This property defines where the second cascade of the shadow map split will occur when
    CSM is active.

    Range: \c{[0.0, 1.0]}

    Default value: \c{0.25}

    \sa csmSplit1, csmSplit3
    \note This property is only used when DirectionalLight::csmNumSplits is greater than \c{1}.
*/

/*!
    \qmlproperty real DirectionalLight::csmSplit3
    \since 6.8

    This property defines where the third cascade of the shadow map split will occur when
    CSM is active.

    Range: \c{[0.0, 1.0]}

    Default value: \c{0.5}

    \sa csmSplit1, csmSplit2
    \note This property is only used when DirectionalLight::csmNumSplits is greater than \c{2}.
*/

/*!
    \qmlproperty int DirectionalLight::csmNumSplits
    \since 6.8

    This property defines the number of splits the frustum should be split by when
    rendering the shadowmap. No splits means that the shadowmap will be rendered
    so that it covers the bounding box of all shadow casting and receiving objects.

    Range: \c{[0, 3]}

    Default value: \c{0}

    \sa csmSplit1, csmSplit2, csmSplit3
*/

/*!
    \qmlproperty real DirectionalLight::csmBlendRatio
    \since 6.8

    This property defines how much of the shadow of any cascade should be blended
    together with the previous one.

    Range: \c{[0.0, 1.0]}

    Default value: \c{0.05}
*/

/*!
    \qmlproperty bool DirectionalLight::lockShadowmapTexels
    \since 6.9

    When this property is enabled, the shadowmap texels are "locked" in position in the
    world eliminating shadow edge shimmering at the cost of bigger shadowmap texels.
    It works by using uniform sized, and texel aligned shadowmaps for each cascade.

    Default value: \c{false}
*/

QQuick3DDirectionalLight::QQuick3DDirectionalLight(QQuick3DNode *parent)
    : QQuick3DAbstractLight(*(new QQuick3DNodePrivate(QQuick3DNodePrivate::Type::DirectionalLight)), parent) {}

float QQuick3DDirectionalLight::csmSplit1() const
{
    return m_csmSplit1;
}

float QQuick3DDirectionalLight::csmSplit2() const
{
    return m_csmSplit2;
}

float QQuick3DDirectionalLight::csmSplit3() const
{
    return m_csmSplit3;
}

int QQuick3DDirectionalLight::csmNumSplits() const
{
    return m_csmNumSplits;
}

float QQuick3DDirectionalLight::csmBlendRatio() const
{
    return m_csmBlendRatio;
}

bool QQuick3DDirectionalLight::lockShadowmapTexels() const
{
    return m_lockShadowmapTexels;
}

void QQuick3DDirectionalLight::setCsmSplit1(float newcsmSplit1)
{
    newcsmSplit1 = qBound(0.0f, newcsmSplit1, 1.0f);
    if (qFuzzyCompare(m_csmSplit1, newcsmSplit1))
        return;

    m_csmSplit1 = newcsmSplit1;
    emit csmSplit1Changed();
    m_dirtyFlags.setFlag(QQuick3DAbstractLight::DirtyFlag::ShadowDirty);
    update();
}

void QQuick3DDirectionalLight::setCsmSplit2(float newcsmSplit2)
{
    newcsmSplit2 = qBound(0.0f, newcsmSplit2, 1.0f);
    if (qFuzzyCompare(m_csmSplit2, newcsmSplit2))
        return;

    m_csmSplit2 = newcsmSplit2;
    emit csmSplit2Changed();
    m_dirtyFlags.setFlag(QQuick3DAbstractLight::DirtyFlag::ShadowDirty);
    update();
}

void QQuick3DDirectionalLight::setCsmSplit3(float newcsmSplit3)
{
    newcsmSplit3 = qBound(0.0f, newcsmSplit3, 1.0f);
    if (qFuzzyCompare(m_csmSplit3, newcsmSplit3))
        return;

    m_csmSplit3 = newcsmSplit3;
    emit csmSplit3Changed();
    m_dirtyFlags.setFlag(QQuick3DAbstractLight::DirtyFlag::ShadowDirty);
    update();
}

void QQuick3DDirectionalLight::setCsmNumSplits(int newcsmNumSplits)
{
    newcsmNumSplits = qBound(0, newcsmNumSplits, 3);
    if (m_csmNumSplits == newcsmNumSplits)
        return;

    m_csmNumSplits = newcsmNumSplits;
    emit csmNumSplitsChanged();
    m_dirtyFlags.setFlag(QQuick3DAbstractLight::DirtyFlag::ShadowDirty);
    update();
}

void QQuick3DDirectionalLight::setCsmBlendRatio(float newcsmBlendRatio)
{
    newcsmBlendRatio = qBound(0.0, newcsmBlendRatio, 1.0);
    if (m_csmBlendRatio == newcsmBlendRatio)
        return;

    m_csmBlendRatio = newcsmBlendRatio;
    emit csmBlendRatioChanged();
    m_dirtyFlags.setFlag(QQuick3DAbstractLight::DirtyFlag::ShadowDirty);
    update();
}

void QQuick3DDirectionalLight::setLockShadowmapTexels(bool newLockShadowmapTexels)
{
    if (m_lockShadowmapTexels == newLockShadowmapTexels)
        return;

    m_lockShadowmapTexels = newLockShadowmapTexels;
    emit lockShadowmapTexelsChanged();
    m_dirtyFlags.setFlag(QQuick3DAbstractLight::DirtyFlag::ShadowDirty);
    update();
}

QSSGRenderGraphObject *QQuick3DDirectionalLight::updateSpatialNode(QSSGRenderGraphObject *node)
{
    if (!node) {
        markAllDirty();
        node = new QSSGRenderLight(/* defaults to directional */);
    }

    if (m_dirtyFlags.testFlag(DirtyFlag::ShadowDirty)) {
        QSSGRenderLight *light = static_cast<QSSGRenderLight *>(node);
        light->m_csmSplit1 = m_csmSplit1;
        light->m_csmSplit2 = m_csmSplit2;
        light->m_csmSplit3 = m_csmSplit3;
        light->m_csmNumSplits = m_csmNumSplits;
        light->m_csmBlendRatio = m_csmBlendRatio;
        light->m_lockShadowmapTexels = m_lockShadowmapTexels;
    }

    QQuick3DAbstractLight::updateSpatialNode(node); // Marks the light node dirty if m_dirtyFlags != 0

    return node;
}

QT_END_NAMESPACE
