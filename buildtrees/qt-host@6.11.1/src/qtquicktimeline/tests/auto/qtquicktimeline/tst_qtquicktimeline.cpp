// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtTest/QtTest>
#include <QtQml/qqmlengine.h>
#include <QtQml/qqmlcomponent.h>

#include <QEasingCurve>
#include <QVector3D>

class Tst_QtQuickTimeline : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void checkImport();
    void simpleTest();
    void parameterization();
    void vectors();
    void deltaFunction();
    void keyframeUpdate();
    void easingcurveInterpolation();
    void restoreBindingTest();
};

inline QUrl testFileUrl(const QString &fileName)
{
    static const QString dir = QTest::qFindTestData("data");

    QString result = dir;
    result += QLatin1Char('/');
    result += fileName;

    return QUrl::fromLocalFile(result);
}

void Tst_QtQuickTimeline::checkImport()
{
    QQmlEngine engine;
    QQmlComponent component(&engine);
    component.setData("import QtQuick 2.0; import QtQuick.Timeline 1.0; Item { }", QUrl());

    QScopedPointer<QObject> object(component.create());
    QVERIFY2(!object.isNull(), qPrintable(component.errorString()));
}

void Tst_QtQuickTimeline::simpleTest()
{
    QQmlEngine engine;
    QQmlComponent component(&engine);
    component.loadUrl(testFileUrl("simpletest.qml"));

    QScopedPointer<QObject> object(component.create());
    QVERIFY2(!object.isNull(), qPrintable(component.errorString()));

    auto  *rectangle = object->findChild<QObject *>("rectangle");
    QVERIFY(rectangle);

    QCOMPARE(rectangle->property("width").toInt(), 20);
    QCOMPARE(rectangle->property("height").toInt(), 20);
    QCOMPARE(rectangle->property("color").value<QColor>(), QColor("blue"));
    QCOMPARE(rectangle->property("x").toInt(), 100);
    QCOMPARE(rectangle->property("y").toInt(), 100);

    auto *timeline = object->findChild<QObject *>("timeline");
    QVERIFY(timeline);

    QCOMPARE(timeline->property("enabled").toBool(), true);
    QCOMPARE(timeline->property("startFrame").toInt(), 0);
    QCOMPARE(timeline->property("endFrame").toInt(), 100);
    QCOMPARE(timeline->property("currentFrame").toInt(), 50);

    auto *animation = object->findChild<QObject *>("animation");
    QVERIFY(animation);

    QCOMPARE(animation->property("running").toBool(), false);
    QCOMPARE(animation->property("duration").toInt(), 200);
    QCOMPARE(animation->property("loops").toInt(), 1);
    QCOMPARE(animation->property("from").toInt(), 0);
    QCOMPARE(animation->property("to").toInt(), 100);

    auto *group = object->findChild<QObject *>("group01");
    QVERIFY(group);

    QCOMPARE(group->property("target").value<QObject*>(), rectangle);

    timeline->setProperty("currentFrame", 0);

    QCOMPARE(rectangle->property("color").value<QColor>(), QColor("red"));
    QCOMPARE(rectangle->property("x").toInt(), 0);
    QCOMPARE(rectangle->property("y").toInt(), 0);

    timeline->setProperty("currentFrame", 100);

    QCOMPARE(rectangle->property("color").value<QColor>(), QColor("yellow"));
    QCOMPARE(rectangle->property("x").toInt(), 200);
    QCOMPARE(rectangle->property("y").toInt(), 200);

    timeline->setProperty("enabled", false);
    QCOMPARE(rectangle->property("color").value<QColor>(), QColor("red"));
    QCOMPARE(rectangle->property("x").toInt(), 0);
    QCOMPARE(rectangle->property("y").toInt(), 0);

    timeline->setProperty("currentFrame", 0);
    timeline->setProperty("enabled", true);

    animation->setProperty("running", true);
    QCOMPARE(animation->property("running").toBool(), true);
}

void Tst_QtQuickTimeline::parameterization()
{
    QQmlEngine engine;
    QQmlComponent component(&engine);
    component.loadUrl(testFileUrl("parameterization.qml"));

    QScopedPointer<QObject> object(component.create());
    QVERIFY2(!object.isNull(), qPrintable(component.errorString()));

    auto *timeline = object->findChild<QObject *>("timeline");
    QVERIFY(timeline);

    QCOMPARE(timeline->property("enabled").toBool(), true);
    QCOMPARE(timeline->property("startFrame").toInt(), 0);
    QCOMPARE(timeline->property("endFrame").toInt(), 200);
    QCOMPARE(timeline->property("currentFrame").toInt(), 10);

    auto *needle = object->findChild<QObject *>("needle");
    QVERIFY(needle);

    QCOMPARE(needle->property("width").toInt(), 150);
    QCOMPARE(needle->property("height").toInt(), 4);
    QCOMPARE(needle->property("x").toInt(), 0);
    QCOMPARE(needle->property("y").toInt(), 148);

    auto *group = object->findChild<QObject *>("group01");
    QVERIFY(group);

    QCOMPARE(group->property("target").value<QObject*>(), needle);

    auto *textInput = object->findChild<QObject *>("textInput");
    QVERIFY(textInput);

    QCOMPARE(textInput->property("text").toString(), "10");

    textInput->setProperty("text", "0");

    QCOMPARE(needle->property("color").value<QColor>(), QColor("blue"));

    timeline->setProperty("enabled", false);
    QCOMPARE(needle->property("color").value<QColor>(), QColor("#c41616"));
    QCOMPARE(needle->property("rotation").toInt(), 0);
    timeline->setProperty("enabled", true);

    textInput->setProperty("text", "100");
    QCOMPARE(needle->property("color").value<QColor>(), QColor("green"));
    QCOMPARE(needle->property("rotation").toInt(), 90);


    textInput->setProperty("text", "200");
    QCOMPARE(needle->property("color").value<QColor>(), QColor("red"));
    QCOMPARE(needle->property("rotation").toInt(), 180);
}


void Tst_QtQuickTimeline::Tst_QtQuickTimeline::vectors()
{
    QQmlEngine engine;
    QQmlComponent component(&engine);
    component.loadUrl(testFileUrl("vectors.qml"));

    QScopedPointer<QObject> object(component.create());
    QVERIFY2(!object.isNull(), qPrintable(component.errorString()));

    auto *timeline = object->findChild<QObject *>("timeline");
    QVERIFY(timeline);

    QCOMPARE(timeline->property("enabled").toBool(), true);
    QCOMPARE(timeline->property("startFrame").toInt(), 0);
    QCOMPARE(timeline->property("endFrame").toInt(), 200);
    QCOMPARE(timeline->property("currentFrame").toInt(), 0);

    auto *rotation = object->findChild<QObject *>("rotation");
    QVERIFY(rotation);

    auto vector = rotation->property("origin").value<QVector3D>();

    QVERIFY(!vector.isNull());
    QCOMPARE(vector.x(), 0.0);
    QCOMPARE(vector.y(), 30.0);

    vector = rotation->property("axis").value<QVector3D>();

    QVERIFY(!vector.isNull());
    QCOMPARE(vector.x(), 0.0);
    QCOMPARE(vector.y(), 1.0);
    QCOMPARE(vector.z(), 0.0);

    timeline->setProperty("currentFrame", 50);

    vector = rotation->property("axis").value<QVector3D>();

    QVERIFY(!vector.isNull());
    QCOMPARE(vector.x(), 1.0);
    QCOMPARE(vector.y(), 1.0);
    QCOMPARE(vector.z(), 0.0);

    vector = rotation->property("origin").value<QVector3D>();

    QVERIFY(!vector.isNull());
    QCOMPARE(vector.x(), 10.0);
    QCOMPARE(vector.y(), 30.0);

    timeline->setProperty("currentFrame", 100);

    vector = rotation->property("axis").value<QVector3D>();

    QVERIFY(!vector.isNull());
    QCOMPARE(vector.x(), 0.0);
    QCOMPARE(vector.y(), 1.0);
    QCOMPARE(vector.z(), 0.0);

    vector = rotation->property("origin").value<QVector3D>();

    QVERIFY(!vector.isNull());
    QCOMPARE(vector.x(), 20.0);
    QCOMPARE(vector.y(), 30.0);

    timeline->setProperty("enabled", false);

    vector = rotation->property("origin").value<QVector3D>();

    QVERIFY(!vector.isNull());
    QCOMPARE(vector.x(), 30.0);
    QCOMPARE(vector.y(), 30.0);
}

void Tst_QtQuickTimeline::deltaFunction()
{
    QQmlEngine engine;
    QQmlComponent component(&engine);
    component.loadUrl(testFileUrl("deltafunction.qml"));

    QScopedPointer<QObject> object(component.create());
    QVERIFY2(!object.isNull(), qPrintable(component.errorString()));

    auto *timeline = object->findChild<QObject *>("timeline");
    QVERIFY(timeline);

    QCOMPARE(timeline->property("enabled").toBool(), true);
    QCOMPARE(timeline->property("startFrame").toInt(), 0);
    QCOMPARE(timeline->property("endFrame").toInt(), 100);
    QCOMPARE(timeline->property("currentFrame").toInt(), 0);

    auto *text = object->findChild<QObject *>("text");
    QVERIFY(text);

    QCOMPARE(text->property("text").toString(), "frame0");

    auto *group = object->findChild<QObject *>("group01");
    QVERIFY(group);

    QCOMPARE(group->property("target").value<QObject*>(), text);

    timeline->setProperty("enabled", false);
    QCOMPARE(text->property("text").toString(), "no timeline");

    timeline->setProperty("enabled", true);
    QCOMPARE(text->property("text").toString(), "frame0");

    timeline->setProperty("currentFrame", 49);
    QCOMPARE(text->property("text").toString(), "frame0");

    timeline->setProperty("currentFrame", 50);
    QCOMPARE(text->property("text").toString(), "frame50");

    timeline->setProperty("currentFrame", 51);
    QCOMPARE(text->property("text").toString(), "frame50");

    timeline->setProperty("currentFrame", 49);
    QCOMPARE(text->property("text").toString(), "frame0");

    timeline->setProperty("currentFrame", 99);
    QCOMPARE(text->property("text").toString(), "frame50");

    timeline->setProperty("currentFrame", 100);
    QCOMPARE(text->property("text").toString(), "frame100");

    timeline->setProperty("currentFrame", 101);
    QCOMPARE(text->property("text").toString(), "frame100");
}

void Tst_QtQuickTimeline::keyframeUpdate()
{
    QQmlEngine engine;
    QQmlComponent component(&engine);
    component.loadUrl(testFileUrl("simpletest.qml"));

    QScopedPointer<QObject> object(component.create());
    QVERIFY2(!object.isNull(), qPrintable(component.errorString()));

    auto *timeline = object->findChild<QObject *>("timeline");
    QVERIFY(timeline);

    QCOMPARE(timeline->property("enabled").toBool(), true);
    QCOMPARE(timeline->property("startFrame").toInt(), 0);
    QCOMPARE(timeline->property("endFrame").toInt(), 100);
    QCOMPARE(timeline->property("currentFrame").toInt(), 50);

    auto  *rectangle = object->findChild<QObject *>("rectangle");
    QVERIFY(rectangle);

    QCOMPARE(rectangle->property("x").toInt(), 100);

    auto  *keyframe = object->findChild<QObject *>("keyframe");
    QVERIFY(keyframe);

    QCOMPARE(keyframe->property("frame").toInt(), 50);
    QCOMPARE(keyframe->property("value").toInt(), 100);

    keyframe->setProperty("value", 70);
    QCOMPARE(keyframe->property("value").toInt(), 70);
    QCOMPARE(rectangle->property("x").toInt(), 70);

    keyframe->setProperty("value", 90);
    QCOMPARE(keyframe->property("value").toInt(), 90);
    QCOMPARE(rectangle->property("x").toInt(), 90);

    timeline->setProperty("currentFrame", 60);
    QCOMPARE(timeline->property("currentFrame").toInt(), 60);

    QVERIFY(rectangle->property("x").toInt() != 90);
    keyframe->setProperty("frame", 60);
    QCOMPARE(rectangle->property("x").toInt(), 90);
}

void Tst_QtQuickTimeline::easingcurveInterpolation()
{
    QQmlEngine engine;
    QQmlComponent component(&engine);
    component.loadUrl(testFileUrl("simpletest.qml"));

    QScopedPointer<QObject> object(component.create());
    QVERIFY2(!object.isNull(), qPrintable(component.errorString()));

    auto *timeline = object->findChild<QObject *>("timeline");
    QVERIFY(timeline);

    QCOMPARE(timeline->property("enabled").toBool(), true);
    QCOMPARE(timeline->property("startFrame").toInt(), 0);
    QCOMPARE(timeline->property("endFrame").toInt(), 100);
    QCOMPARE(timeline->property("currentFrame").toInt(), 50);

    auto  *rectangle = object->findChild<QObject *>("rectangle");
    QVERIFY(rectangle);

    QCOMPARE(rectangle->property("y").toInt(), 100);

    auto  *keyframe = object->findChild<QObject *>("easingBounce");
    QVERIFY(keyframe);

    QCOMPARE(keyframe->property("frame").toInt(), 100);
    QCOMPARE(keyframe->property("value").toInt(), 200);

    auto easingCurve = keyframe->property("easing").value<QEasingCurve>();
    QCOMPARE(easingCurve.type(), QEasingCurve::InBounce);

    timeline->setProperty("currentFrame", 100);
    QCOMPARE(rectangle->property("y").toInt(), 200);

    timeline->setProperty("currentFrame", 75);
    qreal progress = easingCurve.valueForProgress(0.5);
    qreal final = (1 - progress) * 100 + progress * 200;
    QCOMPARE(rectangle->property("y").toReal(), final);

    timeline->setProperty("currentFrame", 90);
    progress = easingCurve.valueForProgress(0.8);
    final = (1 - progress) * 100 + progress * 200;
    QCOMPARE(rectangle->property("y").toReal(), final);

    timeline->setProperty("currentFrame", 95);
    progress = easingCurve.valueForProgress(0.9);
    final = (1 - progress) * 100 + progress * 200;
    QCOMPARE(rectangle->property("y").toReal(), final);

    timeline->setProperty("currentFrame", 55);
    progress = easingCurve.valueForProgress(0.1);
    final = (1 - progress) * 100 + progress * 200;
    QCOMPARE(rectangle->property("y").toReal(), final);

    timeline->setProperty("currentFrame", 60);
    progress = easingCurve.valueForProgress(0.2);
    final = (1 - progress) * 100 + progress * 200;
    QCOMPARE(rectangle->property("y").toReal(), final);
}

void Tst_QtQuickTimeline::restoreBindingTest()
{
    QQmlEngine engine;
    QQmlComponent component(&engine);
    component.loadUrl(testFileUrl("restorebindingtest.qml"));

    QScopedPointer<QObject> object(component.create());
    QVERIFY2(!object.isNull(), qPrintable(component.errorString()));

    auto *rectangle = object->findChild<QObject *>("rectangle");
    QVERIFY(rectangle);

    QCOMPARE(rectangle->property("offset").toInt(), 0);
    QCOMPARE(rectangle->property("x").toInt(), 0);
    QCOMPARE(rectangle->property("y").toInt(), 0);

    auto *timeline = object->findChild<QObject *>("timeline");
    QVERIFY(timeline);

    QCOMPARE(timeline->property("enabled").toBool(), false);
    QCOMPARE(timeline->property("startFrame").toInt(), 0);
    QCOMPARE(timeline->property("endFrame").toInt(), 100);
    QCOMPARE(timeline->property("currentFrame").toInt(), 50);

    // currentFrame == 50
    timeline->setProperty("enabled", true);
    QCOMPARE(rectangle->property("x").toInt(), 70);
    QCOMPARE(rectangle->property("y").toInt(), 70);

    timeline->setProperty("currentFrame", 0);
    QCOMPARE(rectangle->property("x").toInt(), 140);
    QCOMPARE(rectangle->property("y").toInt(), 0);

    timeline->setProperty("currentFrame", 100);
    QCOMPARE(rectangle->property("x").toInt(), 0);
    QCOMPARE(rectangle->property("y").toInt(), 140);


    timeline->setProperty("enabled", false);
    // check restoring of the original binding
    QCOMPARE(rectangle->property("x").toInt(), 0);
    QCOMPARE(rectangle->property("y").toInt(), 0);
    rectangle->setProperty("offset", 50);
    QCOMPARE(rectangle->property("x").toInt(), 50);
    QCOMPARE(rectangle->property("y").toInt(), 50);

    timeline->setProperty("enabled", true);
    // currentFrame == 100, offset == 50
    QCOMPARE(rectangle->property("x").toInt(), 50);
    QCOMPARE(rectangle->property("y").toInt(), 140);
}
QTEST_MAIN(Tst_QtQuickTimeline)

#include "tst_qtquicktimeline.moc"
