// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QTest>
#include <QSignalSpy>
#include <QScopedPointer>

#include <QtQuick3DParticles/private/qquick3dparticlelineparticle_p.h>

class tst_QQuick3DParticleLineParticle : public QObject
{
    Q_OBJECT

private slots:
    void testInitialization();
    void testParticle();
};

void tst_QQuick3DParticleLineParticle::testInitialization()
{
    QQuick3DParticleLineParticle *particle = new QQuick3DParticleLineParticle();

    QCOMPARE(particle->segmentCount(), 1);
    QVERIFY(qFuzzyCompare(particle->alphaFade(), 0.0f));
    QVERIFY(qFuzzyCompare(particle->scaleMultiplier(), 1.0f));
    QVERIFY(qFuzzyCompare(particle->texcoordMultiplier(), 1.0f));
    QVERIFY(qFuzzyCompare(particle->length(), -1.0f));
    QVERIFY(qFuzzyCompare(particle->lengthVariation(), 0.0f));
    QVERIFY(qFuzzyCompare(particle->lengthDeltaMin(), 10.0f));
    QCOMPARE(particle->eolFadeOutDuration(), 0);
    QCOMPARE(particle->texcoordMode(), QQuick3DParticleLineParticle::Absolute);

    delete particle;
}

void tst_QQuick3DParticleLineParticle::testParticle()
{
    QQuick3DParticleLineParticle *particle = new QQuick3DParticleLineParticle();

    particle->setSegmentCount(100);
    QCOMPARE(particle->segmentCount(), 100);

    particle->setSegmentCount(-100);
    QCOMPARE(particle->segmentCount(), 1);

    particle->setAlphaFade(0.5f);
    QVERIFY(qFuzzyCompare(particle->alphaFade(), 0.5f));

    particle->setAlphaFade(1.5f);
    QVERIFY(qFuzzyCompare(particle->alphaFade(), 1.0f));

    particle->setAlphaFade(-1.5f);
    QVERIFY(qFuzzyCompare(particle->alphaFade(), 0.0f));

    particle->setScaleMultiplier(0.5f);
    QVERIFY(qFuzzyCompare(particle->scaleMultiplier(), 0.5f));

    particle->setScaleMultiplier(3.5f);
    QVERIFY(qFuzzyCompare(particle->scaleMultiplier(), 2.0f));

    particle->setScaleMultiplier(-1.5f);
    QVERIFY(qFuzzyCompare(particle->scaleMultiplier(), 0.0f));

    particle->setTexcoordMultiplier(0.5f);
    QVERIFY(qFuzzyCompare(particle->texcoordMultiplier(), 0.5f));

    particle->setTexcoordMultiplier(3.5f);
    QVERIFY(qFuzzyCompare(particle->texcoordMultiplier(), 3.5f));

    particle->setTexcoordMultiplier(-1.5f);
    QVERIFY(qFuzzyCompare(particle->texcoordMultiplier(), -1.5f));

    particle->setLength(100.0f);
    QVERIFY(qFuzzyCompare(particle->length(), 100.0f));

    particle->setLength(-1.0f);
    QVERIFY(qFuzzyCompare(particle->length(), -1.0f));

    particle->setLength(-1.5f);
    QVERIFY(qFuzzyCompare(particle->length(), 0.0f));

    particle->setLengthVariation(100.0f);
    QVERIFY(qFuzzyCompare(particle->lengthVariation(), 100.0f));

    particle->setLengthVariation(-1.0f);
    QVERIFY(qFuzzyCompare(particle->lengthVariation(), 0.0f));

    particle->setLengthDeltaMin(100.0f);
    QVERIFY(qFuzzyCompare(particle->lengthDeltaMin(), 100.0f));

    particle->setLengthDeltaMin(-1.0f);
    QVERIFY(qFuzzyCompare(particle->lengthDeltaMin(), 0.0f));

    particle->setEolFadeOutDuration(100);
    QCOMPARE(particle->eolFadeOutDuration(), 100);

    particle->setEolFadeOutDuration(-100);
    QCOMPARE(particle->eolFadeOutDuration(), 0);

    particle->setTexcoordMode(QQuick3DParticleLineParticle::Fill);
    QCOMPARE(particle->texcoordMode(), QQuick3DParticleLineParticle::Fill);

    delete particle;
}

QTEST_APPLESS_MAIN(tst_QQuick3DParticleLineParticle)
#include "tst_qquick3dparticlelineparticle.moc"
