// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QTest>
#include <QSignalSpy>
#include <QScopedPointer>

#include <QtQuick3DParticles/private/qquick3dparticlepointrotator_p.h>


class tst_QQuick3DParticlePointRotator : public QObject
{
    Q_OBJECT

    class PointRotator : public QQuick3DParticlePointRotator
    {
    public:
        PointRotator(QQuick3DNode *parent = nullptr)
            : QQuick3DParticlePointRotator(parent)
        {

        }
        void testAffectParticle(const QQuick3DParticleData &sd, QQuick3DParticleDataCurrent *d, float time)
        {
            QQuick3DParticlePointRotator::affectParticle(sd, d, time);
        }
    };

private slots:
    void testInitialization();
    void testAffectParticle();
};

void tst_QQuick3DParticlePointRotator::testInitialization()
{
    QQuick3DParticlePointRotator *rotator = new QQuick3DParticlePointRotator();

    const QVector3D direction(0.0f, 1.0f, 0.0f);
    QVERIFY(qFuzzyCompare(rotator->magnitude(), 10.0f));
    QVERIFY(qFuzzyCompare(rotator->direction(), direction));
    QVERIFY(qFuzzyCompare(rotator->pivotPoint(), QVector3D()));

    rotator->setMagnitude(20.0f);
    QCOMPARE(rotator->magnitude(), 20.0f);

    const QVector3D direction2(0.0f, 1.0f, 0.0f);
    rotator->setDirection(direction2);
    QVERIFY(qFuzzyCompare(rotator->direction(), direction2));

    rotator->setPivotPoint(direction2);
    QVERIFY(qFuzzyCompare(rotator->pivotPoint(), direction2));

    delete rotator;
}

void tst_QQuick3DParticlePointRotator::testAffectParticle()
{
    PointRotator *rotator = new PointRotator();

    QQuick3DParticleData particleData = {{}, {}, {}, {}, {}, {}, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0, false};
    QQuick3DParticleDataCurrent particleDataCurrent = {QVector3D(0, 0, 1.0f), {}, {}, {}, {}};

    rotator->setMagnitude(0.0f);
    rotator->testAffectParticle(particleData, &particleDataCurrent, 0.5f);
    QVERIFY(qFuzzyCompare(particleDataCurrent.position, QVector3D(0, 0, 1.0f)));

    rotator->setMagnitude(180.0f);
    rotator->testAffectParticle(particleData, &particleDataCurrent, 0.5f);
    QVERIFY(qFuzzyCompare(particleDataCurrent.position, QVector3D(1.0f, 0, 0)));

    delete rotator;
}

QTEST_APPLESS_MAIN(tst_QQuick3DParticlePointRotator)
#include "tst_qquick3dparticlepointrotator.moc"
