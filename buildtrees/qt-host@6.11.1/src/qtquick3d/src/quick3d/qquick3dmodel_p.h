// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QSSGMODEL_H
#define QSSGMODEL_H

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
#include <QtQuick3D/private/qquick3dmaterial_p.h>
#include <QtQuick3D/private/qquick3dgeometry_p.h>
#include <QtQuick3D/private/qquick3dinstancing_p.h>
#include <QtQuick3D/private/qquick3dskeleton_p.h>
#include <QtQuick3D/private/qquick3dskin_p.h>
#include <QtQuick3D/private/qquick3dbakedlightmap_p.h>
#include <QtQuick3D/private/qquick3dmorphtarget_p.h>
#include <QtQuick3DUtils/private/qssgbounds3_p.h>

#include <QtQml/QQmlListProperty>

#include <QtCore/QVector>
#include <QtCore/QList>
#include <QtCore/QUrl>

#include <QtGui/QVector4D>

QT_BEGIN_NAMESPACE

class Q_QUICK3D_EXPORT QQuick3DBounds3
{
    Q_GADGET
    Q_PROPERTY(QVector3D minimum READ minimum CONSTANT)
    Q_PROPERTY(QVector3D maximum READ maximum CONSTANT)
    QML_VALUE_TYPE(bounds)

public:
    QVector3D minimum() const
    {
        return bounds.minimum;
    }

    QVector3D maximum() const
    {
        return bounds.maximum;
    }

    // For compatibility we start with zero values.
    QSSGBounds3 bounds { QVector3D{}, QVector3D{} };
};

class Q_QUICK3D_EXPORT QQuick3DModel : public QQuick3DNode
{
    Q_OBJECT
    Q_PROPERTY(QUrl source READ source WRITE setSource NOTIFY sourceChanged)
    Q_PROPERTY(bool castsShadows READ castsShadows WRITE setCastsShadows NOTIFY castsShadowsChanged)
    Q_PROPERTY(bool receivesShadows READ receivesShadows WRITE setReceivesShadows NOTIFY receivesShadowsChanged)
    Q_PROPERTY(QQmlListProperty<QQuick3DMaterial> materials READ materials)
    Q_PROPERTY(QQmlListProperty<QQuick3DMorphTarget> morphTargets READ morphTargets NOTIFY morphTargetsChanged)
    Q_PROPERTY(bool pickable READ pickable WRITE setPickable NOTIFY pickableChanged)
    Q_PROPERTY(QQuick3DGeometry *geometry READ geometry WRITE setGeometry NOTIFY geometryChanged)
    Q_PROPERTY(QQuick3DInstancing *instancing READ instancing WRITE setInstancing NOTIFY instancingChanged)
    Q_PROPERTY(QQuick3DNode *instanceRoot READ instanceRoot WRITE setInstanceRoot NOTIFY instanceRootChanged)
    Q_PROPERTY(QQuick3DSkeleton *skeleton READ skeleton WRITE setSkeleton NOTIFY skeletonChanged)
    Q_PROPERTY(QQuick3DSkin *skin READ skin WRITE setSkin NOTIFY skinChanged REVISION(6, 4))
    Q_PROPERTY(QList<QMatrix4x4> inverseBindPoses READ inverseBindPoses WRITE setInverseBindPoses NOTIFY inverseBindPosesChanged)
    Q_PROPERTY(QQuick3DBounds3 bounds READ bounds NOTIFY boundsChanged)
    Q_PROPERTY(float depthBias READ depthBias WRITE setDepthBias NOTIFY depthBiasChanged)
    Q_PROPERTY(bool receivesReflections READ receivesReflections WRITE setReceivesReflections NOTIFY receivesReflectionsChanged REVISION(6, 3))
    Q_PROPERTY(bool castsReflections READ castsReflections WRITE setCastsReflections NOTIFY castsReflectionsChanged REVISION(6, 4))
    Q_PROPERTY(bool usedInBakedLighting READ isUsedInBakedLighting WRITE setUsedInBakedLighting NOTIFY usedInBakedLightingChanged REVISION(6, 4))
    Q_PROPERTY(int lightmapBaseResolution READ lightmapBaseResolution WRITE setLightmapBaseResolution NOTIFY lightmapBaseResolutionChanged REVISION(6, 4))
    Q_PROPERTY(QQuick3DBakedLightmap *bakedLightmap READ bakedLightmap WRITE setBakedLightmap NOTIFY bakedLightmapChanged REVISION(6, 4))
    Q_PROPERTY(float instancingLodMin READ instancingLodMin WRITE setInstancingLodMin NOTIFY instancingLodMinChanged REVISION(6, 5))
    Q_PROPERTY(float instancingLodMax READ instancingLodMax WRITE setInstancingLodMax NOTIFY instancingLodMaxChanged REVISION(6, 5))
    Q_PROPERTY(float levelOfDetailBias READ levelOfDetailBias WRITE setLevelOfDetailBias NOTIFY levelOfDetailBiasChanged REVISION(6, 5))
    Q_PROPERTY(float texelsPerUnit READ texelsPerUnit WRITE setTexelsPerUnit NOTIFY texelsPerUnitChanged FINAL REVISION(6, 10))
    Q_PROPERTY(bool motionVectorEnabled READ motionVectorEnabled WRITE setMotionVectorEnabled NOTIFY motionVectorEnabledChanged REVISION(6, 11))
    Q_PROPERTY(float motionVectorScale READ motionVectorScale WRITE setMotionVectorScale NOTIFY motionVectorScaleChanged REVISION(6, 11))
    QML_NAMED_ELEMENT(Model)

public:
    explicit QQuick3DModel(QQuick3DNode *parent = nullptr);
    ~QQuick3DModel() override;

    QUrl source() const;
    bool castsShadows() const;
    bool receivesShadows() const;
    bool pickable() const;
    QQuick3DGeometry *geometry() const;
    QQuick3DSkeleton *skeleton() const;
    QList<QMatrix4x4> inverseBindPoses() const;
    QQuick3DBounds3 bounds() const;
    float depthBias() const;

    QQmlListProperty<QQuick3DMaterial> materials();
    QQmlListProperty<QQuick3DMorphTarget> morphTargets();

    QQuick3DInstancing *instancing() const;
    QQuick3DNode *instanceRoot() const;

    Q_REVISION(6, 3)  bool receivesReflections() const;
    Q_REVISION(6, 4)  bool castsReflections() const;
    Q_REVISION(6, 4)  QQuick3DSkin *skin() const;

    static QString translateMeshSource(const QUrl &source, QObject *contextObject);

    Q_REVISION(6, 4) bool isUsedInBakedLighting() const;
    Q_REVISION(6, 4) int lightmapBaseResolution() const;
    Q_REVISION(6, 4) QQuick3DBakedLightmap *bakedLightmap() const;

    Q_REVISION(6, 5) float instancingLodMin() const;
    Q_REVISION(6, 5) float instancingLodMax() const;
    Q_REVISION(6, 5) float levelOfDetailBias() const;

    Q_REVISION(6, 10) float texelsPerUnit() const;
    Q_REVISION(6, 10) void setTexelsPerUnit(float newTexelsPerUnit);

    Q_REVISION(6, 11) bool motionVectorEnabled() const;
    Q_REVISION(6, 11) float motionVectorScale() const;

public Q_SLOTS:
    void setSource(const QUrl &source);
    void setCastsShadows(bool castsShadows);
    void setReceivesShadows(bool receivesShadows);
    void setPickable(bool pickable);
    void setGeometry(QQuick3DGeometry *geometry);
    void setSkeleton(QQuick3DSkeleton *skeleton);
    void setInverseBindPoses(const QList<QMatrix4x4> &poses);
    void setBounds(const QVector3D &min, const QVector3D &max);
    void setInstancing(QQuick3DInstancing *instancing);
    void setInstanceRoot(QQuick3DNode *instanceRoot);
    void setDepthBias(float bias);
    Q_REVISION(6, 3)  void setReceivesReflections(bool receivesReflections);
    Q_REVISION(6, 4)  void setCastsReflections(bool castsReflections);
    Q_REVISION(6, 4)  void setSkin(QQuick3DSkin *skin);
    Q_REVISION(6, 4) void setUsedInBakedLighting(bool enable);
    Q_REVISION(6, 4) void setLightmapBaseResolution(int resolution);
    Q_REVISION(6, 4) void setBakedLightmap(QQuick3DBakedLightmap *bakedLightmap);

    Q_REVISION(6, 5) void setInstancingLodMin(float minDistance);
    Q_REVISION(6, 5) void setInstancingLodMax(float maxDistance);
    Q_REVISION(6, 5) void setLevelOfDetailBias(float newLevelOfDetailBias);

    Q_REVISION(6, 11) void setMotionVectorScale(float newMotionVectorScale);
    Q_REVISION(6, 11) void setMotionVectorEnabled(bool newMotionVectorEnabled);

Q_SIGNALS:
    void sourceChanged();
    void castsShadowsChanged();
    void receivesShadowsChanged();
    void pickableChanged();
    void geometryChanged();
    void skeletonChanged();
    void inverseBindPosesChanged();
    void boundsChanged();
    void instancingChanged();
    void instanceRootChanged();
    void morphTargetsChanged();
    void depthBiasChanged();
    Q_REVISION(6, 3)  void receivesReflectionsChanged();
    Q_REVISION(6, 4)  void castsReflectionsChanged();
    Q_REVISION(6, 4)  void skinChanged();
    Q_REVISION(6, 4) void usedInBakedLightingChanged();
    Q_REVISION(6, 4) void lightmapBaseResolutionChanged();
    Q_REVISION(6, 4) void bakedLightmapChanged();

    Q_REVISION(6, 5) void instancingLodMinChanged();
    Q_REVISION(6, 5) void instancingLodMaxChanged();
    Q_REVISION(6, 5) void levelOfDetailBiasChanged();

    Q_REVISION(6, 10) void texelsPerUnitChanged();

    Q_REVISION(6, 11) void motionVectorEnabledChanged();
    Q_REVISION(6, 11) void motionVectorScaleChanged();

protected:
    QSSGRenderGraphObject *updateSpatialNode(QSSGRenderGraphObject *node) override;
    void markAllDirty() override;
    void itemChange(ItemChange, const ItemChangeData &) override;

private Q_SLOTS:
    void onMaterialDestroyed(QObject *object);
    void onMorphTargetDestroyed(QObject *object);

private:
    enum QSSGModelDirtyType {
        SourceDirty =            0x00000001,
        MaterialsDirty =         0x00000002,
        ShadowsDirty =           0x00000004,
        PickingDirty =           0x00000008,
        GeometryDirty =          0x00000010,
        SkeletonDirty =          0x00000020,
        PoseDirty =              0x00000040,
        InstancesDirty =         0x00000080,
        MorphTargetsDirty =      0x00000100,
        PropertyDirty =          0x00000200,
        ReflectionDirty =        0x00000400,
        SkinDirty =              0x00000800,
        LodDirty =               0x00001000,
        InstanceRootDirty =      0x00002000,
    };

    QUrl m_source;

    quint32 m_dirtyAttributes = 0xffffffff; // all dirty by default
    void markDirty(QSSGModelDirtyType type);
    void updateSceneManager(QQuick3DSceneManager *sceneManager);

    static void qmlAppendMaterial(QQmlListProperty<QQuick3DMaterial> *list, QQuick3DMaterial *material);
    static QQuick3DMaterial *qmlMaterialAt(QQmlListProperty<QQuick3DMaterial> *list, qsizetype index);
    static qsizetype qmlMaterialsCount(QQmlListProperty<QQuick3DMaterial> *list);
    static void qmlClearMaterials(QQmlListProperty<QQuick3DMaterial> *list);

    struct Material {
        QQuick3DMaterial *material;
        bool refed;
    };
    QVector<Material> m_materials;

    static void qmlAppendMorphTarget(QQmlListProperty<QQuick3DMorphTarget> *list, QQuick3DMorphTarget *morphTarget);
    static QQuick3DMorphTarget *qmlMorphTargetAt(QQmlListProperty<QQuick3DMorphTarget> *list, qsizetype index);
    static qsizetype qmlMorphTargetsCount(QQmlListProperty<QQuick3DMorphTarget> *list);
    static void qmlClearMorphTargets(QQmlListProperty<QQuick3DMorphTarget> *list);
    QVector<QQuick3DMorphTarget *> m_morphTargets;
    QQuick3DGeometry *m_geometry = nullptr;
    QQuick3DBounds3 m_bounds;
    QQuick3DSkeleton *m_skeleton = nullptr;
    QList<QMatrix4x4> m_inverseBindPoses;
    QQuick3DInstancing *m_instancing = nullptr;
    QQuick3DNode *m_instanceRoot = nullptr;
    QMetaObject::Connection m_geometryConnection;
    QMetaObject::Connection m_instancingConnection;
    float m_depthBias = 0.0f;
    bool m_castsShadows = true;
    bool m_receivesShadows = true;
    bool m_pickable = false;
    bool m_receivesReflections = false;
    bool m_castsReflections = true;
    bool m_usedInBakedLighting = false;
    int m_lightmapBaseResolution = 1024;
    QQuick3DBakedLightmap *m_bakedLightmap = nullptr;
    QMetaObject::Connection m_bakedLightmapSignalConnection;
    QQuick3DSkin *m_skin = nullptr;
    float m_instancingLodMin = -1;
    float m_instancingLodMax = -1;
    float m_levelOfDetailBias = 1.0f;
    float m_texelsPerUnit = 0.0f;
    bool m_motionVectorEnabled = true;
    float m_motionVectorScale = 1.0f;
};

QT_END_NAMESPACE

Q_DECLARE_METATYPE(QQuick3DBounds3)

#endif // QSSGMODEL_H
