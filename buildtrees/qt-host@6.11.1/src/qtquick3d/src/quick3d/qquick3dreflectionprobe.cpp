// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qquick3dreflectionprobe_p.h"
#include "qquick3dnode_p_p.h"

#include <QtQuick3DRuntimeRender/private/qssgrenderreflectionprobe_p.h>
#include <QtQuick3DUtils/private/qssgutils_p.h>

QT_BEGIN_NAMESPACE

/*!
    \qmltype ReflectionProbe
    \inherits Node
    \inqmlmodule QtQuick3D
    \brief Defines a reflection probe in the scene.

    A reflection probe is used to provide reflections of the current scene to the objects. The probe
    provides properties to the runtime which are then used to render the scene to a cube map. The cube map
    is then used as the reflection information for the reflecting objects.

    \sa {Qt Quick 3D - Reflection Probes Example}
*/

QQuick3DReflectionProbe::QQuick3DReflectionProbe(QQuick3DNode *parent)
    : QQuick3DNode(*(new QQuick3DNodePrivate(QQuick3DNodePrivate::Type::ReflectionProbe)), parent)
{
    QObject::connect(this, &QQuick3DReflectionProbe::scenePositionChanged, this, &QQuick3DReflectionProbe::updateDebugView);
}

void QQuick3DReflectionProbe::itemChange(QQuick3DObject::ItemChange change,
                                         const QQuick3DObject::ItemChangeData &value)
{
    if (change == QQuick3DObject::ItemSceneChange)
        updateSceneManager(value.sceneManager);
}

/*!
    \qmlproperty enumeration ReflectionProbe::quality

    Quality determines the resolution of the cube map.

    Possible values are:
    \value ReflectionProbe.VeryLow
    Renders a reflection map using a 128x128 texture.
    \value ReflectionProbe.Low
    Renders a reflection map using a 256x256 texture.
    \value ReflectionProbe.Medium
    Renders a reflection map using a 512x512 texture.
    \value ReflectionProbe.High
    Renders a reflection map using a 1024x1024 texture.
    \value ReflectionProbe.VeryHigh
    Renders a reflection map using a 2048x2048 texture.

    The default value is \c ReflectionProbe.Low
*/
QQuick3DReflectionProbe::ReflectionQuality QQuick3DReflectionProbe::quality() const
{
    return m_quality;
}

/*!
    \qmlproperty Color ReflectionProbe::clearColor

    Clear color is the color used to clear the cube map prior rendering the scene.
*/
QColor QQuick3DReflectionProbe::clearColor() const
{
    return m_clearColor;
}

/*!
    \qmlproperty enumeration ReflectionProbe::refreshMode

    Refresh mode tells the runtime how often the cube map should be updated.

    Possible values are:
    \value ReflectionProbe.FirstFrame
    Renders the scene on the first frame.
    \value ReflectionProbe.EveryFrame
    Renders the scene every frame.

    The default value is \c ReflectionProbe.EveryFrame
    \note Use \c ReflectionProbe.FirstFrame for improved performance.
*/
QQuick3DReflectionProbe::ReflectionRefreshMode QQuick3DReflectionProbe::refreshMode() const
{
    return m_refreshMode;
}

/*!
    \qmlproperty enumeration ReflectionProbe::timeSlicing

    Time slicing determines how the cube map render is timed.

    Possible values are:
    \value ReflectionProbe.None
        All faces of the cube map are rendered and prefiltered during one frame.

    \value ReflectionProbe.AllFacesAtOnce
        All faces are rendered during one frame but the prefiltering
        is divided to subsquent frames with each mip level handled on
        their own frame. Rough surface reflections are thus refreshed
        every sixth frame while smooth surfaces have reflections
        that refresh every frame.

    \value ReflectionProbe.IndividualFaces
        Each face is rendered and prefiltered in a separate frame.
        Thus all reflections are refreshed every sixth frame.

    The default value is \c ReflectionProbe.None
    \note Use \c ReflectionProbe.AllFacesAtOnce or
    \c ReflectionProbe.IndividualFaces to increase performance.
*/
QQuick3DReflectionProbe::ReflectionTimeSlicing QQuick3DReflectionProbe::timeSlicing() const
{
    return m_timeSlicing;
}

/*!
    \qmlproperty bool ReflectionProbe::parallaxCorrection

    By default the reflections provided by the reflection probe are assumed to be from an infinite distance similar
    to the skybox. This works fine for environmental reflections but for tight spaces this causes perspective errors
    in the reflections. To fix this parallax correction can be turned on. The distance of the reflection is then
    determined by the \l ReflectionProbe::boxSize property.

    \sa boxSize
*/
bool QQuick3DReflectionProbe::parallaxCorrection() const
{
    return m_parallaxCorrection;
}

/*!
    \qmlproperty vector3d ReflectionProbe::boxSize

    Box size is used to determine which objects get their reflections from this ReflectionProbe. Objects that are
    inside the box are under the influence of this ReflectionProbe. If an object lies inside more than one reflection
    probe at the same time, the object is considered to be inside the nearest reflection probe.
    With \l ReflectionProbe::parallaxCorrection turned on the size is also used to calculate the distance of
    the reflections in the cube map.

    \sa parallaxCorrection
*/
QVector3D QQuick3DReflectionProbe::boxSize() const
{
    return m_boxSize;
}

/*!
    \qmlproperty bool ReflectionProbe::debugView
    \since 6.4

    If this property is set to true, a wireframe is rendered to visualize the reflection probe box.
*/
bool QQuick3DReflectionProbe::debugView() const
{
    return m_debugView;
}

/*!
    \qmlproperty vector3d ReflectionProbe::boxOffset

    Box offset is used to move the box relative to the reflection probe position. Since the probe
    captures the environment from its position, this property can be used to move the box around
    without affecting the position where the probe capture the environment. This property
    alongside with \l ReflectionProbe::boxSize will be used to determine the object that fall
    inside the box.
    With \l ReflectionProbe::parallaxCorrection turned on, this property can be used to position
    the box to get more accurate reflections.

    \sa parallaxCorrection
*/
QVector3D QQuick3DReflectionProbe::boxOffset() const
{
    return m_boxOffset;
}

/*!
    \qmlproperty CubeMapTexture ReflectionProbe::texture
    \since 6.5

    Instead of rendering the scene, this cube map texture is used to show reflections
    in objects affected by this reflection probe.

    \sa CubeMapTexture
*/
QQuick3DCubeMapTexture *QQuick3DReflectionProbe::texture() const
{
    return m_texture;
}

/*!
    \qmlmethod void ReflectionProbe::scheduleUpdate()

    Updates the reflection probe render when called while \l ReflectionProbe::refreshMode
    is set as \c ReflectionProbe.FirstFrame.
*/
void QQuick3DReflectionProbe::scheduleUpdate()
{
    m_dirtyFlags.setFlag(DirtyFlag::RefreshModeDirty);
    update();
}

void QQuick3DReflectionProbe::setQuality(QQuick3DReflectionProbe::ReflectionQuality reflectionQuality)
{
    if (m_quality == reflectionQuality)
        return;

    m_quality = reflectionQuality;
    m_dirtyFlags.setFlag(DirtyFlag::QualityDirty);
    emit qualityChanged();
    update();
}

void QQuick3DReflectionProbe::setClearColor(const QColor &clearColor)
{
    if (m_clearColor == clearColor)
        return;
    m_clearColor = clearColor;
    m_dirtyFlags.setFlag(DirtyFlag::ClearColorDirty);
    emit clearColorChanged();
    update();
}

void QQuick3DReflectionProbe::setRefreshMode(ReflectionRefreshMode newRefreshMode)
{
    if (m_refreshMode == newRefreshMode)
        return;
    m_refreshMode = newRefreshMode;
    m_dirtyFlags.setFlag(DirtyFlag::RefreshModeDirty);
    emit refreshModeChanged();
    update();
}

void QQuick3DReflectionProbe::setTimeSlicing(ReflectionTimeSlicing newTimeSlicing)
{
    if (m_timeSlicing == newTimeSlicing)
        return;
    m_timeSlicing = newTimeSlicing;
    m_dirtyFlags.setFlag(DirtyFlag::TimeSlicingDirty);
    emit timeSlicingChanged();
    update();
}

void QQuick3DReflectionProbe::setParallaxCorrection(bool parallaxCorrection)
{
    if (m_parallaxCorrection == parallaxCorrection)
        return;
    m_parallaxCorrection = parallaxCorrection;
    m_dirtyFlags.setFlag(DirtyFlag::ParallaxCorrectionDirty);
    emit parallaxCorrectionChanged();
    update();
}

void QQuick3DReflectionProbe::setBoxSize(const QVector3D &boxSize)
{
    if (m_boxSize == boxSize)
        return;
    m_boxSize = boxSize;
    m_dirtyFlags.setFlag(DirtyFlag::BoxDirty);
    emit boxSizeChanged();
    createDebugView();
    updateDebugView();
    update();
}

void QQuick3DReflectionProbe::setDebugView(bool debugView)
{
    if (m_debugView == debugView)
        return;
    m_debugView = debugView;
    emit debugViewChanged();
    createDebugView();
    updateDebugView();
}

void QQuick3DReflectionProbe::setBoxOffset(const QVector3D &boxOffset)
{
    if (m_boxOffset == boxOffset)
        return;
    m_boxOffset = boxOffset;
    m_dirtyFlags.setFlag(DirtyFlag::BoxDirty);
    emit boxOffsetChanged();
    createDebugView();
    updateDebugView();
    update();
}

void QQuick3DReflectionProbe::setTexture(QQuick3DCubeMapTexture *newTexture)
{
    if (m_texture == newTexture)
        return;
    QQuick3DObjectPrivate::attachWatcher(this, &QQuick3DReflectionProbe::setTexture, newTexture, m_texture);
    m_texture = newTexture;
    m_dirtyFlags.setFlag(DirtyFlag::TextureDirty);
    emit textureChanged();
    update();
}

QSSGRenderGraphObject *QQuick3DReflectionProbe::updateSpatialNode(QSSGRenderGraphObject *node)
{
    if (!node) {
        markAllDirty();
        node = new QSSGRenderReflectionProbe();
    }

    QQuick3DNode::updateSpatialNode(node);

    QSSGRenderReflectionProbe *probe = static_cast<QSSGRenderReflectionProbe *>(node);

    if (m_dirtyFlags.testFlag(DirtyFlag::QualityDirty)) {
        m_dirtyFlags.setFlag(DirtyFlag::QualityDirty, false);
        probe->reflectionMapRes = mapToReflectionResolution(m_quality);
    }

    if (m_dirtyFlags.testFlag(DirtyFlag::ClearColorDirty)) {
        m_dirtyFlags.setFlag(DirtyFlag::ClearColorDirty, false);
        probe->clearColor = m_clearColor;
    }

    if (m_dirtyFlags.testFlag(DirtyFlag::RefreshModeDirty)) {
        m_dirtyFlags.setFlag(DirtyFlag::RefreshModeDirty, false);
        switch (m_refreshMode) {
        case ReflectionRefreshMode::FirstFrame:
            probe->refreshMode = QSSGRenderReflectionProbe::ReflectionRefreshMode::FirstFrame;
            break;
        case ReflectionRefreshMode::EveryFrame:
            probe->refreshMode = QSSGRenderReflectionProbe::ReflectionRefreshMode::EveryFrame;
            break;
        }
        probe->hasScheduledUpdate = true;
    }

    if (m_dirtyFlags.testFlag(DirtyFlag::TimeSlicingDirty)) {
        m_dirtyFlags.setFlag(DirtyFlag::TimeSlicingDirty, false);
        switch (m_timeSlicing) {
        case ReflectionTimeSlicing::None:
            probe->timeSlicing = QSSGRenderReflectionProbe::ReflectionTimeSlicing::None;
            break;
        case ReflectionTimeSlicing::AllFacesAtOnce:
            probe->timeSlicing = QSSGRenderReflectionProbe::ReflectionTimeSlicing::AllFacesAtOnce;
            break;
        case ReflectionTimeSlicing::IndividualFaces:
            probe->timeSlicing = QSSGRenderReflectionProbe::ReflectionTimeSlicing::IndividualFaces;
            break;
        }
    }

    if (m_dirtyFlags.testFlag(DirtyFlag::ParallaxCorrectionDirty)) {
        m_dirtyFlags.setFlag(DirtyFlag::ParallaxCorrectionDirty, false);
        probe->parallaxCorrection = m_parallaxCorrection;
    }

    if (m_dirtyFlags.testFlag(DirtyFlag::BoxDirty)) {
        m_dirtyFlags.setFlag(DirtyFlag::BoxDirty, false);
        probe->boxSize = m_boxSize;
        probe->boxOffset = m_boxOffset;
    }

    if (m_dirtyFlags.testFlag(DirtyFlag::TextureDirty)) {
        m_dirtyFlags.setFlag(DirtyFlag::TextureDirty, false);
        if (m_texture)
            probe->texture = m_texture->getRenderImage();
        else
            probe->texture = nullptr;
    }

    return node;
}

void QQuick3DReflectionProbe::markAllDirty()
{
    m_dirtyFlags = QQuick3DReflectionProbe::AllDirty;

    QQuick3DNode::markAllDirty();
}

void QQuick3DReflectionProbe::findSceneView()
{
    // If we have not specified a scene view we find the first available one
    if (m_sceneView != nullptr)
        return;

    QObject *parent = this;
    while (parent->parent() != nullptr) {
        parent = parent->parent();
    }

    // Breath-first search through children
    QList<QObject *> queue;
    queue.append(parent);
    while (!queue.empty()) {
        auto node = queue.takeFirst();
        if (auto converted = qobject_cast<QQuick3DViewport *>(node); converted != nullptr) {
            m_sceneView = converted;
            break;
        }
        queue.append(node->children());
    }
}

void QQuick3DReflectionProbe::createDebugView()
{

    if (m_debugView) {
        findSceneView();
        if (!m_sceneView) {
            qWarning() << "ReflectionProbe: Can not create debug view. A root View3D could not be found.";
            return;
        }

        if (!m_debugViewGeometry)
            m_debugViewGeometry = new QQuick3DGeometry(this);

        m_debugViewGeometry->clear();
        m_debugViewGeometry->addAttribute(QQuick3DGeometry::Attribute::PositionSemantic, 0,
                                          QQuick3DGeometry::Attribute::ComponentType::F32Type);
        m_debugViewGeometry->setStride(12);
        m_debugViewGeometry->setPrimitiveType(QQuick3DGeometry::PrimitiveType::Lines);

        QVector<QVector3D> m_vertices;
        //Lines
        m_vertices.append(QVector3D(-0.5, -0.5, 0.5));
        m_vertices.append(QVector3D(0.5, -0.5, 0.5));

        m_vertices.append(QVector3D(0.5, -0.5, 0.5));
        m_vertices.append(QVector3D(0.5, 0.5, 0.5));

        m_vertices.append(QVector3D(0.5, 0.5, 0.5));
        m_vertices.append(QVector3D(-0.5, 0.5, 0.5));

        m_vertices.append(QVector3D(-0.5, 0.5, 0.5));
        m_vertices.append(QVector3D(-0.5, -0.5, 0.5));

        m_vertices.append(QVector3D(-0.5, -0.5, -0.5));
        m_vertices.append(QVector3D(0.5, -0.5, -0.5));

        m_vertices.append(QVector3D(0.5, -0.5, -0.5));
        m_vertices.append(QVector3D(0.5, 0.5, -0.5));

        m_vertices.append(QVector3D(0.5, 0.5, -0.5));
        m_vertices.append(QVector3D(-0.5, 0.5, -0.5));

        m_vertices.append(QVector3D(-0.5, 0.5, -0.5));
        m_vertices.append(QVector3D(-0.5, -0.5, -0.5));

        m_vertices.append(QVector3D(-0.5, 0.5, -0.5));
        m_vertices.append(QVector3D(-0.5, 0.5, 0.5));

        m_vertices.append(QVector3D(0.5, 0.5, -0.5));
        m_vertices.append(QVector3D(0.5, 0.5, 0.5));

        m_vertices.append(QVector3D(-0.5, -0.5, -0.5));
        m_vertices.append(QVector3D(-0.5, -0.5, 0.5));

        m_vertices.append(QVector3D(0.5, -0.5, -0.5));
        m_vertices.append(QVector3D(0.5, -0.5, 0.5));

        QByteArray vertexData;
        vertexData.resize(m_vertices.size() * 3 * sizeof(float));
        float *data = reinterpret_cast<float *>(vertexData.data());
        for (int i = 0; i < m_vertices.size(); i++) {
            data[0] = m_vertices[i].x();
            data[1] = m_vertices[i].y();
            data[2] = m_vertices[i].z();
            data += 3;
        }

        m_debugViewGeometry->setVertexData(vertexData);
        m_debugViewGeometry->update();

        if (!m_debugViewModel) {
            m_debugViewModel = new QQuick3DModel();
            m_debugViewModel->setParentItem(m_sceneView->scene());
            m_debugViewModel->setParent(this);
            m_debugViewModel->setCastsShadows(false);
            m_debugViewModel->setCastsReflections(false);
            m_debugViewModel->setGeometry(m_debugViewGeometry);
        }

        if (!m_debugViewMaterial) {
            m_debugViewMaterial = new QQuick3DDefaultMaterial();
            m_debugViewMaterial->setParentItem(m_debugViewModel);
            m_debugViewMaterial->setParent(m_debugViewModel);
            m_debugViewMaterial->setDiffuseColor(QColor(3, 252, 219));
            m_debugViewMaterial->setLighting(QQuick3DDefaultMaterial::NoLighting);
            m_debugViewMaterial->setCullMode(QQuick3DMaterial::NoCulling);
            QQmlListReference materialsRef(m_debugViewModel, "materials");
            materialsRef.append(m_debugViewMaterial);
        }
    } else {
        if (m_debugViewModel) {
            delete m_debugViewModel;
            m_debugViewModel = nullptr;
            m_debugViewMaterial = nullptr;
        }

        if (m_debugViewGeometry) {
            delete m_debugViewGeometry;
            m_debugViewGeometry = nullptr;
        }
    }
}

void QQuick3DReflectionProbe::updateDebugView()
{
    if (m_debugViewModel) {
        m_debugViewModel->setPosition(scenePosition() + m_boxOffset);
        m_debugViewModel->setScale(m_boxSize);
    }
}

quint32 QQuick3DReflectionProbe::mapToReflectionResolution(ReflectionQuality quality)
{
    switch (quality) {
    case ReflectionQuality::Low:
        return 8;
    case ReflectionQuality::Medium:
        return 9;
    case ReflectionQuality::High:
        return 10;
    case ReflectionQuality::VeryHigh:
        return 11;
    default:
        break;
    }
    return 7;
}

void QQuick3DReflectionProbe::updateSceneManager(QQuick3DSceneManager *sceneManager)
{
    // Check all the resource value's scene manager, and update as necessary.
    if (sceneManager)
        QQuick3DObjectPrivate::refSceneManager(m_texture, *sceneManager);
    else
        QQuick3DObjectPrivate::derefSceneManager(m_texture);
}


QT_END_NAMESPACE
