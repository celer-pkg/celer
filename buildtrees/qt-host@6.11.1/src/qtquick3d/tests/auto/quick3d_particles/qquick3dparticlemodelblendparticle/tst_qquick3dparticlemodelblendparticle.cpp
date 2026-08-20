// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QTest>
#include <QSignalSpy>
#include <QScopedPointer>

#include <QtQuick3DParticles/private/qquick3dparticle_p.h>
#include <QtQuick3DParticles/private/qquick3dparticlemodelblendparticle_p.h>


class tst_QQuick3DParticleModelBlendParticle : public QObject
{
    Q_OBJECT

private slots:
    void testParticle();
    void testParticleProperties();
};

void tst_QQuick3DParticleModelBlendParticle::testParticle()
{
    QQuick3DParticleModelBlendParticle *particle = new QQuick3DParticleModelBlendParticle();

    QCOMPARE(particle->delegate(), nullptr);
    QCOMPARE(particle->endNode(), nullptr);
    QCOMPARE(particle->modelBlendMode(), QQuick3DParticleModelBlendParticle::Explode);
    QCOMPARE(particle->endTime(), 0);
    QCOMPARE(particle->activationNode(), nullptr);
    QCOMPARE(particle->emitMode(), QQuick3DParticleModelBlendParticle::Sequential);
    QCOMPARE(particle->fadeInEffect(), QQuick3DParticle::FadeNone);
    QCOMPARE(particle->fadeOutEffect(), QQuick3DParticle::FadeNone);

    delete particle;
}

void tst_QQuick3DParticleModelBlendParticle::testParticleProperties()
{
    QQuick3DParticleModelBlendParticle *particle = new QQuick3DParticleModelBlendParticle();
    QQuick3DNode *node = new QQuick3DNode();

    particle->setEndNode(node);
    QCOMPARE(particle->endNode(), node);

    particle->setModelBlendMode(QQuick3DParticleModelBlendParticle::Construct);
    QCOMPARE(particle->modelBlendMode(), QQuick3DParticleModelBlendParticle::Construct);

    particle->setEndTime(1000);
    QCOMPARE(particle->endTime(), 1000);

    particle->setActivationNode(node);
    QCOMPARE(particle->activationNode(), node);

    particle->setEmitMode(QQuick3DParticleModelBlendParticle::Random);
    QCOMPARE(particle->emitMode(), QQuick3DParticleModelBlendParticle::Random);

    particle->setFadeInEffect(QQuick3DParticle::FadeOpacity);
    QCOMPARE(particle->fadeInEffect(), QQuick3DParticle::FadeOpacity);

    particle->setFadeOutEffect(QQuick3DParticle::FadeOpacity);
    QCOMPARE(particle->fadeOutEffect(), QQuick3DParticle::FadeOpacity);

    delete node;
    delete particle;
}

QTEST_APPLESS_MAIN(tst_QQuick3DParticleModelBlendParticle)
#include "tst_qquick3dparticlemodelblendparticle.moc"
