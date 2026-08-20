// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qquick3dmaterial_p.h"
#include "qquick3dobject_p.h"
#include "qquick3dscenemanager_p.h"

#include <QtQuick3DRuntimeRender/private/qssgrenderdefaultmaterial_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrendercustommaterial_p.h>
#include <QtQuick3DUtils/private/qssgutils_p.h>

QT_BEGIN_NAMESPACE

/*!
    \qmltype Material
    \inherits Object3D
    \inqmlmodule QtQuick3D
    \brief Abstract base type providing functionality common to materials.
*/

/*!
    \qmlproperty Texture Material::lightProbe

    This property defines a Texture for overriding or setting an image based
    lighting Texture for use with this material only.

    \note Setting a light probe on the material will override the
    \l {SceneEnvironment::lightProbe} {scene's light probe} for models using this material.

    \note This property is ignored when Reflection Probe is used to show
    reflections on the Model using this material because Reflection Probe uses
    the \l {SceneEnvironment::lightProbe} {scene's light probe}.

    \sa SceneEnvironment::lightProbe
*/

/*!
    \qmlproperty enumeration Material::cullMode

    This property defines whether primitive culling is enabled, and, when
    enabled, which primitives are discarded.

    The default value is Material.BackFaceCulling.

    A triangle is considered front-facing if it has a counter-clockwise
    winding, meaning its vertices in framebuffer coordinates are in
    counter-clockwise order.

    \value Material.BackFaceCulling Back-facing triangles are discarded.
    \value Material.FrontFaceCulling Front-facing triangles are discarded.
    \value Material.NoCulling No triangles are discarded.
*/

/*!
    \qmlproperty enumeration Material::depthDrawMode

    This property determines if and when depth rendering takes place for this
    material. The default behavior when \l {SceneEnvironment::depthTestEnabled}
    is set to \c true is that during the main render pass only opaque materials
    will write to the depth buffer. This property makes it possible to change
    this behavior to fine tune the rendering of a material.

    The default value is Material.OqaqueOnlyDepthDraw

    \value Material.OpaqueOnlyDepthDraw Depth rendering is only performed if
    the material is opaque.
    \value Material.AlwaysDepthDraw Depth rendering is always performed
    regardless of the material type.
    \value Material.NeverDepthDraw Depth rendering is never performed.
    \value Material.OpaquePrePassDepthDraw Depth rendering is performed in a
    separate depth pass, but only opaque values are written. This mode also
    enables transparent materials to be used in combination with shadows.

    \note If \l {SceneEnvironment::depthPrePassEnabled} is set to \c true then all
    depth writes will take place as a result of the depth prepass, but it is
    still necessary to explicitly set \c Material.OpaquePrePassDepthDraw to only
    write the opaque fragments in the depth and shadow passes.

*/


QQuick3DMaterial::QQuick3DMaterial(QQuick3DObjectPrivate &dd, QQuick3DObject *parent)
    : QQuick3DObject(dd, parent) {}

QQuick3DMaterial::~QQuick3DMaterial()
{
}

QQuick3DTexture *QQuick3DMaterial::lightProbe() const
{
    return m_iblProbe;
}

QQuick3DMaterial::CullMode QQuick3DMaterial::cullMode() const
{
    return m_cullMode;
}

QQuick3DMaterial::DepthDrawMode QQuick3DMaterial::depthDrawMode() const
{
    return m_depthDrawMode;
}

void QQuick3DMaterial::setLightProbe(QQuick3DTexture *iblProbe)
{
    if (m_iblProbe == iblProbe)
        return;

    QQuick3DObjectPrivate::attachWatcher(this, &QQuick3DMaterial::setLightProbe, iblProbe, m_iblProbe);

    m_iblProbe = iblProbe;
    emit lightProbeChanged(m_iblProbe);
    update();
}

void QQuick3DMaterial::setCullMode(QQuick3DMaterial::CullMode cullMode)
{
    if (m_cullMode == cullMode)
        return;

    m_cullMode = cullMode;
    emit cullModeChanged(m_cullMode);
    update();
}

void QQuick3DMaterial::setDepthDrawMode(QQuick3DMaterial::DepthDrawMode depthDrawMode)
{
    if (m_depthDrawMode == depthDrawMode)
        return;

    m_depthDrawMode = depthDrawMode;
    emit depthDrawModeChanged(m_depthDrawMode);
    update();
}

QSSGRenderGraphObject *QQuick3DMaterial::updateSpatialNode(QSSGRenderGraphObject *node)
{
    if (!node)
        return nullptr;

    QQuick3DObject::updateSpatialNode(node);

    // Set the common properties
    if (node->type == QSSGRenderGraphObject::Type::DefaultMaterial ||
        node->type == QSSGRenderGraphObject::Type::PrincipledMaterial ||
        node->type == QSSGRenderGraphObject::Type::SpecularGlossyMaterial) {
        auto defaultMaterial = static_cast<QSSGRenderDefaultMaterial *>(node);

        if (!m_iblProbe)
            defaultMaterial->iblProbe = nullptr;
        else
            defaultMaterial->iblProbe = m_iblProbe->getRenderImage();

        defaultMaterial->cullMode = QSSGCullFaceMode(m_cullMode);
        defaultMaterial->depthDrawMode = QSSGDepthDrawMode(m_depthDrawMode);

        DebugViewHelpers::ensureDebugObjectName(defaultMaterial, this);

        node = defaultMaterial;
    } else if (node->type == QSSGRenderGraphObject::Type::CustomMaterial) {
        auto customMaterial = static_cast<QSSGRenderCustomMaterial *>(node);

        if (!m_iblProbe)
            customMaterial->m_iblProbe = nullptr;
        else
            customMaterial->m_iblProbe = m_iblProbe->getRenderImage();

        customMaterial->m_cullMode = QSSGCullFaceMode(m_cullMode);
        customMaterial->m_depthDrawMode = QSSGDepthDrawMode(m_depthDrawMode);

        DebugViewHelpers::ensureDebugObjectName(customMaterial, this);

        node = customMaterial;
    }

    return node;
}

void QQuick3DMaterial::itemChange(QQuick3DObject::ItemChange change, const QQuick3DObject::ItemChangeData &value)
{
    if (change == QQuick3DObject::ItemSceneChange)
        updateSceneManager(value.sceneManager);
}

void QQuick3DMaterial::updateSceneManager(QQuick3DSceneManager *sceneManager)
{
    if (sceneManager) {
        QQuick3DObjectPrivate::refSceneManager(m_iblProbe, *sceneManager);
    } else {
       QQuick3DObjectPrivate::derefSceneManager(m_iblProbe);
    }
}

QT_END_NAMESPACE
