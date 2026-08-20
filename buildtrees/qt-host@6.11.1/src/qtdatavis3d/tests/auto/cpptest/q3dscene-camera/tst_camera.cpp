// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtTest/QtTest>

#include <QtDataVisualization/Q3DCamera>

class tst_camera: public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    void construct();

    void initialProperties();
    void initializeProperties();
    void invalidProperties();

    void changePresets();

private:
    Q3DCamera *m_camera;
};

void tst_camera::initTestCase()
{
}

void tst_camera::cleanupTestCase()
{
}

void tst_camera::init()
{
    m_camera = new Q3DCamera();
}

void tst_camera::cleanup()
{
    delete m_camera;
}

void tst_camera::construct()
{
    Q3DCamera *camera = new Q3DCamera();
    QVERIFY(camera);
    delete camera;
}

void tst_camera::initialProperties()
{
    QVERIFY(m_camera);

    QCOMPARE(m_camera->cameraPreset(), Q3DCamera::CameraPresetNone);
    QCOMPARE(m_camera->maxZoomLevel(), 500.0f);
    QCOMPARE(m_camera->minZoomLevel(), 10.0f);
    QCOMPARE(m_camera->target(), QVector3D(0.0f, 0.0f, 0.0f));
    QCOMPARE(m_camera->wrapXRotation(), true);
    QCOMPARE(m_camera->wrapYRotation(), false);
    QCOMPARE(m_camera->xRotation(), 0.0f);
    QCOMPARE(m_camera->yRotation(), 0.0f);
    QCOMPARE(m_camera->zoomLevel(), 100.0f);

    // Common (from Q3DObject)
    QVERIFY(!m_camera->parentScene());
    QCOMPARE(m_camera->position(), QVector3D(0, 0, 0));
}

void tst_camera::initializeProperties()
{
    QVERIFY(m_camera);

    m_camera->setMaxZoomLevel(1000.0f);
    m_camera->setMinZoomLevel(100.0f);
    m_camera->setTarget(QVector3D(1.0f, -1.0f, 1.0f));
    m_camera->setWrapXRotation(false);
    m_camera->setWrapYRotation(true);
    m_camera->setXRotation(30.0f);
    m_camera->setYRotation(30.0f);
    m_camera->setZoomLevel(500.0f);

    QCOMPARE(m_camera->maxZoomLevel(), 1000.0f);
    QCOMPARE(m_camera->minZoomLevel(), 100.0f);
    QCOMPARE(m_camera->target(), QVector3D(1.0f, -1.0f, 1.0f));
    QCOMPARE(m_camera->wrapXRotation(), false);
    QCOMPARE(m_camera->wrapYRotation(), true);
    QCOMPARE(m_camera->xRotation(), 30.0f);
    QCOMPARE(m_camera->yRotation(), 30.0f);
    QCOMPARE(m_camera->zoomLevel(), 500.0f);

    m_camera->setPosition(QVector3D(1.0f, 1.0f, 1.0f));

    // Common (from Q3DObject)
    QCOMPARE(m_camera->position(), QVector3D(1.0f, 1.0f, 1.0f));
}

void tst_camera::invalidProperties()
{
    m_camera->setTarget(QVector3D(-1.5f, -1.5f, -1.5f));
    QCOMPARE(m_camera->target(), QVector3D(-1.0f, -1.0f, -1.0f));

    m_camera->setTarget(QVector3D(1.5f, 1.5f, 1.5f));
    QCOMPARE(m_camera->target(), QVector3D(1.0f, 1.0f, 1.0f));

    m_camera->setMinZoomLevel(0.1f);
    QCOMPARE(m_camera->minZoomLevel(), 1.0f);
}

void tst_camera::changePresets()
{
    m_camera->setCameraPreset(Q3DCamera::CameraPresetBehind); // Will be overridden by the the following sets
    m_camera->setMaxZoomLevel(1000.0f);
    m_camera->setMinZoomLevel(100.0f);
    m_camera->setTarget(QVector3D(1.0f, -1.0f, 1.0f));
    m_camera->setWrapXRotation(false);
    m_camera->setWrapYRotation(true);
    m_camera->setXRotation(30.0f);
    m_camera->setYRotation(30.0f);
    m_camera->setZoomLevel(500.0f);

    QCOMPARE(m_camera->cameraPreset(), Q3DCamera::CameraPresetNone);
    QCOMPARE(m_camera->maxZoomLevel(), 1000.0f);
    QCOMPARE(m_camera->minZoomLevel(), 100.0f);
    QCOMPARE(m_camera->target(), QVector3D(1.0f, -1.0f, 1.0f));
    QCOMPARE(m_camera->wrapXRotation(), false);
    QCOMPARE(m_camera->wrapYRotation(), true);
    QCOMPARE(m_camera->xRotation(), 30.0f);
    QCOMPARE(m_camera->yRotation(), 30.0f);
    QCOMPARE(m_camera->zoomLevel(), 500.0f);

    m_camera->setCameraPreset(Q3DCamera::CameraPresetBehind); // Sets target and rotations

    QCOMPARE(m_camera->cameraPreset(), Q3DCamera::CameraPresetBehind);
    QCOMPARE(m_camera->maxZoomLevel(), 1000.0f);
    QCOMPARE(m_camera->minZoomLevel(), 100.0f);
    QCOMPARE(m_camera->target(), QVector3D(0.0f, 0.0f, 0.0f));
    QCOMPARE(m_camera->wrapXRotation(), false);
    QCOMPARE(m_camera->wrapYRotation(), true);
    QCOMPARE(m_camera->xRotation(), 180.0f);
    QCOMPARE(m_camera->yRotation(), 22.5f);
    QCOMPARE(m_camera->zoomLevel(), 500.0f);

    m_camera->setCameraPosition(10.0f, 15.0f, 125.0f); // Overrides preset

    QCOMPARE(m_camera->cameraPreset(), Q3DCamera::CameraPresetNone);
    QCOMPARE(m_camera->maxZoomLevel(), 1000.0f);
    QCOMPARE(m_camera->minZoomLevel(), 100.0f);
    QCOMPARE(m_camera->target(), QVector3D(0.0f, 0.0f, 0.0f));
    QCOMPARE(m_camera->wrapXRotation(), false);
    QCOMPARE(m_camera->wrapYRotation(), true);
    QCOMPARE(m_camera->xRotation(), 10.0f);
    QCOMPARE(m_camera->yRotation(), 15.0f);
    QCOMPARE(m_camera->zoomLevel(), 125.0f);
}

QTEST_MAIN(tst_camera)
#include "tst_camera.moc"
