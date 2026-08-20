// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QTest>
#include <QSignalSpy>
#include <QScopedPointer>

#include <QtQuick3DParticles/private/qquick3dparticletargetdirection_p.h>


class tst_QQuick3DParticleTargetDirection : public QObject
{
    Q_OBJECT

private slots:
    void testTargetDirection();
};

void tst_QQuick3DParticleTargetDirection::testTargetDirection()
{
    QQuick3DParticleTargetDirection *targetDir = new QQuick3DParticleTargetDirection();

    delete targetDir;

    targetDir = new QQuick3DParticleTargetDirection();

    QVERIFY(qFuzzyCompare(targetDir->position(), QVector3D()));
    QVERIFY(qFuzzyCompare(targetDir->positionVariation(), QVector3D()));
    QCOMPARE(targetDir->normalized(), false);
    QVERIFY(qFuzzyCompare(targetDir->magnitude(), 1.0f));
    QVERIFY(qFuzzyCompare(targetDir->magnitudeVariation(), 0.0f));

    const QVector3D vecVal(1.0f, 2.0f, 3.0f);
    targetDir->setPosition(vecVal);
    QVERIFY(qFuzzyCompare(targetDir->position(), vecVal));
    targetDir->setPositionVariation(vecVal);
    QVERIFY(qFuzzyCompare(targetDir->positionVariation(), vecVal));

    targetDir->setNormalized(true);
    QCOMPARE(targetDir->normalized(), true);

    targetDir->setMagnitude(10.0f);
    QVERIFY(qFuzzyCompare(targetDir->magnitude(), 10.0f));

    targetDir->setMagnitudeVariation(5.0f);
    QVERIFY(qFuzzyCompare(targetDir->magnitudeVariation(), 5.0f));

    delete targetDir;
}

QTEST_APPLESS_MAIN(tst_QQuick3DParticleTargetDirection)
#include "tst_qquick3dparticletargetdirection.moc"
