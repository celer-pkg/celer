// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QQUICK3DPARTICLESCENESHAPE_H
#define QQUICK3DPARTICLESCENESHAPE_H

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

#include "qquick3dparticleabstractshape_p.h"
#include <QVector3D>
#include <QList>

QT_BEGIN_NAMESPACE

class QQuick3DModel;
class QQuick3DNode;
class QQmlComponent;
class QQuick3DGeometry;

class Q_QUICK3DPARTICLES_EXPORT QQuick3DParticleSceneShape : public QQuick3DParticleAbstractShape
{
    Q_OBJECT
    Q_PROPERTY(QQuick3DNode *scene READ scene WRITE setScene NOTIFY sceneChanged)
    Q_PROPERTY(QVector3D sceneCenter READ sceneCenter WRITE setSceneCenter NOTIFY sceneCenterChanged)
    Q_PROPERTY(QVector3D sceneExtents READ sceneExtents WRITE setSceneExtents NOTIFY sceneExtentsChanged)
    Q_PROPERTY(float shapeResolution READ shapeResolution WRITE setShapeResolution NOTIFY shapeResolutionChanged)
    Q_PROPERTY(QList<QQuick3DNode *> excludedNodes READ excludedNodes WRITE setExcludedNodes NOTIFY excludedNodesChanged)
    Q_PROPERTY(QQuick3DGeometry *geometry READ geometry NOTIFY geometryChanged)
    QML_NAMED_ELEMENT(ParticleSceneShape3D)
    QML_ADDED_IN_VERSION(6, 11)

public:
    explicit QQuick3DParticleSceneShape(QObject *parent = nullptr);
    ~QQuick3DParticleSceneShape() override;

    QQuick3DNode *scene() const;
    QVector3D sceneCenter() const;
    QVector3D sceneExtents() const;
    float shapeResolution() const;
    QQuick3DGeometry *geometry() const;
    QList<QQuick3DNode *> excludedNodes() const;

public Q_SLOTS:
    void setScene(QQuick3DNode *scene);
    void setSceneCenter(const QVector3D &extents);
    void setSceneExtents(const QVector3D &extents);
    void setShapeResolution(float resolution);
    void setExcludedNodes(const QList<QQuick3DNode *> &nodes);

    // Returns point inside this shape
    QVector3D getPosition(int particleIndex) override;
    QVector3D getSurfaceNormal(int particleIndex) override;

Q_SIGNALS:
    void sceneChanged();
    void sceneCenterChanged();
    void sceneExtentsChanged();
    void shapeResolutionChanged();
    void geometryChanged();
    void excludedNodesChanged();

private:
    QVector3D randomPositionModel(int particleIndex);
    void createShapeData();
    void clearModelVertexPositions();
    void calculateModelVertexPositions();
    QList<QVector3D> excludeTriangles(const QList<QVector3D> &list);
    void updateGeometry();
    bool isVisibleRecursive(QQuick3DNode *node);
    bool isExcluded(QQuick3DModel *model);
    void markDirty(bool dirty = true, bool excludeDirty = false);

    QList<QQuick3DNode *> m_excludeList;
    QQuick3DGeometry *m_geometry = nullptr;
    QQuick3DNode *m_root = nullptr;
    QQuick3DNode *m_scene = nullptr;
    struct ShapeData {
        QQuick3DModel *model;
        QVector<QVector3D> vertexPositions;
        float modelTriangleAreasSum = 0;
        QVector<float> modelTriangleAreas;
        QVector3D modelTriangleCenter;
        bool dirty = true;
        bool hasVertices = false;
        bool visible = false;
        bool excluded = false;
        bool excludedDirty = true;
        QMetaObject::Connection sceneTransformConnection;
        QMetaObject::Connection visibilityConnection;
        void clear()
        {
            vertexPositions.clear();
            modelTriangleAreasSum = 0;
            modelTriangleAreas.clear();
            modelTriangleCenter = {};
        }
    };
    struct SceneNapkinMesh;
    QList<ShapeData> m_data;
    ShapeData m_sceneData;
    SceneNapkinMesh *m_sceneMesh = nullptr;
    QVector3D m_cachedNormal;
    QVector3D m_sceneCenter;
    QVector3D m_sceneExtents;
    float m_shapeResolution = 10.0f;
    int m_cachedIndex = -1;
};

QT_END_NAMESPACE

#endif // QQUICK3DPARTICLESCENESHAPE_H
