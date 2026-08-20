// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QQUICK3DPPARTICLESYSTEM_H
#define QQUICK3DPPARTICLESYSTEM_H

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

#include <QtQuick3DParticles/qtquick3dparticlesglobal.h>
#include <QtQuick3D/private/qquick3dnode_p.h>
#include <QtQuick3D/private/qquick3ddefaultmaterial_p.h>
#include <QtQuick3D/private/qquick3dprincipledmaterial_p.h>
#include <QtQuick3D/private/qquick3dloader_p.h>
#include <QtQuick3DParticles/private/qquick3dparticlesystemlogging_p.h>
#include <QtQuick3DParticles/private/qquick3dparticlerandomizer_p.h>
#include <QtQuick3DParticles/private/qquick3dparticledata_p.h>
#include <QElapsedTimer>
#include <QVector>
#include <QList>
#include <QHash>
#include <QPointer>
#include <QAbstractAnimation>
#include <QtQml/qqml.h>
#include <QElapsedTimer>
#include <QTimer>

QT_BEGIN_NAMESPACE

class QQuick3DParticleSpriteParticle;
class QQuick3DParticleModelParticle;
class QQuick3DParticleModelBlendParticle;
class QQuick3DParticleEmitter;
class QQuick3DParticleTrailEmitter;
class QQuick3DParticleAffector;
class QQuick3DParticleStatelessAffector;

class QQuick3DParticle;
class QQuick3DParticleSystemAnimation;
class QQuick3DParticleSystemUpdate;
class QQuick3DParticleInstanceTable;

class Q_QUICK3DPARTICLES_EXPORT QQuick3DParticleSystem : public QQuick3DNode
{
    Q_OBJECT
    Q_PROPERTY(bool running READ isRunning WRITE setRunning NOTIFY runningChanged)
    Q_PROPERTY(bool paused READ isPaused WRITE setPaused NOTIFY pausedChanged)
    Q_PROPERTY(int startTime READ startTime WRITE setStartTime NOTIFY startTimeChanged)
    Q_PROPERTY(int time READ time WRITE setTime NOTIFY timeChanged)
    Q_PROPERTY(bool useRandomSeed READ useRandomSeed WRITE setUseRandomSeed NOTIFY useRandomSeedChanged)
    Q_PROPERTY(int seed READ seed WRITE setSeed NOTIFY seedChanged)
    Q_PROPERTY(bool logging READ logging WRITE setLogging NOTIFY loggingChanged)
    Q_PROPERTY(QQuick3DParticleSystemLogging *loggingData READ loggingData NOTIFY loggingDataChanged)
    QML_NAMED_ELEMENT(ParticleSystem3D)
    QML_ADDED_IN_VERSION(6, 2)

public:
    QQuick3DParticleSystem(QQuick3DNode *parent = nullptr);
    ~QQuick3DParticleSystem() override;

    bool isRunning() const;
    bool isPaused() const;
    int startTime() const;
    int time() const;
    bool useRandomSeed() const;
    int seed() const;
    // Return the total amount of particles
    int particleCount() const;
    bool logging() const;
    QQuick3DParticleSystemLogging *loggingData() const;

    // Registering of different components into system
    void registerParticle(QQuick3DParticle *particle);
    void unRegisterParticle(QQuick3DParticle *particle);
    void registerParticleEmitter(QQuick3DParticleEmitter* e);
    void unRegisterParticleEmitter(QQuick3DParticleEmitter* e);
    void registerParticleAffector(QQuick3DParticleAffector* a);
    void unRegisterParticleAffector(QQuick3DParticleAffector* a);

    void updateCurrentTime(int currentTime);

    QPRand *rand();
    bool isShared(const QQuick3DParticle *particle) const;
    int currentTime() const;

    struct TrailEmits {
        QQuick3DParticleTrailEmitter *emitter = nullptr;
        int amount = 0;
    };

    Q_INVOKABLE void reset();

public Q_SLOTS:
    void setRunning(bool running);
    void setPaused(bool paused);
    void setStartTime(int startTime);
    void setTime(int time);
    void setUseRandomSeed(bool randomize);
    void setSeed(int seed);
    void setLogging(bool logging);

    void setEditorTime(int time);

Q_SIGNALS:
    void runningChanged();
    void pausedChanged();
    void timeChanged();
    void startTimeChanged();
    void useRandomSeedChanged();
    void seedChanged();
    void loggingChanged();
    void loggingDataChanged();

protected:
    void componentComplete() override;

private:
    void updateLoggingData();
    void resetLoggingVariables();
    void doSeedRandomization();
    void refresh();
    void markDirty();
    void processModelParticle(QQuick3DParticleModelParticle *modelParticle, const QVector<TrailEmits> &trailEmits, float timeS);
    void processSpriteParticle(QQuick3DParticleSpriteParticle *spriteParticle, const QVector<TrailEmits> &trailEmits, float timeS);
    void processModelBlendParticle(QQuick3DParticleModelBlendParticle *particle, const QVector<TrailEmits> &trailEmits, float timeS);
    void processParticleCommon(QQuick3DParticleDataCurrent &currentData, const QQuick3DParticleData *d, float particleTimeS);
    void processParticleFadeInOut(QQuick3DParticleDataCurrent &currentData, const QQuick3DParticle *particle, float particleTimeS, float particleTimeLeftS);
    void processParticleAlignment(QQuick3DParticleDataCurrent &currentData, const QQuick3DParticle *particle, const QQuick3DParticleData *d);
    static bool isGloballyDisabled();
    static bool isEditorModeOn();

private:
    friend class QQuick3DParticleEmitter;
    friend class QQuick3DParticleTrailEmitter;
    friend class QQuick3DParticleSystemUpdate;
    friend class QQuick3DParticleSystemAnimation;
    friend class QQuick3DParticleModelBlendParticle;

    bool m_running;
    bool m_paused;
    bool m_initialized;
    bool m_componentComplete;
    // This animation runs the system, progressing time with pause, continue etc.
    QQuick3DParticleSystemAnimation *m_animation = nullptr;
    // This animation handles system dirty updates and runs always.
    // It makes sure that updates are done in sync with other animations and only once per frame.
    QQuick3DParticleSystemUpdate *m_updateAnimation = nullptr;

    QList<QQuick3DParticle *> m_particles;
    QList<QQuick3DParticleEmitter *> m_emitters;
    QList<QQuick3DParticleTrailEmitter *> m_trailEmitters;
    QList<QQuick3DParticleAffector *> m_affectors;
    QMap<QQuick3DParticleAffector *, QMetaObject::Connection> m_connections;

    int m_startTime = 0;
    // Current time in ms
    int m_time = 0;
    int m_currentTime = 0;

    // This overrides the time when editor mode is on
    int m_editorTime = 0;

    QElapsedTimer m_perfTimer;
    QTimer m_loggingTimer;
    qint64 m_timeAnimation = 0;
    int m_particlesMax = 0;
    int m_particlesUsed = 0;
    int m_updates = 0;
    bool m_useRandomSeed = true;
    int m_seed = 0;
    bool m_logging;
    QQuick3DParticleSystemLogging *m_loggingData = nullptr;
    QPRand m_rand;
    int m_particleIdIndex = 0;
};

class QQuick3DParticleSystemAnimation : public QAbstractAnimation
{
    Q_OBJECT
public:
    QQuick3DParticleSystemAnimation(QQuick3DParticleSystem *system)
        : QAbstractAnimation(static_cast<QObject *>(system)), m_system(system)
    { }
protected:
    void updateCurrentTime(int t) override
    {
        // Keep the time property up-to-date
        if (!m_system->isEditorModeOn() && !m_system->isGloballyDisabled())
            m_system->setTime(t);

        m_system->updateCurrentTime(t + m_system->startTime());
    }

    int duration() const override
    {
        return -1;
    }

private:
    QQuick3DParticleSystem *m_system;
};

class QQuick3DParticleSystemUpdate : public QAbstractAnimation
{
    Q_OBJECT
public:
    QQuick3DParticleSystemUpdate(QQuick3DParticleSystem *system)
        : QAbstractAnimation(static_cast<QObject *>(system)), m_system(system)
    { }

    void setDirty(bool dirty)
    {
        m_dirty = dirty;
    }

protected:
    void updateCurrentTime(int t) override
    {
        Q_UNUSED(t);
        if (m_dirty)
            m_system->refresh();
    }

    int duration() const override
    {
        return -1;
    }

private:
    QQuick3DParticleSystem *m_system;
    bool m_dirty = false;
};

QT_END_NAMESPACE

#endif // QQUICK3DPPARTICLESYSTEM_H


