// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default



#include "qquick3dresourceloader_p.h"
#include "qquick3dmodel_p.h"
#include <QtQuick3DRuntimeRender/private/qssgrenderresourceloader_p.h>

QT_BEGIN_NAMESPACE

/*!
    \qmltype ResourceLoader
    \inqmlmodule QtQuick3D
    \inherits Object3D

    \brief Allows pre-loading of 3D resources.

    ResourceLoader is used to pre-load resources for Qt Quick 3D. Normally
    resources are only loaded when they are needed to render a frame, and are
    unloaded when they are not used to render the scene. This aggressive
    approach to resource lifetimes means that only the bare minimum of GPU
    resources are used to render a frame, but for some dynamic scenes this
    can lead to resources being loaded and released frequently. The
    ResourceLoader component enables a finer grain control on the lifetimes
    of resources in the scene. Resources listed in the ResourceLoader
    component are loaded into GPU memory and will remain there until they
    are removed from the ResourceLoader lists or the ResourceLoader is
    destroyed.

    ResourceLoader can also be used to make sure that large resources are
    available before rendering a frame. Since resources are loaded only
    when needed for a frame, this can lead to frames being dropped waiting
    for a large resource to be loaded. By pre-loading large resources before
    showing a scene, there is no risk of dropping any frames due to resources
    being loaded during an animation.

    For usage examples, see \l {Qt Quick 3D - Principled Material Example}

*/

/*!
    \qmlproperty List<url> ResourceLoader::meshSources

    This property defines a list of locations of mesh files containing geometry.
    When a mesh file is added to this list, it will be loaded to the GPU and
    cached. If these same mesh files are source is used by a /c Model they will
    not need to be loaded again.

*/

/*!
    \qmlproperty List<QtQuick3D::Texture> ResourceLoader::textures

    This property defines a list of Texture resources that will be loaded to the
    GPU and cached.

*/

/*!
    \qmlproperty List<QtQuick3D::Geometry> ResourceLoader::geometries

    This property defines a list of Geometry resources that will be loaded to the
    GPU and cached.

*/

QQuick3DResourceLoader::QQuick3DResourceLoader(QQuick3DObject *parent)
    : QQuick3DObject(*(new QQuick3DObjectPrivate(QQuick3DObjectPrivate::Type::ResourceLoader)), parent)
{
}

const QList<QUrl> &QQuick3DResourceLoader::meshSources() const
{
    return m_meshSources;
}

void QQuick3DResourceLoader::setMeshSources(const QList<QUrl> &newMeshSources)
{
    if (m_meshSources == newMeshSources)
        return;
    m_meshSources = newMeshSources;
    emit meshSourcesChanged();
    markDirty(QQuick3DResourceLoader::MeshesDirty);
}


QQmlListProperty<QQuick3DGeometry> QQuick3DResourceLoader::geometries()
{
    return QQmlListProperty<QQuick3DGeometry>(this,
                                  nullptr,
                                  QQuick3DResourceLoader::qmlAppendGeometry,
                                  QQuick3DResourceLoader::qmlGeometriesCount,
                                  QQuick3DResourceLoader::qmlGeometryAt,
                                  QQuick3DResourceLoader::qmlClearGeometries);
}

QQmlListProperty<QQuick3DTexture> QQuick3DResourceLoader::textures()
{
    return QQmlListProperty<QQuick3DTexture>(this,
                                  nullptr,
                                  QQuick3DResourceLoader::qmlAppendTexture,
                                  QQuick3DResourceLoader::qmlTexturesCount,
                                  QQuick3DResourceLoader::qmlTextureAt,
                                  QQuick3DResourceLoader::qmlClearTextures);
}

void QQuick3DResourceLoader::onGeometryDestroyed(QObject *object)
{
    bool found = false;
    for (int i = 0; i < m_geometries.size(); ++i) {
        if (m_geometries[i] == object) {
            m_geometries.removeAt(i--);
            found = true;
        }
    }
    if (found)
        markDirty(QQuick3DResourceLoader::GeometriesDirty);
}

void QQuick3DResourceLoader::onTextureDestroyed(QObject *object)
{
    bool found = false;
    for (int i = 0; i < m_textures.size(); ++i) {
        if (m_textures[i] == object) {
            m_textures.removeAt(i--);
            found = true;
        }
    }
    if (found)
        markDirty(QQuick3DResourceLoader::TexturesDirty);
}

QSSGRenderGraphObject *QQuick3DResourceLoader::updateSpatialNode(QSSGRenderGraphObject *node)
{
    if (!node) {
        markAllDirty();
        node = new QSSGRenderResourceLoader();
    }
    QQuick3DObject::updateSpatialNode(node);
    int dirtyAttribute = 0;

    auto resourceLoaderNode = static_cast<QSSGRenderResourceLoader *>(node);
    if (m_dirtyAttributes & MeshesDirty) {
        resourceLoaderNode->meshes.clear();
        for (const auto &mesh : std::as_const(m_meshSources))
            resourceLoaderNode->meshes.push_back(QSSGRenderPath(QQuick3DModel::translateMeshSource(mesh, this)));
    }

    if (m_dirtyAttributes & TexturesDirty) {
        resourceLoaderNode->textures.clear();
        for (const auto &texture : std::as_const(m_textures)) {
            auto graphObject = QQuick3DObjectPrivate::get(texture)->spatialNode;
            if (graphObject)
                resourceLoaderNode->textures.push_back(graphObject);
            else
                dirtyAttribute |= TexturesDirty;
        }
    }

    if (m_dirtyAttributes & GeometriesDirty) {
        resourceLoaderNode->geometries.clear();
        for (const auto &geometry : std::as_const(m_geometries)) {
            auto graphObject = QQuick3DObjectPrivate::get(geometry)->spatialNode;
            if (graphObject)
                resourceLoaderNode->geometries.push_back(graphObject);
            else
                dirtyAttribute |= GeometriesDirty;
        }
    }

    m_dirtyAttributes = dirtyAttribute;
    return resourceLoaderNode;

}

void QQuick3DResourceLoader::markAllDirty()
{
    m_dirtyAttributes = 0xffffffff;
    QQuick3DObject::markAllDirty();
}

void QQuick3DResourceLoader::itemChange(ItemChange change, const ItemChangeData &value)
{
    if (change == QQuick3DObject::ItemSceneChange)
        updateSceneManager(value.sceneManager);
}

void QQuick3DResourceLoader::qmlAppendGeometry(QQmlListProperty<QQuick3DGeometry> *list, QQuick3DGeometry *geometry)
{
    if (geometry == nullptr)
        return;
    QQuick3DResourceLoader *self = static_cast<QQuick3DResourceLoader *>(list->object);
    self->m_geometries.push_back(geometry);

    self->markDirty(QQuick3DResourceLoader::GeometriesDirty);

    if (geometry->parentItem() == nullptr) {
        // If the material has no parent, check if it has a hierarchical parent that's a QQuick3DObject
        // and re-parent it to that, e.g., inline materials
        QQuick3DObject *parentItem = qobject_cast<QQuick3DObject *>(geometry->parent());
        if (parentItem) {
            geometry->setParentItem(parentItem);
        } else { // If no valid parent was found, make sure the material refs our scene manager
            const auto &sceneManager = QQuick3DObjectPrivate::get(self)->sceneManager;
            if (sceneManager) {
                QQuick3DObjectPrivate::get(geometry)->refSceneManager(*sceneManager);
            }
        }
    }

    // Make sure geometries are removed when destroyed
    connect(geometry, &QQuick3DGeometry::destroyed, self, &QQuick3DResourceLoader::onGeometryDestroyed);
}

QQuick3DGeometry *QQuick3DResourceLoader::qmlGeometryAt(QQmlListProperty<QQuick3DGeometry> *list, qsizetype index)
{
    QQuick3DResourceLoader *self = static_cast<QQuick3DResourceLoader *>(list->object);

    if (index >= self->m_geometries.size()) {
        qWarning("The index exceeds the range of valid geometries.");
        return nullptr;
    }

    return self->m_geometries.at(index);
}

qsizetype QQuick3DResourceLoader::qmlGeometriesCount(QQmlListProperty<QQuick3DGeometry> *list)
{
    QQuick3DResourceLoader *self = static_cast<QQuick3DResourceLoader *>(list->object);
    return self->m_geometries.size();
}

void QQuick3DResourceLoader::qmlClearGeometries(QQmlListProperty<QQuick3DGeometry> *list)
{
    QQuick3DResourceLoader *self = static_cast<QQuick3DResourceLoader *>(list->object);
    for (const auto &geometry : std::as_const(self->m_geometries)) {
        if (geometry->parentItem() == nullptr)
            QQuick3DObjectPrivate::get(geometry)->derefSceneManager();
        disconnect(geometry, &QQuick3DGeometry::destroyed, self, &QQuick3DResourceLoader::onGeometryDestroyed);
    }

    self->m_geometries.clear();
    self->markDirty(QQuick3DResourceLoader::GeometriesDirty);
}

void QQuick3DResourceLoader::qmlAppendTexture(QQmlListProperty<QQuick3DTexture> *list, QQuick3DTexture *texture)
{
    if (texture == nullptr)
        return;
    QQuick3DResourceLoader *self = static_cast<QQuick3DResourceLoader *>(list->object);
    self->m_textures.push_back(texture);

    self->markDirty(QQuick3DResourceLoader::TexturesDirty);

    if (texture->parentItem() == nullptr) {
        // If the material has no parent, check if it has a hierarchical parent that's a QQuick3DObject
        // and re-parent it to that, e.g., inline materials
        QQuick3DObject *parentItem = qobject_cast<QQuick3DObject *>(texture->parent());
        if (parentItem) {
            texture->setParentItem(parentItem);
        } else { // If no valid parent was found, make sure the material refs our scene manager
            const auto &sceneManager = QQuick3DObjectPrivate::get(self)->sceneManager;
            if (sceneManager) {
                QQuick3DObjectPrivate::get(texture)->refSceneManager(*sceneManager);
            }
        }
    }
    // Make sure TextureData are removed when destroyed
    connect(texture, &QQuick3DTextureData::destroyed, self, &QQuick3DResourceLoader::onTextureDestroyed);
}

QQuick3DTexture *QQuick3DResourceLoader::qmlTextureAt(QQmlListProperty<QQuick3DTexture> *list, qsizetype index)
{
    QQuick3DResourceLoader *self = static_cast<QQuick3DResourceLoader *>(list->object);
    if (index >= self->m_textures.size()) {
        qWarning("The index exceeds the range of valid texture data.");
        return nullptr;
    }

    return self->m_textures.at(index);
}

qsizetype QQuick3DResourceLoader::qmlTexturesCount(QQmlListProperty<QQuick3DTexture> *list)
{
    QQuick3DResourceLoader *self = static_cast<QQuick3DResourceLoader *>(list->object);
    return self->m_textures.size();
}

void QQuick3DResourceLoader::qmlClearTextures(QQmlListProperty<QQuick3DTexture> *list)
{
    QQuick3DResourceLoader *self = static_cast<QQuick3DResourceLoader *>(list->object);
    for (const auto &data : std::as_const(self->m_textures)) {
        if (data->parentItem() == nullptr)
            QQuick3DObjectPrivate::get(data)->derefSceneManager();
        disconnect(data, &QQuick3DTextureData::destroyed, self, &QQuick3DResourceLoader::onTextureDestroyed);
    }
    self->m_textures.clear();
    self->markDirty(QQuick3DResourceLoader::TexturesDirty);
}

void QQuick3DResourceLoader::markDirty(ResourceLoaderDirtyType type)
{
    if (!(m_dirtyAttributes & quint32(type))) {
        m_dirtyAttributes |= quint32(type);
        update();
    }
}

void QQuick3DResourceLoader::updateSceneManager(QQuick3DSceneManager *sceneManager)
{
    if (sceneManager) {
        for (auto &geometry : m_geometries)
            if (!geometry->parentItem() && !QQuick3DObjectPrivate::get(geometry)->sceneManager)
                QQuick3DObjectPrivate::refSceneManager(geometry, *sceneManager);
        for (auto &texture : m_textures)
            if (!texture->parentItem() && !QQuick3DObjectPrivate::get(texture)->sceneManager)
                QQuick3DObjectPrivate::refSceneManager(texture, *sceneManager);
    } else {
        for (auto &geometry : m_geometries)
            QQuick3DObjectPrivate::derefSceneManager(geometry);
        for (auto &texture : m_textures)
            QQuick3DObjectPrivate::derefSceneManager(texture);
    }
}

QT_END_NAMESPACE
