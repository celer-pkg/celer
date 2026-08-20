// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QTest>
#include <QSignalSpy>
#include <QScopedPointer>

#include <QtQuick3DParticles/private/qquick3dparticle_p.h>
#include <QtQuick3DParticles/private/qquick3dparticlegravity_p.h>


class tst_QQuick3DParticleGravity : public QObject
{
    Q_OBJECT

    class Gravity : public QQuick3DParticleGravity
    {
        friend class tst_QQuick3DParticleGravity;
    public:
        Gravity() : QQuick3DParticleGravity() {}

        void testAffectParticle(const QQuick3DParticleData &sd, QQuick3DParticleDataCurrent *d, float time)
        {
            QQuick3DParticleGravity::affectParticle(sd, d, time);
        }
    };

private slots:
    void testGravity();
    void testGravityAffect();
};

void tst_QQuick3DParticleGravity::testGravity()
{
    QQuick3DParticleGravity *gravity = new QQuick3DParticleGravity();

    delete gravity;

    gravity = new QQuick3DParticleGravity();

    QVERIFY(qFuzzyCompare(gravity->magnitude(), 100.0f));
    QVERIFY(qFuzzyCompare(gravity->direction(), QVector3D(0.0f, -1.0f, 0.0f)));

    gravity->setMagnitude(50.0f);
    QVERIFY(qFuzzyCompare(gravity->magnitude(), 50.0f));

    const QVector3D dirVal(1.0f, 2.0f, 3.0f);
    gravity->setDirection(dirVal);
    QVERIFY(qFuzzyCompare(gravity->direction(), dirVal));

    delete gravity;
}

void tst_QQuick3DParticleGravity::testGravityAffect()
{
    Gravity *gravity = new Gravity();

    QQuick3DParticleData particleData = {{}, {}, {}, {}, {}, {}, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0, false};
    QQuick3DParticleDataCurrent particleDataCurrent = {{}, {}, {}, {}, {}};

    gravity->setMagnitude(0.0f);
    gravity->testAffectParticle(particleData, &particleDataCurrent, 0.5f);
    QVERIFY(qFuzzyCompare(particleDataCurrent.position, QVector3D()));

    gravity->setMagnitude(100.0f);
    gravity->testAffectParticle(particleData, &particleDataCurrent, 0.5f);
    QVERIFY(!qFuzzyCompare(particleDataCurrent.position, QVector3D()));

    delete gravity;
}

QTEST_APPLESS_MAIN(tst_QQuick3DParticleGravity)
#include "tst_qquick3dparticlegravity.moc"
