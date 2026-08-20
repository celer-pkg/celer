// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QTest>
#include <QSignalSpy>
#include <QScopedPointer>

#include <QtQuick3DParticles/private/qquick3dparticle_p.h>
#include <QtQuick3DParticles/private/qquick3dparticlespriteparticle_p.h>
#include <QtQuick3DParticles/private/qquick3dparticlespritesequence_p.h>


class tst_QQuick3DParticleSpriteParticle : public QObject
{
    Q_OBJECT

private slots:
    void testInitialization();
    void testParticle();
};

void tst_QQuick3DParticleSpriteParticle::testInitialization()
{
    QQuick3DParticleSpriteParticle *particle = new QQuick3DParticleSpriteParticle();
    QQuick3DParticleSpriteSequence *sequence = new QQuick3DParticleSpriteSequence();

    QCOMPARE(particle->blendMode(), QQuick3DParticleSpriteParticle::SourceOver);
    QCOMPARE(particle->sprite(), nullptr);
    QCOMPARE(sequence->frameCount(), 1);
    QCOMPARE(sequence->interpolate(), true);
    QCOMPARE(particle->billboard(), false);
    QCOMPARE(particle->colorTable(), nullptr);
    QVERIFY(qFuzzyCompare(particle->particleScale(), 5.0f));

    delete sequence;
    delete particle;
}

void tst_QQuick3DParticleSpriteParticle::testParticle()
{
    QQuick3DParticleSpriteParticle *particle = new QQuick3DParticleSpriteParticle();
    QQuick3DParticleSpriteSequence *sequence = new QQuick3DParticleSpriteSequence();

    particle->setBlendMode(QQuick3DParticleSpriteParticle::Multiply);
    QCOMPARE(particle->blendMode(), QQuick3DParticleSpriteParticle::Multiply);

    sequence->setFrameCount(1000);
    QCOMPARE(sequence->frameCount(), 1000);

    sequence->setInterpolate(false);
    QCOMPARE(sequence->interpolate(), false);

    particle->setBillboard(true);
    QCOMPARE(particle->billboard(), true);

    particle->setParticleScale(20.0f);
    QVERIFY(qFuzzyCompare(particle->particleScale(), 20.0f));

    QQuick3DTexture *texture = new QQuick3DTexture();
    particle->setSprite(texture);
    QCOMPARE(particle->sprite(), texture);

    particle->setColorTable(texture);
    QCOMPARE(particle->colorTable(), texture);

    delete texture;
    delete sequence;
    delete particle;
}

QTEST_APPLESS_MAIN(tst_QQuick3DParticleSpriteParticle)
#include "tst_qquick3dparticlespriteparticle.moc"
