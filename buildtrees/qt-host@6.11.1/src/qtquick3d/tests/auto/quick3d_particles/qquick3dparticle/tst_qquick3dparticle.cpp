// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QTest>
#include <QSignalSpy>
#include <QScopedPointer>

#include <QtQuick3DParticles/private/qquick3dparticlesystem_p.h>
#include <QtQuick3DParticles/private/qquick3dparticle_p.h>


class tst_QQuick3DParticle : public QObject
{
    Q_OBJECT

    class Particle : public QQuick3DParticle
    {
    public:
        Particle(QQuick3DObject *parent) : QQuick3DParticle(parent) {}
        void reset() override
        {

        }
    };

private slots:
    void testParticle();
};

void tst_QQuick3DParticle::testParticle()
{
    QScopedPointer<QQuick3DParticleSystem> system(new QQuick3DParticleSystem());
    Particle *p = new Particle(system.data());
    p->setSystem(system.data());
    QCOMPARE(p->system(), system.data());

    // Test deleting
    delete p;

    p = new Particle(system.data());

    QSignalSpy spy(p, SIGNAL(systemChanged()));

    auto *newSystem = new QQuick3DParticleSystem();
    p->setSystem(newSystem);

    QCOMPARE(spy.size(), 1);

    QCOMPARE(p->maxAmount(), 100);
    p->setMaxAmount(200);
    QCOMPARE(p->maxAmount(), 200);

    QCOMPARE(p->color(), Qt::white);
    p->setColor(Qt::red);
    QCOMPARE(p->color(), Qt::red);

    QCOMPARE(p->colorVariation(), QVector4D());
    p->setColorVariation(QVector4D(0.5f, 0.5f, 0.5f, 0.5f));
    QVERIFY(qFuzzyCompare(p->colorVariation(), QVector4D(0.5f, 0.5f, 0.5f, 0.5f)));

    QCOMPARE(p->fadeInEffect(), QQuick3DParticle::FadeOpacity);
    QCOMPARE(p->fadeOutEffect(), QQuick3DParticle::FadeOpacity);
    p->setFadeInEffect(QQuick3DParticle::FadeNone);
    QCOMPARE(p->fadeInEffect(), QQuick3DParticle::FadeNone);
    p->setFadeOutEffect(QQuick3DParticle::FadeNone);
    QCOMPARE(p->fadeOutEffect(), QQuick3DParticle::FadeNone);

    QCOMPARE(p->fadeInDuration(), 250);
    QCOMPARE(p->fadeOutDuration(), 250);
    p->setFadeInDuration(1000);
    QCOMPARE(p->fadeInDuration(), 1000);
    p->setFadeOutDuration(1000);
    QCOMPARE(p->fadeOutDuration(), 1000);

    QCOMPARE(p->alignMode(), QQuick3DParticle::AlignNone);
    p->setAlignMode(QQuick3DParticle::AlignTowardsTarget);
    QCOMPARE(p->alignMode(), QQuick3DParticle::AlignTowardsTarget);

    QVERIFY(qFuzzyCompare(p->alignTargetPosition(), QVector3D()));
    p->setAlignTargetPosition(QVector3D(100.0f, 100.0f, 100.0f));
    QVERIFY(qFuzzyCompare(p->alignTargetPosition(), QVector3D(100.0f, 100.0f, 100.0f)));

    QCOMPARE(p->sortMode(), QQuick3DParticle::SortNone);
    p->setSortMode(QQuick3DParticle::SortDistance);
    QCOMPARE(p->sortMode(), QQuick3DParticle::SortDistance);

}

QTEST_APPLESS_MAIN(tst_QQuick3DParticle)
#include "tst_qquick3dparticle.moc"
