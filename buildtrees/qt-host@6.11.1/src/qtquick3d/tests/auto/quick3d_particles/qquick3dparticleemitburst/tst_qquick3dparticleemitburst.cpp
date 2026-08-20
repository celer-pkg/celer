// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QTest>
#include <QSignalSpy>
#include <QScopedPointer>

#include <QtQuick3DParticles/private/qquick3dparticleemitburst_p.h>
#include <QtQuick3DParticles/private/qquick3dparticledynamicburst_p.h>


class tst_QQuick3DParticleEmitBurst : public QObject
{
    Q_OBJECT

private slots:
    void testEmitBurst();
    void testDynamicBurst();
};

void tst_QQuick3DParticleEmitBurst::testEmitBurst()
{
    QQuick3DParticleEmitBurst *burst = new QQuick3DParticleEmitBurst();

    QCOMPARE(burst->time(), 0);
    QCOMPARE(burst->amount(), 0);
    QCOMPARE(burst->duration(), 0);

    burst->setTime(1000);
    QCOMPARE(burst->time(), 1000);

    burst->setAmount(100);
    QCOMPARE(burst->amount(), 100);

    burst->setAmount(-100);
    QCOMPARE(burst->amount(), 100);

    burst->setDuration(100);
    QCOMPARE(burst->duration(), 100);

    burst->setDuration(-100);
    QCOMPARE(burst->duration(), 100);

    delete burst;
}

void tst_QQuick3DParticleEmitBurst::testDynamicBurst()
{
    QQuick3DParticleDynamicBurst *burst = new QQuick3DParticleDynamicBurst();

    // Derived properties
    QCOMPARE(burst->time(), 0);
    QCOMPARE(burst->amount(), 0);
    QCOMPARE(burst->duration(), 0);

    QCOMPARE(burst->enabled(), true);
    QCOMPARE(burst->amountVariation(), 0);
    QCOMPARE(burst->triggerMode(), QQuick3DParticleDynamicBurst::TriggerTime);

    burst->setEnabled(false);
    QCOMPARE(burst->enabled(), false);

    burst->setAmountVariation(20);
    QCOMPARE(burst->amountVariation(), 20);

    burst->setTriggerMode(QQuick3DParticleDynamicBurst::TriggerEnd);
    QCOMPARE(burst->triggerMode(), QQuick3DParticleDynamicBurst::TriggerEnd);

    delete burst;
}

QTEST_APPLESS_MAIN(tst_QQuick3DParticleEmitBurst)
#include "tst_qquick3dparticleemitburst.moc"
