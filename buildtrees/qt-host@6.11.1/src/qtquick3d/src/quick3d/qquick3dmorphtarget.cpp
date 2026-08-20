// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qquick3dmorphtarget_p.h"

#include <QtQuick3DRuntimeRender/private/qssgrendermorphtarget_p.h>
#include <QtQml/QQmlFile>

QT_BEGIN_NAMESPACE

/*!
    \qmltype MorphTarget
    \inherits Object3D
    \inqmlmodule QtQuick3D
    \brief Defines the properties of a morph target.

    Each \e MorphTarget is a morph target for a \l{Morphing Animation}{vertex animation}. The degree
    of morphing is controlled by changing the \l {weight} property.

    \qml
    MorphTarget {
        id: morphtarget0
        attributes: MorphTarget.Position | MorphTarget.Normal
        weight: 0.5
    }
    \endqml

    The \l {Qt Quick 3D - Morphing Example}{morphing example} shows how to use a morph target.
*/

QQuick3DMorphTarget::QQuick3DMorphTarget(QQuick3DObject *parent)
    : QQuick3DObject(*(new QQuick3DObjectPrivate(QQuick3DObjectPrivate::Type::MorphTarget)), parent) {}

QQuick3DMorphTarget::~QQuick3DMorphTarget()
{
}

/*!
    \qmlproperty real MorphTarget::weight

    Specifies the weight of the current morph target. The weight is the multiplication factor used
    by the linear interpolation. A weight of 1 means that this target is fully applied. A weight of
    0 means that it has no influence.
*/

float QQuick3DMorphTarget::weight() const
{
    return m_weight;
}

/*!
    \qmlproperty enumeration MorphTarget::attributes

    Specifies the set of attributes of the current morph target.
    In order to animate vertex attributes in morphing, the mesh must
    contain those target attributes and the morph target must have the
    attributes enabled.

    The attributes for a morph target are specified by OR-ing together the following values:
    \value MorphTarget.Position animates the vertex positions
    \value MorphTarget.Normal animates the normal vectors
    \value MorphTarget.Tangent animates the tangent vectors
    \value MorphTarget.Binormal animates the binormal vectors
    \value MorphTarget.TexCoord0 animates the texture coordinate 0 vectors
    \value MorphTarget.TexCoord1 animates the texture coordinate 1 vectors
    \value MorphTarget.Color animates the vertex color vectors
*/

QQuick3DMorphTarget::MorphTargetAttributes QQuick3DMorphTarget::attributes() const
{
    return m_attributes;
}

void QQuick3DMorphTarget::markAllDirty()
{
    m_dirtyAttributes = 0xffffffff;
    QQuick3DObject::markAllDirty();
}

void QQuick3DMorphTarget::setWeight(float weight)
{
    if (m_weight == weight)
        return;

    m_weight = weight;
    emit weightChanged();
    markDirty(WeightDirty);
}

void QQuick3DMorphTarget::setAttributes(MorphTargetAttributes attributes)
{
    if (m_attributes == attributes)
        return;

    m_attributes = attributes;
    m_numAttribs = 0;
    int flags = attributes;
    while (flags) {
        m_numAttribs += flags & 0x1;
        flags >>= 1;
    }
    emit attributesChanged();
    markDirty(MorphTargetAttributesDirty);
}

QSSGRenderGraphObject *QQuick3DMorphTarget::updateSpatialNode(QSSGRenderGraphObject *node)
{
    if (!node) {
        markAllDirty();
        node = new QSSGRenderMorphTarget();
    }
    QQuick3DObject::updateSpatialNode(node);
    auto modelNode = static_cast<QSSGRenderMorphTarget *>(node);
    if (m_dirtyAttributes & WeightDirty)
        modelNode->weight = m_weight;
    if (m_dirtyAttributes & MorphTargetAttributesDirty)
        modelNode->attributes = QSSGRenderMorphTarget::InputAttributes(int(m_attributes));

    m_dirtyAttributes = 0;

    return modelNode;
}

void QQuick3DMorphTarget::markDirty(QQuick3DMorphTarget::QSSGMorphTargetDirtyType type)
{
    if (!(m_dirtyAttributes & quint32(type))) {
        m_dirtyAttributes |= quint32(type);
        update();
    }
}

size_t QQuick3DMorphTarget::numAttribs()
{
    return m_numAttribs;
}

QT_END_NAMESPACE
