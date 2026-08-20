// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QTest>
#include <QSignalSpy>
#include <QScopedPointer>

#include <QtQuick3DParticles/private/qquick3dparticletrailemitter_p.h>


class tst_QQuick3DParticleTrailEmitter : public QObject
{
    Q_OBJECT

private slots:
    void testInitialization();
};

void tst_QQuick3DParticleTrailEmitter::testInitialization()
{
    QQuick3DParticleTrailEmitter *emitter = new QQuick3DParticleTrailEmitter();

    QCOMPARE(emitter->follow(), nullptr);

    QQuick3DParticleModelParticle *particle = new QQuick3DParticleModelParticle();
    emitter->setFollow(particle);
    QCOMPARE(emitter->follow(), particle);

    delete particle;
    delete emitter;
}

QTEST_APPLESS_MAIN(tst_QQuick3DParticleTrailEmitter)
#include "tst_qquick3dparticletrailemitter.moc"
