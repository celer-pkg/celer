// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QTest>
#include <QSignalSpy>
#include <QScopedPointer>

#include <QtQuick3DParticles/private/qquick3dparticle_p.h>
#include <QtQuick3DParticles/private/qquick3dparticlemodelparticle_p.h>


class tst_QQuick3DParticleModelParticle : public QObject
{
    Q_OBJECT

private slots:
    void testParticle();
};

void tst_QQuick3DParticleModelParticle::testParticle()
{
    QQuick3DParticleModelParticle *particle = new QQuick3DParticleModelParticle();

    QCOMPARE(particle->delegate(), nullptr);
    QCOMPARE(particle->instanceTable(), nullptr);

    delete particle;
}

QTEST_APPLESS_MAIN(tst_QQuick3DParticleModelParticle)
#include "tst_qquick3dparticlemodelparticle.moc"
