// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QTest>
#include <QSignalSpy>
#include <QScopedPointer>

#include <QtQuick3DParticles/private/qquick3dparticlespriteparticle_p.h>
#include <QtQuick3DParticles/private/qquick3dparticlemodelparticle_p.h>
#include <QtQuick3DParticles/private/qquick3dparticlesystem_p.h>


class tst_QQuick3DParticleSystem : public QObject
{
    Q_OBJECT

private slots:
    void testInitialization();
    void testSystem();
};

void tst_QQuick3DParticleSystem::testInitialization()
{
    QQuick3DParticleSystem *system = new QQuick3DParticleSystem();

    QCOMPARE(system->isRunning(), true);
    QCOMPARE(system->isPaused(), false);
    QCOMPARE(system->particleCount(), 0);
    QCOMPARE(system->startTime(), 0);
    QCOMPARE(system->time(), 0);
    QCOMPARE(system->logging(), false);
    QCOMPARE(system->useRandomSeed(), true);
    QCOMPARE(system->seed(), 0);

    delete system;
}

void tst_QQuick3DParticleSystem::testSystem()
{
    QQuick3DParticleSystem *system = new QQuick3DParticleSystem();

    QQuick3DParticleSpriteParticle *sp = new QQuick3DParticleSpriteParticle(system);
    sp->setMaxAmount(10);

    system->registerParticle(sp);
    QCOMPARE(system->particleCount(), 10);

    QQuick3DParticleModelParticle *mp = new QQuick3DParticleModelParticle(system);
    mp->setMaxAmount(10);

    system->registerParticle(mp);
    QCOMPARE(system->particleCount(), 20);

    system->setStartTime(2000);
    QCOMPARE(system->startTime(), 2000);

    system->setTime(2000);
    QCOMPARE(system->time(), 2000);

    system->setUseRandomSeed(false);
    QCOMPARE(system->useRandomSeed(), false);

    system->setSeed(1234);
    QCOMPARE(system->seed(), 1234);

    delete system;
}


QTEST_APPLESS_MAIN(tst_QQuick3DParticleSystem)
#include "tst_qquick3dparticlesystem.moc"
