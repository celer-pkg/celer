// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QTest>

#include <QtQuick3D/private/qquick3dreflectionprobe_p.h>

#include <QtQuick3DRuntimeRender/private/qssgrenderreflectionprobe_p.h>

class tst_QQuick3DReflectionProbe : public QObject
{
    Q_OBJECT

    // Work-around to get access to updateSpatialNode
    class ReflectionProbe : public QQuick3DReflectionProbe
    {
    public:
        using QQuick3DReflectionProbe::updateSpatialNode;
    };

private slots:
    void testProperties();
    void testEnums();
};

void tst_QQuick3DReflectionProbe::testProperties()
{
    ReflectionProbe probe;
    auto node = static_cast<QSSGRenderReflectionProbe*>(probe.updateSpatialNode(nullptr));
    const auto originalNode = node;
    QVERIFY(node);

    probe.setParallaxCorrection(true);
    node = static_cast<QSSGRenderReflectionProbe*>(probe.updateSpatialNode(node));
    QVERIFY(probe.parallaxCorrection());
    QVERIFY(node->parallaxCorrection);
    probe.setParallaxCorrection(false);
    node = static_cast<QSSGRenderReflectionProbe*>(probe.updateSpatialNode(node));
    QVERIFY(!probe.parallaxCorrection());
    QVERIFY(!node->parallaxCorrection);

    probe.setDebugView(true);
    QVERIFY(probe.debugView());
    probe.setDebugView(false);
    QVERIFY(!probe.debugView());

    QVector3D size(20, 40, 60);
    probe.setBoxSize(size);
    node = static_cast<QSSGRenderReflectionProbe*>(probe.updateSpatialNode(node));
    QCOMPARE(size, probe.boxSize());
    QCOMPARE(size, node->boxSize);

    QVector3D offset(10, 30, 50);
    probe.setBoxOffset(offset);
    node = static_cast<QSSGRenderReflectionProbe*>(probe.updateSpatialNode(node));
    QCOMPARE(offset, probe.boxOffset());
    QCOMPARE(offset, node->boxOffset);

    QColor clearColor(Qt::black);
    probe.setClearColor(clearColor);
    node = static_cast<QSSGRenderReflectionProbe*>(probe.updateSpatialNode(node));
    QCOMPARE(clearColor, probe.clearColor());
    QCOMPARE(clearColor, node->clearColor);

    QCOMPARE(originalNode, node);
}

void tst_QQuick3DReflectionProbe::testEnums()
{
    ReflectionProbe probe;
    auto node = static_cast<QSSGRenderReflectionProbe*>(probe.updateSpatialNode(nullptr));
    QVERIFY(node);

    auto reflectionQualities = { QQuick3DReflectionProbe::ReflectionQuality::VeryLow,
                                 QQuick3DReflectionProbe::ReflectionQuality::Low,
                                 QQuick3DReflectionProbe::ReflectionQuality::Medium,
                                 QQuick3DReflectionProbe::ReflectionQuality::High,
                                 QQuick3DReflectionProbe::ReflectionQuality::VeryHigh };
    const unsigned int mappedResolutions[] = {7, 8, 9, 10, 11};
    int idx = 0;
    for (const auto quality : reflectionQualities) {
        probe.setQuality(quality);
        node = static_cast<QSSGRenderReflectionProbe*>(probe.updateSpatialNode(node));
        QCOMPARE(quality, probe.quality());
        QCOMPARE(mappedResolutions[idx], node->reflectionMapRes);
        idx++;
    }

    auto refreshModes = { QQuick3DReflectionProbe::ReflectionRefreshMode::FirstFrame,
                          QQuick3DReflectionProbe::ReflectionRefreshMode::EveryFrame };
    for (const auto mode : refreshModes) {
        probe.setRefreshMode(mode);
        node = static_cast<QSSGRenderReflectionProbe*>(probe.updateSpatialNode(node));
        QCOMPARE(int(mode), int(probe.refreshMode()));
        QCOMPARE(int(mode), int(node->refreshMode));
    }

    auto timeSlicings = { QQuick3DReflectionProbe::ReflectionTimeSlicing::None,
                          QQuick3DReflectionProbe::ReflectionTimeSlicing::AllFacesAtOnce,
                          QQuick3DReflectionProbe::ReflectionTimeSlicing::IndividualFaces };
    for (const auto timeSlice : timeSlicings) {
        probe.setTimeSlicing(timeSlice);
        node = static_cast<QSSGRenderReflectionProbe*>(probe.updateSpatialNode(node));
        QCOMPARE(int(timeSlice), int(probe.timeSlicing()));
        QCOMPARE(int(timeSlice), int(node->timeSlicing));
    }
}

QTEST_APPLESS_MAIN(tst_QQuick3DReflectionProbe)
#include "tst_qquick3dreflectionprobe.moc"
