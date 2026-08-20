// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default



#ifndef QQUICK3DRESOURCELOADER_H
#define QQUICK3DRESOURCELOADER_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtQuick3D/private/qquick3dnode_p.h>
#include <QtQuick3D/private/qquick3dgeometry_p.h>
#include <QtQuick3D/private/qquick3dtexture_p.h>
#include <QtQml/QQmlListProperty>

QT_BEGIN_NAMESPACE

class Q_QUICK3D_EXPORT QQuick3DResourceLoader : public QQuick3DObject
{
    Q_OBJECT
    Q_PROPERTY(QList<QUrl> meshSources READ meshSources WRITE setMeshSources NOTIFY meshSourcesChanged)
    Q_PROPERTY(QQmlListProperty<QQuick3DTexture> textures READ textures)
    Q_PROPERTY(QQmlListProperty<QQuick3DGeometry> geometries READ geometries)
    QML_NAMED_ELEMENT(ResourceLoader)
    QML_ADDED_IN_VERSION(6, 3)
public:
    QQuick3DResourceLoader(QQuick3DObject *parent = nullptr);

    const QList<QUrl> &meshSources() const;
    void setMeshSources(const QList<QUrl> &newMeshSources);
    QQmlListProperty<QQuick3DGeometry> geometries();
    QQmlListProperty<QQuick3DTexture> textures();
Q_SIGNALS:
    void meshSourcesChanged();

private Q_SLOTS:
    void onGeometryDestroyed(QObject *object);
    void onTextureDestroyed(QObject *object);
protected:
    QSSGRenderGraphObject *updateSpatialNode(QSSGRenderGraphObject *node) override;
    void markAllDirty() override;
    void itemChange(ItemChange change, const ItemChangeData &value) override;

private:
    static void qmlAppendGeometry(QQmlListProperty<QQuick3DGeometry> *list, QQuick3DGeometry *geometry);
    static QQuick3DGeometry *qmlGeometryAt(QQmlListProperty<QQuick3DGeometry> *list, qsizetype index);
    static qsizetype qmlGeometriesCount(QQmlListProperty<QQuick3DGeometry> *list);
    static void qmlClearGeometries(QQmlListProperty<QQuick3DGeometry> *list);

    static void qmlAppendTexture(QQmlListProperty<QQuick3DTexture> *list, QQuick3DTexture *texture);
    static QQuick3DTexture *qmlTextureAt(QQmlListProperty<QQuick3DTexture> *list, qsizetype index);
    static qsizetype qmlTexturesCount(QQmlListProperty<QQuick3DTexture> *list);
    static void qmlClearTextures(QQmlListProperty<QQuick3DTexture> *list);

    enum ResourceLoaderDirtyType {
        MeshesDirty =       0x00000001,
        TexturesDirty =     0x00000002,
        GeometriesDirty =   0x00000004
    };


    quint32 m_dirtyAttributes = 0xffffffff; // all dirty by default
    void markDirty(ResourceLoaderDirtyType type);
    void updateSceneManager(QQuick3DSceneManager *sceneManager);

    QList<QUrl> m_meshSources;
    QList<QQuick3DGeometry *> m_geometries;
    QList<QQuick3DTexture *> m_textures;
};

QT_END_NAMESPACE

#endif // QQUICK3DRESOURCELOADER_H
