// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QTest>
#include <QSignalSpy>
#include <QScopedPointer>

#include <QtQuick3DParticles/private/qquick3dparticleshape_p.h>


class tst_QQuick3DParticleShape : public QObject
{
    Q_OBJECT

private slots:
    void testShape();
};

void tst_QQuick3DParticleShape::testShape()
{
    QQuick3DParticleShape *shape = new QQuick3DParticleShape();

    QCOMPARE(shape->fill(), true);
    QCOMPARE(shape->type(), QQuick3DParticleShape::Cube);

    shape->setFill(false);
    QCOMPARE(shape->fill(), false);

    shape->setType(QQuick3DParticleShape::Sphere);
    QCOMPARE(shape->type(), QQuick3DParticleShape::Sphere);

    delete shape;
}

QTEST_APPLESS_MAIN(tst_QQuick3DParticleShape)
#include "tst_qquick3dparticleshape.moc"
