// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtTest/QtTest>
#include <QtQml/qqmlengine.h>
#include <QtQml/qqmlcomponent.h>

#include <QEasingCurve>
#include <QVector3D>

inline QUrl testFileUrl(const QString &fileName)
{
    static const QString dir = QTest::qFindTestData("data");

    QString result = dir;
    result += QLatin1Char('/');
    result += fileName;

    return QUrl::fromLocalFile(result);
}

class Tst_BlendTrees : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void checkImport();
    void testBlendAnimationNode();

};



void Tst_BlendTrees::checkImport()
{
    QQmlEngine engine;
    QQmlComponent component(&engine);
    component.setData("import QtQuick; import QtQuick.Timeline; import QtQuick.Timeline.BlendTrees; Item { }", QUrl());

    QScopedPointer<QObject> object(component.create());
    QVERIFY2(!object.isNull(), qPrintable(component.errorString()));
}

void Tst_BlendTrees::testBlendAnimationNode()
{
    QQmlEngine engine;
    QQmlComponent component(&engine);
    component.loadUrl(testFileUrl("BlendTreeTest.qml"));

    QScopedPointer<QObject> object(component.create());
    QVERIFY2(!object.isNull(), qPrintable(component.errorString()));

    // Get all of the necessary objects
    auto *timeline = object->findChild<QObject * >("timeline");
    auto *timelineAnimation1 = object->findChild<QObject *>("animation1");
    QVERIFY2(timelineAnimation1, "Could not find animation1");
    auto *timelineAnimation2 = object->findChild<QObject *>("animation2");
    QVERIFY2(timelineAnimation2, "Could not find animation2");
    auto *animation1Node = object->findChild<QObject *>("animation1Node");
    QVERIFY2(animation1Node, "Could not find animation1Node");
    auto *animation2Node = object->findChild<QObject *>("animation2Node");
    QVERIFY2(animation2Node, "Could not find animation2Node");
    auto *blendAnimation = object->findChild<QObject *>("blendAnimation");
    QVERIFY2(blendAnimation, "Could not find blendAnimation");
    auto *rectangle = object->findChild<QObject *>("rectangle");
    QVERIFY2(rectangle, "Could not find rectangle");
    auto *animation1Controller = object->findChild<QObject *>("animation1Controller");
    QVERIFY2(animation1Controller, "Could not find animation1Controller");
    auto *animation2Controller = object->findChild<QObject *>("animation2Controller");
    QVERIFY2(animation2Controller, "Could not find animation2Controller");


    // At this point nothing should be happening because the animations have controllers
    // attached to this which forces them to always be paused.  Starting states is:
    // animation1Node.currentFrame == 0
    // animation2Node.currentFrame == 100
    // rectangle.x = 100
    // rectangle.y = 100
    // rectangle.color = #ff7f00

    QCOMPARE(animation1Node->property("currentFrame").toInt(), 0);
    QCOMPARE(animation2Node->property("currentFrame").toInt(), 100);
    QCOMPARE(rectangle->property("x").toInt(), 100);
    QCOMPARE(rectangle->property("y").toInt(), 100);
    QCOMPARE(rectangle->property("color").value<QColor>(), QColor("#ff7f00"));

    // Push animation1 to end
    // Should be a blend of 50% blend of animation1 and and 50% blend of animation2
    // animation 1 should be at frame 100 (100%)
    // animation 2 should be at frame 100 (0%)
    animation1Controller->setProperty("progress", 1.0f);
    QCOMPARE(animation1Node->property("currentFrame").toInt(), 100);
    QCOMPARE(animation2Node->property("currentFrame").toInt(), 100);
    QCOMPARE(rectangle->property("x").toInt(), 200);
    QCOMPARE(rectangle->property("y").toInt(), 200);
    QCOMPARE(rectangle->property("color").value<QColor>(), QColor("#ffff00"));

    // Push animation2 to end
    // Should be a blend of 50% blend of animation1 and and 50% blend of animation2
    // animation 1 should be at frame 100 (100%)
    // animation 2 should be at frame 200 (100%)
    animation2Controller->setProperty("progress", 1.0f);
    QCOMPARE(animation1Node->property("currentFrame").toInt(), 100);
    QCOMPARE(animation2Node->property("currentFrame").toInt(), 200);
    QCOMPARE(rectangle->property("x").toInt(), 100);
    QCOMPARE(rectangle->property("y").toInt(), 300);
    QCOMPARE(rectangle->property("color").value<QColor>(), QColor("#ff7f7f"));

    // Change weight to 0.0
    // Should be a blend of 100% blend of animation1 and and 0% blend of animation2
    // animation 1 should be at frame 100 (100%)
    // animation 2 should be at frame 200 (100%)
    blendAnimation->setProperty("weight", 0.0f);
    QCOMPARE(animation1Node->property("currentFrame").toInt(), 100);
    QCOMPARE(animation2Node->property("currentFrame").toInt(), 200);
    QCOMPARE(rectangle->property("x").toInt(), 200);
    QCOMPARE(rectangle->property("y").toInt(), 200);
    QCOMPARE(rectangle->property("color").value<QColor>(), QColor("#ffff00"));

    // Change weight to 1.0
    // Should be a blend of 0% blend of animation1 and and 100% blend of animation2
    // animation 1 should be at frame 100 (100%)
    // animation 2 should be at frame 200 (100%)
    blendAnimation->setProperty("weight", 1.0f);
    QCOMPARE(animation1Node->property("currentFrame").toInt(), 100);
    QCOMPARE(animation2Node->property("currentFrame").toInt(), 200);
    QCOMPARE(rectangle->property("x").toInt(), 0);
    QCOMPARE(rectangle->property("y").toInt(), 400);
    QCOMPARE(rectangle->property("color").value<QColor>(), QColor("#ff00ff"));

    // Change animation1 to start (should be the same as previous)
    // Should be a blend of 0% blend of animation1 and and 100% blend of animation2
    // animation 1 should be at frame 0 (0%)
    // animation 2 should be at frame 200 (100%)
    animation1Controller->setProperty("progress", 0.0f);
    QCOMPARE(animation1Node->property("currentFrame").toInt(), 0);
    QCOMPARE(animation2Node->property("currentFrame").toInt(), 200);
    QCOMPARE(rectangle->property("x").toInt(), 0);
    QCOMPARE(rectangle->property("y").toInt(), 400);
    QCOMPARE(rectangle->property("color").value<QColor>(), QColor("#ff00ff"));

    // Change weight to 0.0
    // Should be a blend of 100% blend of animation1 and and 0% blend of animation2
    // animation 1 should be at frame 0 (0%)
    // animation 2 should be at frame 200 (100%)
    blendAnimation->setProperty("weight", 0.0f);
    QCOMPARE(animation1Node->property("currentFrame").toInt(), 0);
    QCOMPARE(animation2Node->property("currentFrame").toInt(), 200);
    QCOMPARE(rectangle->property("x").toInt(), 0);
    QCOMPARE(rectangle->property("y").toInt(), 0);
    QCOMPARE(rectangle->property("color").value<QColor>(), QColor("#ff0000"));

    // Change animation2 to start (should be the same as previous)
    // Should be a blend of 100% blend of animation1 and and 0% blend of animation2
    // animation 1 should be at frame 0 (0%)
    // animation 2 should be at frame 100 (0%)
    animation2Controller->setProperty("progress", 0.0f);
    QCOMPARE(animation1Node->property("currentFrame").toInt(), 0);
    QCOMPARE(animation2Node->property("currentFrame").toInt(), 100);
    QCOMPARE(rectangle->property("x").toInt(), 0);
    QCOMPARE(rectangle->property("y").toInt(), 0);
    QCOMPARE(rectangle->property("color").value<QColor>(), QColor("#ff0000"));

    // Disable committing of changes by animationBlendNode
    blendAnimation->setProperty("outputEnabled", false);
    // Now changing either animation progress or weight should have no effect on the scene
    animation1Controller->setProperty("progress", 1.0f);
    QCOMPARE(animation1Node->property("currentFrame").toInt(), 100);
    QCOMPARE(animation2Node->property("currentFrame").toInt(), 100);
    QCOMPARE(rectangle->property("x").toInt(), 0);
    QCOMPARE(rectangle->property("y").toInt(), 0);
    QCOMPARE(rectangle->property("color").value<QColor>(), QColor("#ff0000"));

    animation2Controller->setProperty("progress", 1.0f);
    QCOMPARE(animation1Node->property("currentFrame").toInt(), 100);
    QCOMPARE(animation2Node->property("currentFrame").toInt(), 200);
    QCOMPARE(rectangle->property("x").toInt(), 0);
    QCOMPARE(rectangle->property("y").toInt(), 0);
    QCOMPARE(rectangle->property("color").value<QColor>(), QColor("#ff0000"));

    blendAnimation->setProperty("weight", 0.5f);
    QCOMPARE(animation1Node->property("currentFrame").toInt(), 100);
    QCOMPARE(animation2Node->property("currentFrame").toInt(), 200);
    QCOMPARE(rectangle->property("x").toInt(), 0);
    QCOMPARE(rectangle->property("y").toInt(), 0);
    QCOMPARE(rectangle->property("color").value<QColor>(), QColor("#ff0000"));

    // re-enable committing of changes by animationBlendNode
    // This should cause the animation to blend to the new state
    blendAnimation->setProperty("outputEnabled", true);
    QCOMPARE(animation1Node->property("currentFrame").toInt(), 100);
    QCOMPARE(animation2Node->property("currentFrame").toInt(), 200);
    QCOMPARE(rectangle->property("x").toInt(), 100);
    QCOMPARE(rectangle->property("y").toInt(), 300);
    QCOMPARE(rectangle->property("color").value<QColor>(), QColor("#ff7f7f"));

    // Test disconnecting animation1
    blendAnimation->setProperty("source1", QVariant());

    animation1Controller->setProperty("progress", 0.0f);
    QCOMPARE(animation1Node->property("currentFrame").toInt(), 0);
    QCOMPARE(animation2Node->property("currentFrame").toInt(), 200);
    QCOMPARE(rectangle->property("x").toInt(), 100);
    QCOMPARE(rectangle->property("y").toInt(), 300);
    QCOMPARE(rectangle->property("color").value<QColor>(), QColor("#ff7f7f"));

    // Test disconnecting animation2
    blendAnimation->setProperty("source2", QVariant());
    animation2Controller->setProperty("progress", 0.0f);
    QCOMPARE(animation1Node->property("currentFrame").toInt(), 0);
    QCOMPARE(animation2Node->property("currentFrame").toInt(), 100);
    QCOMPARE(rectangle->property("x").toInt(), 100);
    QCOMPARE(rectangle->property("y").toInt(), 300);
    QCOMPARE(rectangle->property("color").value<QColor>(), QColor("#ff7f7f"));

    // Disable committing of changes by animationBlendNode
    blendAnimation->setProperty("outputEnabled", false);

    // Try outputting dirrectly from animation1
    animation1Node->setProperty("outputEnabled", true);
    // Should have changed to be the value of animation1 at frame 0
    QCOMPARE(rectangle->property("x").toInt(), 0);
    QCOMPARE(rectangle->property("y").toInt(), 0);
    QCOMPARE(rectangle->property("color").value<QColor>(), QColor("#ff0000"));

    animation1Controller->setProperty("progress", 1.0f);
    // Should have changed to be the value of animation1 at frame 100
    QCOMPARE(rectangle->property("x").toInt(), 200);
    QCOMPARE(rectangle->property("y").toInt(), 200);
    QCOMPARE(rectangle->property("color").value<QColor>(), QColor("#ffff00"));

    // Disable outputting dirrectly from animation1
    animation1Node->setProperty("outputEnabled", false);
    // Try outputting dirrectly from animation2
    animation2Node->setProperty("outputEnabled", true);
    // Nothing should have changed since both would be at frame 100 anyway
    QCOMPARE(animation1Node->property("currentFrame").toInt(), 100);
    QCOMPARE(animation2Node->property("currentFrame").toInt(), 100);
    QCOMPARE(rectangle->property("x").toInt(), 200);
    QCOMPARE(rectangle->property("y").toInt(), 200);
    QCOMPARE(rectangle->property("color").value<QColor>(), QColor("#ffff00"));

    animation2Controller->setProperty("progress", 1.0f);
    QCOMPARE(animation1Node->property("currentFrame").toInt(), 100);
    QCOMPARE(animation2Node->property("currentFrame").toInt(), 200);
    QCOMPARE(rectangle->property("x").toInt(), 0);
    QCOMPARE(rectangle->property("y").toInt(), 400);
    QCOMPARE(rectangle->property("color").value<QColor>(), QColor("#ff00ff"));

    // Try breaking animation2Node's connection to timeline (the source of frame data)
    // currentFrame should change but the output should not
    animation2Node->setProperty("timeline", QVariant());
    animation2Controller->setProperty("progress", 0.0f);
    QCOMPARE(animation2Node->property("currentFrame").toInt(), 100);
    QCOMPARE(rectangle->property("x").toInt(), 0);
    QCOMPARE(rectangle->property("y").toInt(), 400);
    QCOMPARE(rectangle->property("color").value<QColor>(), QColor("#ff00ff"));

    // reattach the timeline
    animation2Node->setProperty("timeline", QVariant::fromValue(timeline));
    // Should update the output now that we can fetch frameData for frame 100
    QCOMPARE(animation2Node->property("currentFrame").toInt(), 100);
    QCOMPARE(rectangle->property("x").toInt(), 200);
    QCOMPARE(rectangle->property("y").toInt(), 200);
    QCOMPARE(rectangle->property("color").value<QColor>(), QColor("#ffff00"));

    // Try breaking animation2Node's connection to animation
    animation2Node->setProperty("animation", QVariant());
    animation2Controller->setProperty("progress", 1.0f);
    QCOMPARE(rectangle->property("x").toInt(), 200);
    QCOMPARE(rectangle->property("y").toInt(), 200);
    QCOMPARE(rectangle->property("color").value<QColor>(), QColor("#ffff00"));

    // reattach the animation
    animation2Node->setProperty("animation", QVariant::fromValue(timelineAnimation2));
    // Should update the output now that we can fetch frameData for frame 200
    QCOMPARE(rectangle->property("x").toInt(), 0);
    QCOMPARE(rectangle->property("y").toInt(), 400);
    QCOMPARE(rectangle->property("color").value<QColor>(), QColor("#ff00ff"));
}

QTEST_MAIN(Tst_BlendTrees)

#include "tst_blendtrees.moc"
