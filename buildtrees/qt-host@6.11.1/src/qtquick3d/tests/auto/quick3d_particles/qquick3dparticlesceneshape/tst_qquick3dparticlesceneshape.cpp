// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QTest>
#include <QSignalSpy>
#include <QScopedPointer>

#include <QtQuick3D/private/qquick3dmodel_p.h>
#include <QtQuick3DParticles/private/qquick3dparticlesystem_p.h>
#include <QtQuick3DParticles/private/qquick3dparticleemitter_p.h>
#include <QtQuick3DParticles/private/qquick3dparticlesceneshape_p.h>


class tst_QQuick3DParticleSceneShape : public QObject
{
    Q_OBJECT

private slots:
    void testInitialization();
    void testModelWithGeometry();
};

void tst_QQuick3DParticleSceneShape::testInitialization()
{
    QScopedPointer<QQuick3DParticleSceneShape> sceneShape(new QQuick3DParticleSceneShape());
    QVERIFY(qFuzzyCompare(sceneShape->sceneCenter(), QVector3D(0,0,0)));
    QVERIFY(qFuzzyCompare(sceneShape->sceneExtents(), QVector3D(0,0,0)));
    QVERIFY(qFuzzyCompare(sceneShape->shapeResolution(), 10.0f));
    QCOMPARE(sceneShape->scene(), nullptr);
    QCOMPARE(sceneShape->geometry(), nullptr);
}

void tst_QQuick3DParticleSceneShape::testModelWithGeometry()
{
    QScopedPointer<QQuick3DParticleSystem> system(new QQuick3DParticleSystem());
    QScopedPointer<QQuick3DParticleEmitter> emitter(new QQuick3DParticleEmitter());
    QScopedPointer<QQuick3DNode> root(new QQuick3DNode());
    QQuick3DModel *model = new QQuick3DModel(root.get());

    QScopedPointer<QQuick3DGeometry> geometry(new QQuick3DGeometry());

    emitter->setSystem(system.get());
    system->setSeed(31);

    geometry->setBounds(QVector3D(-100, 0, -100), QVector3D(-50, 0, 100));

    // Purposfully exclude origin
    QVector3D vertexData[] = {{-100, 0, 100}, {-100, 0, -100}, {-50, 0, -100}, {-50, 0, -100}};
    quint32 indexData[] = {0, 2, 1, 0, 2, 3};
    QByteArray indices(reinterpret_cast<const char *>(indexData), sizeof(quint32) * 6u);
    QByteArray vertices(reinterpret_cast<const char *>(vertexData), sizeof(QVector3D) * 4u);

    geometry->setIndexData(indices);
    geometry->setVertexData(vertices);
    geometry->addAttribute(QQuick3DGeometry::Attribute::PositionSemantic, 0, QQuick3DGeometry::Attribute::F32Type);
    geometry->addAttribute(QQuick3DGeometry::Attribute::IndexSemantic, 0, QQuick3DGeometry::Attribute::U32Type);
    geometry->setPrimitiveType(QQuick3DGeometry::PrimitiveType::Triangles);
    geometry->setStride(sizeof(QVector3D));
    model->setGeometry(geometry.get());
    QQuick3DParticleSceneShape *sceneShape = new QQuick3DParticleSceneShape(emitter.get());
    emitter->setShape(sceneShape);

    sceneShape->setShapeResolution(40.0f);
    sceneShape->setSceneExtents(QVector3D(100.0f, 100.0f, 100.0f));
    sceneShape->setScene(model);

    const QVector3D position = sceneShape->getPosition(0);
    const QVector3D normal = sceneShape->getSurfaceNormal(0);

    QVERIFY(qFuzzyCompare(normal, QVector3D(0, 1.0f, 0)));
    QVERIFY(qFuzzyCompare(position.y(), 0.0f));
    QVERIFY(position.x() >= -100.0f && position.x() <= -50.0f);
    QVERIFY(position.z() >= -100.0f && position.z() <= 100.0f);
}

QTEST_APPLESS_MAIN(tst_QQuick3DParticleSceneShape)
#include "tst_qquick3dparticlesceneshape.moc"
