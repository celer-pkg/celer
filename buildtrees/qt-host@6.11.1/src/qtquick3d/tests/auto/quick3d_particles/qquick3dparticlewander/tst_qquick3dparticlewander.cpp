// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QTest>
#include <QSignalSpy>
#include <QScopedPointer>

#include <QtQuick3DParticles/private/qquick3dparticlewander_p.h>


class tst_QQuick3DParticleWander : public QObject
{
    Q_OBJECT

    class Wander : public QQuick3DParticleWander
    {
    public:
        Wander(QQuick3DNode *parent = nullptr)
            : QQuick3DParticleWander(parent)
        {

        }
        void testAffectParticle(const QQuick3DParticleData &sd, QQuick3DParticleDataCurrent *d, float time)
        {
            QQuick3DParticleWander::affectParticle(sd, d, time);
        }
    };

    class TestSystem : public QQuick3DParticleSystem
    {
    public:
        TestSystem(QQuick3DNode *parent = nullptr)
            : QQuick3DParticleSystem(parent)
        {

        }
        void init()
        {
            QQuick3DParticleSystem::componentComplete();
        }
    };

private slots:
    void testInitialization();
    void testAffectParticle();
};

void tst_QQuick3DParticleWander::testInitialization()
{
    QQuick3DParticleWander *wander = new QQuick3DParticleWander();

    QVERIFY(qFuzzyCompare(wander->globalAmount(), QVector3D()));
    QVERIFY(qFuzzyCompare(wander->globalPace(), QVector3D()));
    QVERIFY(qFuzzyCompare(wander->uniqueAmount(), QVector3D()));
    QVERIFY(qFuzzyCompare(wander->uniquePace(), QVector3D()));
    QVERIFY(qFuzzyCompare(wander->uniqueAmountVariation(), 0.0f));
    QVERIFY(qFuzzyCompare(wander->uniquePaceVariation(), 0.0f));

    const QVector3D amount(1.0f, 1.0f, 1.0f);
    const QVector3D pace(1.0f, 1.0f, 1.0f);
    wander->setGlobalAmount(amount);
    QVERIFY(qFuzzyCompare(wander->globalAmount(), amount));

    wander->setUniqueAmount(amount);
    QVERIFY(qFuzzyCompare(wander->uniqueAmount(), amount));

    wander->setUniqueAmountVariation(1.0f);
    QVERIFY(qFuzzyCompare(wander->uniqueAmountVariation(), 1.0f));

    wander->setGlobalPace(pace);
    QVERIFY(qFuzzyCompare(wander->globalPace(), pace));

    wander->setUniquePace(pace);
    QVERIFY(qFuzzyCompare(wander->uniquePace(), pace));

    wander->setUniquePaceVariation(1.0f);
    QVERIFY(qFuzzyCompare(wander->uniquePaceVariation(), 1.0f));

    delete wander;
}

void tst_QQuick3DParticleWander::testAffectParticle()
{
    TestSystem *system = new TestSystem();
    Wander *wander = new Wander();

    system->init();
    wander->setSystem(system);

    QQuick3DParticleData particleData = {{}, {}, {}, {}, {}, {}, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0, false};
    QQuick3DParticleDataCurrent particleDataCurrent = {{}, {}, {}, {}, {}};

    wander->testAffectParticle(particleData, &particleDataCurrent, 0.5f);
    QCOMPARE(particleDataCurrent.position, QVector3D());

    const QVector3D amount(1.0f, 1.0f, 1.0f);
    const QVector3D pace(1.0f, 1.0f, 1.0f);

    wander->setGlobalAmount(amount);
    wander->setUniqueAmount(amount);
    wander->setUniqueAmountVariation(1.0f);
    wander->setGlobalPace(pace);
    wander->setUniquePace(pace);
    wander->setUniquePaceVariation(1.0f);

    wander->testAffectParticle(particleData, &particleDataCurrent, 0.5f);
    QVERIFY(!qFuzzyCompare(particleDataCurrent.position, QVector3D()));

    delete wander;
    delete system;
}

QTEST_APPLESS_MAIN(tst_QQuick3DParticleWander)
#include "tst_qquick3dparticlewander.moc"
