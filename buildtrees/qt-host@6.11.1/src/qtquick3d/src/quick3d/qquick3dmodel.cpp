// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qquick3dmodel_p.h"
#include "qquick3dobject_p.h"
#include "qquick3dscenemanager_p.h"
#include "qquick3dnode_p_p.h"
#include "qquick3dinstancing_p.h"

#include <QtQuick3DRuntimeRender/private/qssgrendergraphobject_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrendercustommaterial_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrenderdefaultmaterial_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrendermodel_p.h>

#include <QtQuick3DUtils/private/qssgutils_p.h>

#include <QtQml/QQmlFile>

QT_BEGIN_NAMESPACE

/*!
    \qmltype Model
    \inherits Node
    \inqmlmodule QtQuick3D
    \brief Lets you load a 3D model data.

    The Model item makes it possible to load a mesh and modify how its shaded, by adding materials
    to it. For a model to be renderable, it needs at least a mesh and a material.

    \section1 Mesh format and built-in primitives

    The model can load static meshes from storage or one of the built-in primitive types.
    The mesh format used is a run-time format that's native to the engine, but additional formats are
    supported through the asset import tool \l {Balsam Asset Import Tool}{Balsam}.

    The built-in primitives can be loaded by setting the \c source property to one of these values:
    \c {#Rectangle, #Sphere, #Cube, #Cylinder or #Cone}.

    \qml
    Model {
        source: "#Sphere"
    }
    \endqml

    The table below describes the built-in meshes in more detail.

    \table
    \header
        \li Name
        \li Description
        \li No. faces
    \row
        \li #Cone
        \li A cone with a height and diameter of \c{100}. The base is at the
            origin and the cone is pointing upwards in the y direction.
        \li \c{78}
    \row
        \li #Cube
        \li A cube centered at the origin with \c{100} long sides along x, y and z.
        \li \c{12}
    \row
        \li #Cylinder
        \li A cylinder with \c{100} height and diameter centered at the origin
            expanding along the y direction.
        \li \c{316}
    \row
        \li #Rectangle
        \li A rectangle with \c{100} width and height centered at the origin
            lying in the xy plane.
        \li \c{2}
    \row
        \li #Sphere
        \li A sphere with \c{100} diameter centered at the origin.
        \li \c{4900}
    \endtable

    \section2 Custom geometry

    In addition to using static meshes, you can implement a
    \l {QQuick3DGeometry}{custom geometry} provider that provides the model with
    custom vertex data at run-time. See the
    \l {Qt Quick 3D - Custom Geometry Example}{Custom Geometry Example} for an
    example on how to create and use a custom material with your model.

    \section1 Materials

    A model can consist of several sub-meshes, each of which can have its own
    material. The sub-mesh uses a material from the \l{materials} list,
    corresponding to its index. If the number of materials is less than the
    sub-meshes, the last material in the list is used for subsequent sub-meshes.
    This is demonstrated in the
    \l {Qt Quick 3D - Sub-mesh Example}{Sub-mesh example}.

    You can use the following materials with the model item:
    \l {PrincipledMaterial}, \l {DefaultMaterial}, and \l {CustomMaterial}.

    \section1 Picking

    \e Picking is the process of sending a ray through the scene from some
    starting position to find which models intersect with the ray. In
    Qt Quick 3D, the ray is normally sent from the view using 2D coordinates
    resulting from a touch or mouse event. If a model was hit by the ray,
    \l {pickResult} will be returned with a handle to the model and information
    about where the ray hit the model. For models that use
    \l {QQuick3DGeometry}{custom geometry}, the picking is less accurate than
    for static mesh data, as picking is only done against the model's
    \l {bounds}{bounding volume}. If the ray goes through more than one model,
    the closest \l {Model::pickable}{pickable} model is selected.

    Note that for models to be \l {Model::pickable}{pickable}, their
    \l {Model::pickable}{pickable} property must be set to \c true. For more
    information, see \l {Qt Quick 3D - Picking example}.

*/

/*!
    \qmlvaluetype bounds
    \inqmlmodule QtQuick3D
    \since 5.15
    \brief Specifies the bounds of a model.

    bounds specify a bounding box with minimum and maximum points.
    bounds is a readonly property of the model.
*/

/*!
    \qmlproperty vector3d bounds::minimum

    Specifies the minimum point of the model bounds.
    \sa maximum
*/

/*!
    \qmlproperty vector3d bounds::maximum

    Specifies the maximum point of the model bounds.
    \sa minimum
*/

QQuick3DModel::QQuick3DModel(QQuick3DNode *parent)
    : QQuick3DNode(*(new QQuick3DNodePrivate(QQuick3DNodePrivate::Type::Model)), parent) {}

QQuick3DModel::~QQuick3DModel()
{
    disconnect(m_geometryConnection);

    auto matList = materials();
    qmlClearMaterials(&matList);
    auto morphList = morphTargets();
    qmlClearMorphTargets(&morphList);
}

/*!
    \qmlproperty url Model::source

    This property defines the location of the mesh file containing the geometry
    of this Model or one of the built-in primitive meshes listed below
    as described in \l {Mesh format and built-in primitives}.

    \list
    \li "#Rectangle"
    \li "#Sphere"
    \li "#Cube"
    \li "#Cone"
    \li "#Cylinder"
    \endlist
*/

QUrl QQuick3DModel::source() const
{
    return m_source;
}

/*!
    \qmlproperty List<QtQuick3D::Material> Model::materials

    This property contains a list of materials used to render the provided
    geometry. To render anything, there must be at least one material. Normally
    there should be one material for each sub-mesh included in the source
    geometry.

    \sa {Qt Quick 3D - Sub-mesh Example}
*/


QQmlListProperty<QQuick3DMaterial> QQuick3DModel::materials()
{
    return QQmlListProperty<QQuick3DMaterial>(this,
                                            nullptr,
                                            QQuick3DModel::qmlAppendMaterial,
                                            QQuick3DModel::qmlMaterialsCount,
                                            QQuick3DModel::qmlMaterialAt,
                                            QQuick3DModel::qmlClearMaterials);
}

/*!
    \qmlproperty List<QtQuick3D::MorphTarget> Model::morphTargets

    This property contains a list of \l [QtQuick3D QML] {MorphTarget}{MorphTarget}s used to
    render the provided geometry. Meshes should have at least one attribute
    among positions, normals, tangents, binormals, texture coordinates,
    and vertex colors for the morph targets.

    \sa {MorphTarget}
*/

QQmlListProperty<QQuick3DMorphTarget> QQuick3DModel::morphTargets()
{
    return QQmlListProperty<QQuick3DMorphTarget>(this,
                                            nullptr,
                                            QQuick3DModel::qmlAppendMorphTarget,
                                            QQuick3DModel::qmlMorphTargetsCount,
                                            QQuick3DModel::qmlMorphTargetAt,
                                            QQuick3DModel::qmlClearMorphTargets);
}

/*!
    \qmlproperty QtQuick3D::Instancing Model::instancing

    If this property is set, the model will not be rendered normally. Instead, a number of
    instances of the model will be rendered, as defined by the instance table.

    \sa Instancing
*/

QQuick3DInstancing *QQuick3DModel::instancing() const
{
    return m_instancing;
}

/*!
    \qmlproperty QtQuick3D::Node Model::instanceRoot

    This property defines the origin of the instance’s coordinate system.

    See the \l{Transforms and instancing}{overview documentation} for a detailed explanation.

    \sa instancing, Instancing
*/
QQuick3DNode *QQuick3DModel::instanceRoot() const
{
    return m_instanceRoot;
}

// Source URL's need a bit of translation for the engine because of the
// use of fragment syntax for specifiying primitives and sub-meshes
// So we need to check for the fragment before translating to a qmlfile

QString QQuick3DModel::translateMeshSource(const QUrl &source, QObject *contextObject)
{
    QString fragment;
    if (source.hasFragment()) {
        // Check if this is an index, or primitive
        bool isNumber = false;
        source.fragment().toInt(&isNumber);
        fragment = QStringLiteral("#") + source.fragment();
        // If it wasn't an index, then it was a primitive
        if (!isNumber)
            return fragment;
    }

    const QQmlContext *context = qmlContext(contextObject);
    const auto resolvedUrl = context ? context->resolvedUrl(source) : source;
    const auto qmlSource = QQmlFile::urlToLocalFileOrQrc(resolvedUrl);
    return (qmlSource.isEmpty() ? source.path() : qmlSource) + fragment;
}

void QQuick3DModel::markAllDirty()
{
    m_dirtyAttributes = 0xffffffff;
    QQuick3DNode::markAllDirty();
}

/*!
    \qmlproperty bool Model::castsShadows

    When this property is \c true, the geometry of this model is used when
    rendering to the shadow maps, and is also generating shadows in baked
    lighting.

    The default value is \c true.
*/

bool QQuick3DModel::castsShadows() const
{
    return m_castsShadows;
}

/*!
    \qmlproperty bool Model::receivesShadows

    When this property is set to \c true, the model's materials take shadow
    contribution from shadow casting lights into account.

    \note When lightmapping is enabled for this model, fully baked lights with
    Light::bakeMode set to Light.BakeModeAll will always generate (baked)
    shadows on the model, regardless of the value of this property.

    The default value is \c true.
*/

bool QQuick3DModel::receivesShadows() const
{
    return m_receivesShadows;
}

/*!
    \qmlproperty bool Model::pickable

    This property controls whether the model is pickable or not. Set this property
    to \c true to make the model pickable. Models are not pickable by default.

    \sa {View3D::pick}
*/
bool QQuick3DModel::pickable() const
{
    return m_pickable;
}

/*!
    \qmlproperty Geometry Model::geometry

    Specify a custom geometry for the model. The Model::source must be empty when custom geometry
    is used.

*/
QQuick3DGeometry *QQuick3DModel::geometry() const
{
    return m_geometry;
}

/*!
    \qmlproperty Skeleton Model::skeleton

    Contains the skeleton for the model. The Skeleton is used together with \l {inverseBindPoses}
    for \l {Vertex Skinning}{skinning}.

    \note Meshes of the model must have both joints and weights attributes.
    \note If this property is set, skinning animation is enabled. It means
    that \l {Model} is transformed based on \l {Skeleton} ignoring Model's global
    transformation.

    \sa {Model::inverseBindPoses}, {Qt Quick 3D - Simple Skinning Example}
*/
QQuick3DSkeleton *QQuick3DModel::skeleton() const
{
    return m_skeleton;
}

/*!
    \qmlproperty Skin Model::skin

    Contains the skeleton for the model. The Skeleton is used for
    \l {Vertex Skinning}{skinning}.

    \note Meshes of the model must have both joints and weights attributes.
    \note If this property is set, skinning animation is enabled. This means
    the \l {Model} will be transformed based on \l {Skin::joints} and the
    Model's global transformation will be ignored.
    \note If a model has both a skeleton and a skin, then the skin will be used.

    \sa {Model::skeleton} {Qt Quick 3D - Simple Skinning Example}
*/
QQuick3DSkin *QQuick3DModel::skin() const
{
    return m_skin;
}


/*!
    \qmlproperty List<matrix4x4> Model::inverseBindPoses

    This property contains a list of Inverse Bind Pose matrixes used for the
    skeletal animation. Each inverseBindPose matrix means the inverse of the
    global transform of the repective \l {Joint::index} in \l {skeleton}, which
    will be used initially.

    \note This property is only used if the Model::skeleton is valid.
    \note If some of the matrices are not set, identity values will be used.

    \sa {skeleton} {Joint::index}
*/
QList<QMatrix4x4> QQuick3DModel::inverseBindPoses() const
{
    return m_inverseBindPoses;
}

/*!
    \qmlproperty Bounds Model::bounds

    The bounds of the model descibes the extents of the bounding volume around the model.

    \note The bounds might not be immediately available if the model needs to be loaded first.

    \readonly
*/
QQuick3DBounds3 QQuick3DModel::bounds() const
{
    return m_bounds;
}

/*!
    \qmlproperty real Model::depthBias

    Holds the depth bias of the model. Depth bias is added to the object distance from camera when sorting
    objects. This can be used to force rendering order between objects close to each other, that
    might otherwise be rendered in different order in different frames. Negative values cause the
    sorting value to move closer to the camera while positive values move it further from the camera.
*/
float QQuick3DModel::depthBias() const
{
    return m_depthBias;
}

/*!
    \qmlproperty bool Model::receivesReflections

    When this property is set to \c true, the model's materials take reflections contribution from
    a reflection probe. If the model is inside more than one reflection probe at the same time,
    the nearest reflection probe is taken into account.
*/

bool QQuick3DModel::receivesReflections() const
{
    return m_receivesReflections;
}

/*!
    \qmlproperty bool Model::castsReflections
    \since 6.4

    When this property is set to \c true, the model is rendered by reflection probes and can be
    seen in the reflections.
*/
bool QQuick3DModel::castsReflections() const
{
    return m_castsReflections;
}

/*!
    \qmlproperty bool Model::usedInBakedLighting

    When this property is set to \c true, the model contributes to baked
    lighting, such as lightmaps, for example in form of casting shadows or
    indirect light. This setting is independent of controlling lightmap
    generation for the model, use \l bakedLightmap for that.

    The default value is \c false.

    \note The default value is false, because designers and developers must
    always evaluate on a per-model basis if the object is suitable to take part
    in baked lighting.

    \warning Models with dynamically changing properties, for example, animated
    position, rotation, or other properties, are not suitable for participating
    in baked lighting.

    For more information on how to bake lightmaps, see the \l Lightmapper
    documentation.

    This property is relevant only when baking lightmaps. It has no effect
    afterwards, when using the generated lightmaps during rendering.

    \sa Light::bakeMode, bakedLightmap, Lightmapper, {Lightmaps and Global Illumination}
 */

bool QQuick3DModel::isUsedInBakedLighting() const
{
    return m_usedInBakedLighting;
}

/*!
    \qmlproperty int Model::lightmapBaseResolution

    \deprecated [6.10] This has no effect. Use Model::texelsPerUnit instead.
 */
int QQuick3DModel::lightmapBaseResolution() const
{
    return m_lightmapBaseResolution;
}

/*!
    \qmlproperty BakedLightmap Model::bakedLightmap

    When this property is set to a valid, enabled BakedLightmap object, the
    model will get a lightmap generated when baking lighting, the lightmap is
    then stored persistently. When rendering, the model will load and use the
    associated lightmap. The default value is null.

    \note When the intention is to generate a persistently stored lightmap for
    a Model, both bakedLightmap and \l usedInBakedLighting must be set on it,
    in order to indicate that the Model not only participates in the
    lightmapped scene, but also wants a full lightmap to be baked and stored.

    For more information on how to bake lightmaps, see the \l Lightmapper
    documentation.

    This property is relevant both when baking and when using lightmaps. A
    consistent state between the baking run and the subsequent runs that use
    the generated data is essential. For example, changing the lightmap key
    will make it impossible to load the previously generated data. An exception
    is \l {BakedLightmap::}{enabled}, which can be used to dynamically toggle
    the usage of lightmaps (outside of the baking run), but be aware that the
    rendered results will depend on the Lights' \l{Light::bakeMode}{bakeMode}
    settings in the scene.

    \sa usedInBakedLighting, Lightmapper
 */

QQuick3DBakedLightmap *QQuick3DModel::bakedLightmap() const
{
    return m_bakedLightmap;
}

/*!
    \qmlproperty real Model::instancingLodMin

    Defines the minimum distance from camera that an instance of this model is shown.
    Used for a level of detail implementation.
*/
float QQuick3DModel::instancingLodMin() const
{
    return m_instancingLodMin;
}

/*!
    \qmlproperty real Model::instancingLodMax

    Defines the maximum distance from camera that an instance of this model is shown.
    Used for a level of detail implementation.
*/
float QQuick3DModel::instancingLodMax() const
{
    return m_instancingLodMax;
}

void QQuick3DModel::setSource(const QUrl &source)
{
    if (m_source == source)
        return;

    m_source = source;
    emit sourceChanged();
    markDirty(SourceDirty);
    if (QQuick3DObjectPrivate::get(this)->sceneManager)
        QQuick3DObjectPrivate::get(this)->sceneManager->dirtyBoundingBoxList.append(this);
}

void QQuick3DModel::setCastsShadows(bool castsShadows)
{
    if (m_castsShadows == castsShadows)
        return;

    m_castsShadows = castsShadows;
    emit castsShadowsChanged();
    markDirty(ShadowsDirty);
}

void QQuick3DModel::setReceivesShadows(bool receivesShadows)
{
    if (m_receivesShadows == receivesShadows)
        return;

    m_receivesShadows = receivesShadows;
    emit receivesShadowsChanged();
    markDirty(ShadowsDirty);
}

void QQuick3DModel::setPickable(bool isPickable)
{
    if (m_pickable == isPickable)
        return;

    m_pickable = isPickable;
    emit pickableChanged();
    markDirty(PickingDirty);
}

void QQuick3DModel::setGeometry(QQuick3DGeometry *geometry)
{
    if (geometry == m_geometry)
        return;

    // Make sure to disconnect if the geometry gets deleted out from under us
    QQuick3DObjectPrivate::attachWatcher(this, &QQuick3DModel::setGeometry, geometry, m_geometry);

    if (m_geometry)
        QObject::disconnect(m_geometryConnection);
    m_geometry = geometry;

    if (m_geometry) {
        m_geometryConnection
                = QObject::connect(m_geometry, &QQuick3DGeometry::geometryNodeDirty, this, [this]() {
            markDirty(GeometryDirty);
        });
    }
    emit geometryChanged();
    markDirty(GeometryDirty);
}

void QQuick3DModel::setSkeleton(QQuick3DSkeleton *skeleton)
{
    if (skeleton == m_skeleton)
        return;

    // Make sure to disconnect if the skeleton gets deleted out from under us
    QQuick3DObjectPrivate::attachWatcher(this, &QQuick3DModel::setSkeleton, skeleton, m_skeleton);

    m_skeleton = skeleton;

    emit skeletonChanged();
    markDirty(SkeletonDirty);
}

void QQuick3DModel::setSkin(QQuick3DSkin *skin)
{
    if (skin == m_skin)
        return;

    // Make sure to disconnect if the skin gets deleted out from under us
    QQuick3DObjectPrivate::attachWatcher(this, &QQuick3DModel::setSkin, skin, m_skin);

    m_skin = skin;
    emit skinChanged();
    markDirty(SkinDirty);
}

void QQuick3DModel::setInverseBindPoses(const QList<QMatrix4x4> &poses)
{
    if (m_inverseBindPoses == poses)
        return;

    m_inverseBindPoses = poses;
    emit inverseBindPosesChanged();
    markDirty(PoseDirty);
}


void QQuick3DModel::setBounds(const QVector3D &min, const QVector3D &max)
{
    if (!qFuzzyCompare(m_bounds.maximum(), max)
            || !qFuzzyCompare(m_bounds.minimum(), min))  {
        m_bounds.bounds = QSSGBounds3 { min, max };
        emit boundsChanged();
    }
}

void QQuick3DModel::setInstancing(QQuick3DInstancing *instancing)
{
    if (m_instancing == instancing)
        return;

    // Make sure to disconnect if the instance table gets deleted out from under us
    QQuick3DObjectPrivate::attachWatcher(this, &QQuick3DModel::setInstancing, instancing, m_instancing);
    if (m_instancing)
        QObject::disconnect(m_instancingConnection);
    m_instancing = instancing;
    if (m_instancing) {
        m_instancingConnection = QObject::connect
                (m_instancing, &QQuick3DInstancing::instanceNodeDirty,
                 this, [this]{ markDirty(InstancesDirty);});
    }
    markDirty(InstancesDirty);
    emit instancingChanged();
}

void QQuick3DModel::setInstanceRoot(QQuick3DNode *instanceRoot)
{
    if (m_instanceRoot == instanceRoot)
        return;

    QQuick3DObjectPrivate::attachWatcher(this, &QQuick3DModel::setInstanceRoot, instanceRoot, m_instanceRoot);

    m_instanceRoot = instanceRoot;
    markDirty(InstanceRootDirty);
    emit instanceRootChanged();
}

void QQuick3DModel::setDepthBias(float bias)
{
    if (qFuzzyCompare(bias, m_depthBias))
        return;

    m_depthBias = bias;
    markDirty(PropertyDirty);
    emit depthBiasChanged();
}

void QQuick3DModel::setReceivesReflections(bool receivesReflections)
{
    if (m_receivesReflections == receivesReflections)
        return;

    m_receivesReflections = receivesReflections;
    emit receivesReflectionsChanged();
    markDirty(ReflectionDirty);
}

void QQuick3DModel::setCastsReflections(bool castsReflections)
{
    if (m_castsReflections == castsReflections)
        return;
    m_castsReflections = castsReflections;
    emit castsReflectionsChanged();
    markDirty(ReflectionDirty);
}

void QQuick3DModel::setUsedInBakedLighting(bool enable)
{
    if (m_usedInBakedLighting == enable)
        return;

    m_usedInBakedLighting = enable;
    emit usedInBakedLightingChanged();
    markDirty(PropertyDirty);
}

void QQuick3DModel::setLightmapBaseResolution(int resolution)
{
    resolution = qMax(128, resolution);
    if (m_lightmapBaseResolution == resolution)
        return;

    qWarning() << "Model::lightmapBaseResolution is deprecated and will have no effect.";

    m_lightmapBaseResolution = resolution;
    emit lightmapBaseResolutionChanged();
    markDirty(PropertyDirty);
}

void QQuick3DModel::setBakedLightmap(QQuick3DBakedLightmap *bakedLightmap)
{
    if (m_bakedLightmap == bakedLightmap)
        return;

    if (m_bakedLightmap)
        m_bakedLightmap->disconnect(m_bakedLightmapSignalConnection);

    m_bakedLightmap = bakedLightmap;

    m_bakedLightmapSignalConnection = QObject::connect(m_bakedLightmap, &QQuick3DBakedLightmap::changed, this,
                                                       [this] { markDirty(PropertyDirty); });

    QObject::connect(m_bakedLightmap, &QObject::destroyed, this,
                     [this]
    {
        m_bakedLightmap = nullptr;
        markDirty(PropertyDirty);
    });

    emit bakedLightmapChanged();
    markDirty(PropertyDirty);
}

void QQuick3DModel::itemChange(ItemChange change, const ItemChangeData &value)
{
    if (change == QQuick3DObject::ItemSceneChange)
        updateSceneManager(value.sceneManager);
}

QSSGRenderGraphObject *QQuick3DModel::updateSpatialNode(QSSGRenderGraphObject *node)
{
    if (!node) {
        markAllDirty();
        node = new QSSGRenderModel();
    }

    QQuick3DNode::updateSpatialNode(node);
    int dirtyAttribute = 0;

    auto modelNode = static_cast<QSSGRenderModel *>(node);
    if (m_dirtyAttributes & SourceDirty) {
        const QString path = translateMeshSource(m_source, this);
        const QString lightmapKey = m_bakedLightmap ? m_bakedLightmap->key() : QString();
        modelNode->meshPath = QSSGRenderPath(path, lightmapKey);
    }
    if (m_dirtyAttributes & PickingDirty)
        modelNode->setState(QSSGRenderModel::LocalState::Pickable, m_pickable);

    if (m_dirtyAttributes & ShadowsDirty) {
        modelNode->castsShadows = m_castsShadows;
        modelNode->receivesShadows = m_receivesShadows;
    }

    if (m_dirtyAttributes & MaterialsDirty) {
        if (!m_materials.isEmpty()) {
            if (modelNode->materials.isEmpty()) {
                // Easy mode, just add each material
                for (const Material &material : m_materials) {
                    QSSGRenderGraphObject *graphObject = QQuick3DObjectPrivate::get(material.material)->spatialNode;
                    if (graphObject)
                        modelNode->materials.append(graphObject);
                    else
                        dirtyAttribute |= MaterialsDirty; // We still got dirty materials
                }
            } else {
                // Hard mode, go through each material and see if they match
                if (modelNode->materials.size() != m_materials.size())
                    modelNode->materials.resize(m_materials.size());
                for (int i = 0; i < m_materials.size(); ++i) {
                    QSSGRenderGraphObject *graphObject = QQuick3DObjectPrivate::get(m_materials[i].material)->spatialNode;
                    if (modelNode->materials[i] != graphObject)
                        modelNode->materials[i] = graphObject;
                }
            }
        } else {
            // No materials
            modelNode->materials.clear();
        }
    }

    if (m_dirtyAttributes & MorphTargetsDirty) {
        if (!m_morphTargets.isEmpty()) {
            const int numMorphTarget = m_morphTargets.size();
            if (modelNode->morphTargets.isEmpty()) {
                // Easy mode, just add each morphTarget
                for (const auto morphTarget : std::as_const(m_morphTargets)) {
                    QSSGRenderGraphObject *graphObject = QQuick3DObjectPrivate::get(morphTarget)->spatialNode;
                    if (graphObject)
                        modelNode->morphTargets.append(graphObject);
                    else
                        dirtyAttribute |= MorphTargetsDirty; // We still got dirty morphTargets
                }
                modelNode->morphWeights.resize(numMorphTarget);
                modelNode->morphAttributes.resize(numMorphTarget);
            } else {
                // Hard mode, go through each morphTarget and see if they match
                if (modelNode->morphTargets.size() != numMorphTarget) {
                    modelNode->morphTargets.resize(numMorphTarget);
                    modelNode->morphWeights.resize(numMorphTarget);
                    modelNode->morphAttributes.resize(numMorphTarget);
                }
                for (int i = 0; i < numMorphTarget; ++i)
                    modelNode->morphTargets[i] = QQuick3DObjectPrivate::get(m_morphTargets.at(i))->spatialNode;
            }
        } else {
            // No morphTargets
            modelNode->morphTargets.clear();
        }
    }

    if (m_dirtyAttributes & quint32(InstancesDirty | InstanceRootDirty)) {
        // If we have an instance root set we have lower priority and the instance root node should already
        // have been created.
        QSSGRenderNode *instanceRootNode = nullptr;
        if (m_instanceRoot) {
            if (m_instanceRoot == this)
                instanceRootNode = modelNode;
            else
                instanceRootNode = static_cast<QSSGRenderNode *>(QQuick3DObjectPrivate::get(m_instanceRoot)->spatialNode);
        }
        if (instanceRootNode != modelNode->instanceRoot) {
            modelNode->instanceRoot = instanceRootNode;
            modelNode->markDirty(QSSGRenderNode::DirtyFlag::TransformDirty);
        }

        if (m_instancing) {
            modelNode->instanceTable = static_cast<QSSGRenderInstanceTable *>(QQuick3DObjectPrivate::get(m_instancing)->spatialNode);
        } else {
            modelNode->instanceTable = nullptr;
        }
    }

    if (m_dirtyAttributes & GeometryDirty) {
        if (m_geometry) {
            modelNode->geometry = static_cast<QSSGRenderGeometry *>(QQuick3DObjectPrivate::get(m_geometry)->spatialNode);
            setBounds(m_geometry->boundsMin(), m_geometry->boundsMax());
        } else {
            modelNode->geometry = nullptr;
            setBounds(QVector3D(), QVector3D());
        }
    }

    if (m_dirtyAttributes & SkeletonDirty) {
        if (m_skeleton) {
            modelNode->skeleton = static_cast<QSSGRenderSkeleton *>(QQuick3DObjectPrivate::get(m_skeleton)->spatialNode);
            if (modelNode->skeleton)
                modelNode->skeleton->skinningDirty = true;
        } else {
            modelNode->skeleton = nullptr;
        }
    }

    if (m_dirtyAttributes & SkinDirty) {
        if (m_skin)
            modelNode->skin = static_cast<QSSGRenderSkin *>(QQuick3DObjectPrivate::get(m_skin)->spatialNode);
        else
            modelNode->skin = nullptr;
    }

    if (m_dirtyAttributes & LodDirty) {
        modelNode->instancingLodMin = m_instancingLodMin;
        modelNode->instancingLodMax = m_instancingLodMax;
    }

    if (m_dirtyAttributes & PoseDirty) {
        modelNode->inverseBindPoses = m_inverseBindPoses.toVector();
        if (modelNode->skeleton)
            modelNode->skeleton->skinningDirty = true;
    }

    if (m_dirtyAttributes & PropertyDirty) {
        modelNode->m_depthBiasSq = QSSGRenderModel::signedSquared(m_depthBias);
        modelNode->usedInBakedLighting = m_usedInBakedLighting;
        modelNode->texelsPerUnit = m_texelsPerUnit;
        if (m_bakedLightmap && m_bakedLightmap->isEnabled()) {
            modelNode->lightmapKey = m_bakedLightmap->key();
        } else {
            modelNode->lightmapKey.clear();
        }
        // Need new hash if lightmapKey has changed
        const QString path = translateMeshSource(m_source, this);
        modelNode->meshPath = QSSGRenderPath(path, modelNode->lightmapKey);
        modelNode->levelOfDetailBias = m_levelOfDetailBias;
        modelNode->motionVectorEnabled = m_motionVectorEnabled;
        modelNode->motionVectorScale = m_motionVectorScale;
    }

    if (m_dirtyAttributes & ReflectionDirty) {
        modelNode->receivesReflections = m_receivesReflections;
        modelNode->castsReflections = m_castsReflections;
    }

    m_dirtyAttributes = dirtyAttribute;

    return modelNode;
}

void QQuick3DModel::markDirty(QQuick3DModel::QSSGModelDirtyType type)
{
    if (InstanceRootDirty & quint32(type))
        QQuick3DObjectPrivate::get(this)->dirty(QQuick3DObjectPrivate::InstanceRootChanged);

    if (!(m_dirtyAttributes & quint32(type))) {
        m_dirtyAttributes |= quint32(type);
        update();
    }
}

void QQuick3DModel::updateSceneManager(QQuick3DSceneManager *sceneManager)
{
    if (sceneManager) {
        sceneManager->dirtyBoundingBoxList.append(this);
        QQuick3DObjectPrivate::refSceneManager(m_skeleton, *sceneManager);
        QQuick3DObjectPrivate::refSceneManager(m_skin, *sceneManager);
        QQuick3DObjectPrivate::refSceneManager(m_geometry, *sceneManager);
        QQuick3DObjectPrivate::refSceneManager(m_instancing, *sceneManager);
        for (Material &mat : m_materials) {
            if (!mat.material->parentItem() && !QQuick3DObjectPrivate::get(mat.material)->sceneManager) {
                if (!mat.refed) {
                    QQuick3DObjectPrivate::refSceneManager(mat.material, *sceneManager);
                    mat.refed = true;
                }
            }
        }
    } else {
        QQuick3DObjectPrivate::derefSceneManager(m_skeleton);
        QQuick3DObjectPrivate::derefSceneManager(m_skin);
        QQuick3DObjectPrivate::derefSceneManager(m_geometry);
        QQuick3DObjectPrivate::derefSceneManager(m_instancing);
        for (Material &mat : m_materials) {
            if (mat.refed) {
                QQuick3DObjectPrivate::derefSceneManager(mat.material);
                mat.refed = false;
            }
        }
    }
}

void QQuick3DModel::onMaterialDestroyed(QObject *object)
{
    bool found = false;
    for (int i = 0; i < m_materials.size(); ++i) {
        if (m_materials[i].material == object) {
            m_materials.removeAt(i--);
            found = true;
        }
    }
    if (found)
        markDirty(QQuick3DModel::MaterialsDirty);
}

void QQuick3DModel::qmlAppendMaterial(QQmlListProperty<QQuick3DMaterial> *list, QQuick3DMaterial *material)
{
    if (material == nullptr)
        return;
    QQuick3DModel *self = static_cast<QQuick3DModel *>(list->object);
    self->m_materials.push_back({ material, false });
    self->markDirty(QQuick3DModel::MaterialsDirty);

    if (material->parentItem() == nullptr) {
        // If the material has no parent, check if it has a hierarchical parent that's a QQuick3DObject
        // and re-parent it to that, e.g., inline materials
        QQuick3DObject *parentItem = qobject_cast<QQuick3DObject *>(material->parent());
        if (parentItem) {
            material->setParentItem(parentItem);
        } else { // If no valid parent was found, make sure the material refs our scene manager
            const auto &sceneManager = QQuick3DObjectPrivate::get(self)->sceneManager;
            if (sceneManager) {
                QQuick3DObjectPrivate::get(material)->refSceneManager(*sceneManager);
                // Have to keep track if we called refSceneManager because we
                // can end up in double deref attempts when a model is going
                // away, due to updateSceneManager() being called on
                // ItemSceneChanged (and also doing deref). We must ensure that
                // is one deref for each ref.
                self->m_materials.last().refed = true;
            }
            // else: If there's no scene manager, defer until one is set, see itemChange()
        }
    }

    // Make sure materials are removed when destroyed
    connect(material, &QQuick3DMaterial::destroyed, self, &QQuick3DModel::onMaterialDestroyed);
}

QQuick3DMaterial *QQuick3DModel::qmlMaterialAt(QQmlListProperty<QQuick3DMaterial> *list, qsizetype index)
{
    QQuick3DModel *self = static_cast<QQuick3DModel *>(list->object);
    return self->m_materials.at(index).material;
}

qsizetype QQuick3DModel::qmlMaterialsCount(QQmlListProperty<QQuick3DMaterial> *list)
{
    QQuick3DModel *self = static_cast<QQuick3DModel *>(list->object);
    return self->m_materials.size();
}

void QQuick3DModel::qmlClearMaterials(QQmlListProperty<QQuick3DMaterial> *list)
{
    QQuick3DModel *self = static_cast<QQuick3DModel *>(list->object);
    for (Material &mat : self->m_materials) {
        if (mat.material->parentItem() == nullptr) {
            if (mat.refed) {
                QQuick3DObjectPrivate::get(mat.material)->derefSceneManager();
                mat.refed = false;
            }
        }
        disconnect(mat.material, &QQuick3DMaterial::destroyed, self, &QQuick3DModel::onMaterialDestroyed);
    }
    self->m_materials.clear();
    self->markDirty(QQuick3DModel::MaterialsDirty);
}

void QQuick3DModel::onMorphTargetDestroyed(QObject *object)
{
    bool found = false;
    for (int i = 0; i < m_morphTargets.size(); ++i) {
        if (m_morphTargets.at(i) == object) {
            m_morphTargets.removeAt(i--);
            found = true;
        }
    }
    if (found) {
        markDirty(QQuick3DModel::MorphTargetsDirty);
    }
}

void QQuick3DModel::qmlAppendMorphTarget(QQmlListProperty<QQuick3DMorphTarget> *list, QQuick3DMorphTarget *morphTarget)
{
    if (morphTarget == nullptr)
        return;
    QQuick3DModel *self = static_cast<QQuick3DModel *>(list->object);
    self->m_morphTargets.push_back(morphTarget);

    self->markDirty(QQuick3DModel::MorphTargetsDirty);

    if (morphTarget->parentItem() == nullptr) {
        // If the morphTarget has no parent, check if it has a hierarchical parent that's a QQuick3DObject
        // and re-parent it to that, e.g., inline morphTargets
        QQuick3DObject *parentItem = qobject_cast<QQuick3DObject *>(morphTarget->parent());
        if (parentItem) {
            morphTarget->setParentItem(parentItem);
        } else { // If no valid parent was found, make sure the morphTarget refs our scene manager
            const auto &scenManager = QQuick3DObjectPrivate::get(self)->sceneManager;
            if (scenManager)
                QQuick3DObjectPrivate::get(morphTarget)->refSceneManager(*scenManager);
            // else: If there's no scene manager, defer until one is set, see itemChange()
        }
    }

    // Make sure morphTargets are removed when destroyed
    connect(morphTarget, &QQuick3DMorphTarget::destroyed, self, &QQuick3DModel::onMorphTargetDestroyed);
}

QQuick3DMorphTarget *QQuick3DModel::qmlMorphTargetAt(QQmlListProperty<QQuick3DMorphTarget> *list, qsizetype index)
{
    QQuick3DModel *self = static_cast<QQuick3DModel *>(list->object);
    if (index >= self->m_morphTargets.size()) {
        qWarning("The index exceeds the range of valid morph targets.");
        return nullptr;
    }
    return self->m_morphTargets.at(index);
}

qsizetype QQuick3DModel::qmlMorphTargetsCount(QQmlListProperty<QQuick3DMorphTarget> *list)
{
    QQuick3DModel *self = static_cast<QQuick3DModel *>(list->object);
    return self->m_morphTargets.size();
}

void QQuick3DModel::qmlClearMorphTargets(QQmlListProperty<QQuick3DMorphTarget> *list)
{
    QQuick3DModel *self = static_cast<QQuick3DModel *>(list->object);
    for (const auto &morph : std::as_const(self->m_morphTargets)) {
        if (morph->parentItem() == nullptr)
            QQuick3DObjectPrivate::get(morph)->derefSceneManager();
        disconnect(morph, &QQuick3DMorphTarget::destroyed, self, &QQuick3DModel::onMorphTargetDestroyed);
    }
    self->m_morphTargets.clear();
    self->markDirty(QQuick3DModel::MorphTargetsDirty);
}

void QQuick3DModel::setInstancingLodMin(float minDistance)
{
    if (qFuzzyCompare(m_instancingLodMin, minDistance))
        return;
    m_instancingLodMin = minDistance;
    emit instancingLodMinChanged();
    markDirty(LodDirty);
}

void QQuick3DModel::setInstancingLodMax(float maxDistance)
{
    if (qFuzzyCompare(m_instancingLodMax, maxDistance))
        return;
    m_instancingLodMax = maxDistance;
    emit instancingLodMaxChanged();
    markDirty(LodDirty);
}

/*!
    \qmlproperty real Model::levelOfDetailBias
    \since 6.5

    This property changes the size the model needs to be when rendered before the
    automatic level of detail meshes are used. Each generated level of detail
    mesh contains an ideal size value that each level should be shown, which is
    a ratio of how much of the rendered scene will be that mesh. A model that
    represents only a few pixels on screen will not require the full geometry
    to look correct, so a lower level of detail mesh will be used instead in
    this case. This value is a bias to the ideal value such that a value smaller
    than \c 1.0 will require an even smaller rendered size before switching to
    a lesser level of detail. Values above \c 1.0 will lead to lower levels of detail
    being used sooner. A value of \c 0.0 will disable the usage of levels of detail
    completely.

    The default value is \c 1.0

    \note This property will only have an effect when the Model's geometry contains
    levels of detail.

    \sa Camera::levelOfDetailBias
*/

float QQuick3DModel::levelOfDetailBias() const
{
    return m_levelOfDetailBias;
}

void QQuick3DModel::setLevelOfDetailBias(float newLevelOfDetailBias)
{
    if (qFuzzyCompare(m_levelOfDetailBias, newLevelOfDetailBias))
        return;
    m_levelOfDetailBias = newLevelOfDetailBias;
    emit levelOfDetailBiasChanged();
    markDirty(QQuick3DModel::PropertyDirty);
}

/*!
    \qmlproperty real Model::texelsPerUnit
    \since 6.10
    \default 0.0

    A value greater than zero means this value will override the
    \l {Lightmapper::texelsPerUnit} value for this specific model during
    lightmap baking.

    \sa Lightmapper::texelsPerUnit
*/

float QQuick3DModel::texelsPerUnit() const
{
    return m_texelsPerUnit;
}

void QQuick3DModel::setTexelsPerUnit(float newTexelsPerUnit)
{
    if (qFuzzyCompare(m_texelsPerUnit, newTexelsPerUnit))
        return;
    m_texelsPerUnit = newTexelsPerUnit;
    emit texelsPerUnitChanged();
    markDirty(PropertyDirty);
}

/*!
    \qmlproperty bool Model::motionVectorEnabled
    \since 6.11
    \default true

    Specifies whether the model participates in the motion vector pass.

    When this property is set to \c true, the model contributes its
    motion information to the motion vector buffer. This is used by
    temporal anti-aliasing (TAA) and other effects that rely on
    per-pixel motion data.

    When set to \c false, the model is excluded from motion vector
    generation.

    \sa Model::motionVectorScale
*/

bool QQuick3DModel::motionVectorEnabled() const
{
    return m_motionVectorEnabled;
}

void QQuick3DModel::setMotionVectorEnabled(bool newMotionVectorEnabled)
{
    if (m_motionVectorEnabled == newMotionVectorEnabled)
        return;
    m_motionVectorEnabled = newMotionVectorEnabled;
    emit motionVectorEnabledChanged();
    markDirty(QQuick3DModel::PropertyDirty);
}

/*!
    \qmlproperty real Model::motionVectorScale
    \since 6.11
    \default 1.0

    Specifies a scaling factor applied to the model's generated motion
    vectors.

    A value of \c 1.0 preserves the model's actual motion, while values
    greater than \c 1.0 exaggerate the motion vectors and values lower
    than \c 1.0 reduce them. This can be useful for stylized effects or
    fine-tuning visual behavior in effects that rely on the motion
    vector buffer.

    \sa Model::motionVectorEnabled
*/

float QQuick3DModel::motionVectorScale() const
{
    return m_motionVectorScale;
}

void QQuick3DModel::setMotionVectorScale(float newMotionVectorScale)
{
    if (qFuzzyCompare(m_motionVectorScale, newMotionVectorScale))
        return;
    m_motionVectorScale = newMotionVectorScale;
    emit motionVectorScaleChanged();
    markDirty(QQuick3DModel::PropertyDirty);
}

QT_END_NAMESPACE
