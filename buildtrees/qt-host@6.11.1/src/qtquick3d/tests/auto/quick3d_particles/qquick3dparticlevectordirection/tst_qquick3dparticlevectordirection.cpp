// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QTest>
#include <QSignalSpy>
#include <QScopedPointer>

#include <QtQuick3DParticles/private/qquick3dparticlevectordirection_p.h>


class tst_QQuick3DParticleVectorDirection : public QObject
{
    Q_OBJECT

private slots:
    void testVectorDirection();
};

void tst_QQuick3DParticleVectorDirection::testVectorDirection()
{
    QQuick3DParticleVectorDirection *vectorDir = new QQuick3DParticleVectorDirection();

    delete vectorDir;

    vectorDir = new QQuick3DParticleVectorDirection();

    QVERIFY(qFuzzyCompare(vectorDir->direction(), QVector3D(0.0f, 100.0f, 0.0f)));
    QVERIFY(qFuzzyCompare(vectorDir->directionVariation(), QVector3D()));
    QVERIFY(vectorDir->normalized() == false);

    const QVector3D vecVal(100.0f, 200.0f, 300.0f);
    vectorDir->setDirection(vecVal);
    QVERIFY(qFuzzyCompare(vectorDir->direction(), vecVal));
    vectorDir->setDirectionVariation(vecVal);
    QVERIFY(qFuzzyCompare(vectorDir->directionVariation(), vecVal));
    vectorDir->setNormalized(true);
    QVERIFY(vectorDir->normalized() == true);

    delete vectorDir;
}

QTEST_APPLESS_MAIN(tst_QQuick3DParticleVectorDirection)
#include "tst_qquick3dparticlevectordirection.moc"
