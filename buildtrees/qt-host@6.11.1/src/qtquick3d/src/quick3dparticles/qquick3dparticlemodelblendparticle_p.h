// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QQUICK3DMODELBLENDPARTICLE_H
#define QQUICK3DMODELBLENDPARTICLE_H

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

#include <QColor>
#include <QVector4D>
#include <QMatrix4x4>

#include <QtQuick3DParticles/private/qquick3dparticle_p.h>
#include <QtQuick3DParticles/private/qquick3dparticlesystem_p.h>
#include <QtQuick3DParticles/private/qquick3dparticledata_p.h>
#include <QtQuick3D/private/qquick3dmodel_p.h>
#include <QtQuick3D/private/qquick3dgeometry_p.h>

QT_BEGIN_NAMESPACE

struct QSSGParticleBuffer;
class Q_QUICK3DPARTICLES_EXPORT QQuick3DParticleModelBlendParticle : public QQuick3DParticle
{
    Q_OBJECT
    Q_PROPERTY(QQmlComponent *delegate READ delegate WRITE setDelegate NOTIFY delegateChanged)
    Q_PROPERTY(QQuick3DNode *endNode READ endNode WRITE setEndNode NOTIFY endNodeChanged)
    Q_PROPERTY(ModelBlendMode modelBlendMode READ modelBlendMode WRITE setModelBlendMode NOTIFY modelBlendModeChanged)
    Q_PROPERTY(int endTime READ endTime WRITE setEndTime NOTIFY endTimeChanged)
    Q_PROPERTY(QQuick3DNode *activationNode READ activationNode WRITE setActivationNode NOTIFY activationNodeChanged)
    Q_PROPERTY(ModelBlendEmitMode emitMode READ emitMode WRITE setEmitMode NOTIFY emitModeChanged)
    QML_NAMED_ELEMENT(ModelBlendParticle3D)
    QML_ADDED_IN_VERSION(6, 2)

public:
    QQuick3DParticleModelBlendParticle(QQuick3DNode *parent = nullptr);
    ~QQuick3DParticleModelBlendParticle() override;

    enum ModelBlendMode
    {
        Explode,
        Construct,
        Transfer
    };
    Q_ENUM(ModelBlendMode)

    enum ModelBlendEmitMode
    {
        Sequential,
        Random,
        Activation
    };
    Q_ENUM(ModelBlendEmitMode)

    QQmlComponent *delegate() const;
    QQuick3DNode *endNode() const;
    ModelBlendMode modelBlendMode() const;
    int endTime() const;
    QQuick3DNode *activationNode() const;
    ModelBlendEmitMode emitMode() const;

public Q_SLOTS:
    void setDelegate(QQmlComponent *setDelegate);
    void setEndNode(QQuick3DNode *endNode);
    void setEndTime(int endTime);
    void setModelBlendMode(ModelBlendMode mode);
    void setActivationNode(QQuick3DNode *activationNode);
    void setEmitMode(ModelBlendEmitMode emitMode);

Q_SIGNALS:
    void delegateChanged();
    void blendFactorChanged();
    void endNodeChanged();
    void modelBlendModeChanged();
    void endTimeChanged();
    void activationNodeChanged();
    void emitModeChanged();

protected:
    void itemChange(ItemChange, const ItemChangeData &) override;
    void reset() override;
    bool lastParticle() const;
    void doSetMaxAmount(int amount) override;
    void componentComplete() override;
    int nextCurrentIndex(const QQuick3DParticleEmitter *emitter) override;
    void setParticleData(int particleIndex,
                         const QVector3D &position,
                         const QVector3D &rotation,
                         const QVector4D &color,
                         float size, float age);
    QVector3D particleCenter(int particleIndex) const;
    QVector3D particleEndPosition(int particleIndex) const;
    QVector3D particleEndRotation(int particleIndex) const;
    int randomIndex(int particleIndex);
    void commitParticles()
    {
        markAllDirty();
        update();
    }

private:
    friend class QQuick3DParticleSystem;
    friend class QQuick3DParticleEmitter;

    struct TriangleParticleData
    {
        QVector3D position;
        QVector3D rotation;
        QVector3D center;
        QVector4D color;
        float age = 0.0f;
        float size = 1.0f;
        int emitterIndex = -1;
    };

    struct PerEmitterData
    {
        int particleCount = 0;
        int emitterIndex = -1;
        const QQuick3DParticleEmitter *emitter = nullptr;
    };

    QSSGRenderGraphObject *updateSpatialNode(QSSGRenderGraphObject *node) override;
    void updateParticleBuffer(QSSGParticleBuffer *buffer, const QMatrix4x4 &sceneTransform);
    void regenerate();
    void updateParticles();
    void handleEndNodeChanged();
    PerEmitterData &perEmitterData(int emitterIndex);

    QVector<TriangleParticleData> m_triangleParticleData;
    QVector<QVector3D> m_centerData;
    QHash<QByteArray, QMetaObject::Connection> m_connections;
    QMap<const QQuick3DParticleEmitter *, PerEmitterData> m_perEmitterData;
    QVector<int> m_randomParticles;
    PerEmitterData n_noPerEmitterData;
    int m_nextEmitterIndex = 0;
    QQmlComponent *m_delegate = nullptr;
    QQuick3DModel *m_model = nullptr;
    QQuick3DGeometry *m_modelGeometry = nullptr;
    QQuick3DNode *m_endNode = nullptr;
    QVector3D m_endNodePosition;
    QVector3D m_endNodeRotation;
    QVector3D m_endNodeScale;
    QMatrix4x4 m_endRotationMatrix;
    int m_particleCount = 0;
    ModelBlendMode m_modelBlendMode = Explode;
    int m_endTime = 0;
    bool m_dataChanged = true;
    ModelBlendEmitMode m_emitMode = Sequential;
    QQuick3DNode *m_activationNode = nullptr;
    float m_maxTriangleRadius = 0.f;
};

QT_END_NAMESPACE

#endif
