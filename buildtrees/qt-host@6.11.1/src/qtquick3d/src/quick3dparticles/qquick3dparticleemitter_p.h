// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QQUICK3DPARTICLEEMITTER_H
#define QQUICK3DPARTICLEEMITTER_H

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
#include <QtQuick3DParticles/private/qquick3dparticledirection_p.h>
#include <QtQuick3DParticles/private/qquick3dparticlesystem_p.h>
#include <QtQuick3DParticles/private/qquick3dparticlemodelparticle_p.h>
#include <QtQuick3DParticles/private/qquick3dparticleabstractshape_p.h>
#include <QtQuick3DParticles/private/qquick3dparticleemitburst_p.h>
#include <QtQuick3DParticles/private/qquick3dparticledynamicburst_p.h>

QT_BEGIN_NAMESPACE

class QQuaternion;

class Q_QUICK3DPARTICLES_EXPORT QQuick3DParticleEmitter : public QQuick3DNode
{
    Q_OBJECT
    Q_PROPERTY(QQuick3DParticleSystem *system READ system WRITE setSystem NOTIFY systemChanged)
    Q_PROPERTY(QQmlListProperty<QQuick3DParticleEmitBurst> emitBursts READ emitBursts)
    Q_PROPERTY(QQuick3DParticleDirection *velocity READ velocity WRITE setVelocity NOTIFY velocityChanged)
    Q_PROPERTY(QQuick3DParticle *particle READ particle WRITE setParticle NOTIFY particleChanged)
    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY enabledChanged)
    Q_PROPERTY(QQuick3DParticleAbstractShape *shape READ shape WRITE setShape NOTIFY shapeChanged)
    Q_PROPERTY(float emitRate READ emitRate WRITE setEmitRate NOTIFY emitRateChanged)
    Q_PROPERTY(int lifeSpan READ lifeSpan WRITE setLifeSpan NOTIFY lifeSpanChanged)
    Q_PROPERTY(int lifeSpanVariation READ lifeSpanVariation WRITE setLifeSpanVariation NOTIFY lifeSpanVariationChanged)
    Q_PROPERTY(float particleScale READ particleScale WRITE setParticleScale NOTIFY particleScaleChanged)
    Q_PROPERTY(float particleEndScale READ particleEndScale WRITE setParticleEndScale NOTIFY particleEndScaleChanged)
    Q_PROPERTY(float particleScaleVariation READ particleScaleVariation WRITE setParticleScaleVariation NOTIFY particleScaleVariationChanged)
    Q_PROPERTY(float particleEndScaleVariation READ particleEndScaleVariation WRITE setParticleEndScaleVariation NOTIFY particleEndScaleVariationChanged)
    Q_PROPERTY(QVector3D particleRotation READ particleRotation WRITE setParticleRotation NOTIFY particleRotationChanged)
    Q_PROPERTY(QVector3D particleRotationVariation READ particleRotationVariation WRITE setParticleRotationVariation NOTIFY particleRotationVariationChanged)
    Q_PROPERTY(QVector3D particleRotationVelocity READ particleRotationVelocity WRITE setParticleRotationVelocity NOTIFY particleRotationVelocityChanged)
    Q_PROPERTY(QVector3D particleRotationVelocityVariation READ particleRotationVelocityVariation WRITE setParticleRotationVelocityVariation NOTIFY particleRotationVariationVelocityChanged)
    Q_PROPERTY(float depthBias READ depthBias WRITE setDepthBias NOTIFY depthBiasChanged)
    Q_PROPERTY(bool reversed READ reversed WRITE setReversed NOTIFY reversedChanged REVISION(6, 10))

    Q_PROPERTY(EmitMode emitMode READ emitMode WRITE setEmitMode NOTIFY emitModeChanged REVISION(6, 10))
    QML_NAMED_ELEMENT(ParticleEmitter3D)
    QML_ADDED_IN_VERSION(6, 2)

public:

    enum EmitMode { Default = 0, SurfaceNormal, SurfaceReflected };
    Q_ENUM(EmitMode)

    QQuick3DParticleEmitter(QQuick3DNode *parent = nullptr);
    ~QQuick3DParticleEmitter() override;

    bool enabled() const;
    QQuick3DParticleDirection *velocity() const;
    QQuick3DParticleSystem *system() const;
    float emitRate() const;
    float particleScale() const;
    float particleEndScale() const;
    float particleScaleVariation() const;
    float particleEndScaleVariation() const;
    int lifeSpan() const;
    int lifeSpanVariation() const;
    QQuick3DParticle *particle() const;
    QQuick3DParticleAbstractShape *shape() const;
    QVector3D particleRotation() const;
    QVector3D particleRotationVariation() const;
    QVector3D particleRotationVelocity() const;
    QVector3D particleRotationVelocityVariation() const;
    float depthBias() const;
    Q_REVISION(6, 10) bool reversed() const;

    Q_REVISION(6, 10) EmitMode emitMode() const;
    QQmlListProperty<QQuick3DParticleEmitBurst> emitBursts();

    Q_INVOKABLE virtual void burst(int count);
    Q_INVOKABLE virtual void burst(int count, int duration);
    Q_INVOKABLE virtual void burst(int count, int duration, const QVector3D &position);

public Q_SLOTS:
    void setEnabled(bool enabled);
    void setVelocity(QQuick3DParticleDirection *velocity);
    void setSystem(QQuick3DParticleSystem *system);
    void setEmitRate(float emitRate);
    void setParticleScale(float particleScale);
    void setParticleEndScale(float particleEndScale);
    void setParticleScaleVariation(float particleScaleVariation);
    void setParticleEndScaleVariation(float particleEndScaleVariation);
    void setLifeSpan(int lifeSpan);
    void setLifeSpanVariation(int lifeSpanVariation);
    void setParticle(QQuick3DParticle *particle);
    void setShape(QQuick3DParticleAbstractShape *shape);
    void setParticleRotation(const QVector3D &particleRotation);
    void setParticleRotationVariation(const QVector3D &particleRotationVariation);
    void setParticleRotationVelocity(const QVector3D &particleRotationVelocity);
    void setParticleRotationVelocityVariation(const QVector3D &particleRotationVelocityVariation);
    void setDepthBias(float bias);
    Q_REVISION(6, 10) void setReversed(bool reversed);

    Q_REVISION(6, 10) void setEmitMode(EmitMode mode);
Q_SIGNALS:
    void velocityChanged();
    void systemChanged();
    void emitRateChanged();
    void particleScaleChanged();
    void particleEndScaleChanged();
    void particleScaleVariationChanged();
    void particleEndScaleVariationChanged();
    void lifeSpanChanged();
    void lifeSpanVariationChanged();
    void particleChanged();
    void shapeChanged();
    void particleRotationChanged();
    void particleRotationVariationChanged();
    void particleRotationVelocityChanged();
    void particleRotationVariationVelocityChanged();
    void enabledChanged();
    void depthBiasChanged();
    Q_REVISION(6, 10) void reversedChanged();
    Q_REVISION(6, 10) void emitModeChanged();

protected:
    friend class QQuick3DParticleSystem;
    friend class QQuick3DParticleEmitBurst;
    friend class QQuick3DParticleTrailEmitter;

    void componentComplete() override;
    void registerEmitBurst(QQuick3DParticleEmitBurst *emitBurst);
    void unRegisterEmitBurst(QQuick3DParticleEmitBurst *emitBurst);
    void generateEmitBursts();
    void emitParticle(QQuick3DParticle *particle, float startTime, const QMatrix4x4 &transform, const QQuaternion &parentRotation, const QVector3D &centerPos, int index = -1,
                      const QVector3D &velocity = QVector3D(), const QVector3D &normal = QVector3D());
    void emitParticles();
    void emitActivationNodeParticles(QQuick3DParticleModelBlendParticle *particle);
    void emitParticlesBurst(const QQuick3DParticleEmitBurstData &burst);
    int getEmitAmount();
    int getEmitAmountFromDynamicBursts(int triggerType = 0);

    void reset();

    // EmitBursts - list handling
    void appendEmitBurst(QQuick3DParticleEmitBurst *);
    qsizetype emitBurstCount() const;
    QQuick3DParticleEmitBurst *emitBurst(qsizetype) const;
    void clearEmitBursts();
    void replaceEmitBurst(qsizetype, QQuick3DParticleEmitBurst *);
    void removeLastEmitBurst();

    // EmitBursts - static
    static void appendEmitBurst(QQmlListProperty<QQuick3DParticleEmitBurst> *, QQuick3DParticleEmitBurst *);
    static qsizetype emitBurstCount(QQmlListProperty<QQuick3DParticleEmitBurst> *);
    static QQuick3DParticleEmitBurst *emitBurst(QQmlListProperty<QQuick3DParticleEmitBurst> *, qsizetype);
    static void clearEmitBursts(QQmlListProperty<QQuick3DParticleEmitBurst> *);
    static void replaceEmitBurst(QQmlListProperty<QQuick3DParticleEmitBurst> *, qsizetype, QQuick3DParticleEmitBurst *);
    static void removeLastEmitBurst(QQmlListProperty<QQuick3DParticleEmitBurst> *);

private:
    struct BurstEmitData
    {
        int startTime;
        int endTime;
        int emitAmount;
        int emitCounter = 0;
        int prevBurstTime;
    };
    QQuick3DParticleDirection *m_velocity = nullptr;
    QQuick3DParticleSystem *m_system = nullptr;
    float m_emitRate = 0.0f;
    // Time in ms when emitting last time happened
    int m_prevEmitTime = 0;
    // Time in ms when dynamic burst last time happened
    int m_prevBurstTime = 0;
    float m_particleScale = 1.0f;
    float m_particleEndScale = -1.0f;
    float m_particleScaleVariation = 0.0f;
    float m_particleEndScaleVariation = -1.0f;
    int m_lifeSpan = 1000;
    int m_lifeSpanVariation = 0;
    float m_unemittedF = 0.0f;
    float m_depthBias = 0.0f;
    QQuick3DParticle *m_particle = nullptr;
    QQuick3DParticleAbstractShape *m_shape = nullptr;
    QVector3D m_particleRotation;
    QVector3D m_particleRotationVariation;
    QVector3D m_particleRotationVelocity;
    QVector3D m_particleRotationVelocityVariation;
    bool m_enabled = true;
    const QQuick3DParticleData m_clearData = {};
    bool m_burstGenerated = false;
    QQuick3DNode *m_systemSharedParent = nullptr;
    // This list contains all emit bursts (both QQuick3DParticleEmitBurst and QQuick3DParticleDynamicBurst)
    QList<QQuick3DParticleEmitBurst *> m_emitBursts;
    QList<BurstEmitData> m_burstEmitData;
    bool m_reversed = false;
    EmitMode m_emitMode = EmitMode::Default;
};

QT_END_NAMESPACE

#endif // QQUICK3DPARTICLEEMITTER_H
