// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtTest/QTest>
#include <QtTest/QSignalSpy>
#include <QtQuickShapesDesignHelpers/private/qquickrectangleshape_p.h>
#include <QtQuickTest/quicktest.h>
#include <QtQuickTestUtils/private/visualtestutils_p.h>

using namespace QQuickVisualTestUtils;

class tst_QQuickRectangleShape : public QQmlDataTest
{
    Q_OBJECT
public:
    tst_QQuickRectangleShape();

private slots:
    void initTestCase() override;

    void changeSignals_data();
    void changeSignals();
    void radii();
    void bevel();

private:
    // If we load the QML from scratch every test, each row takes ~350ms.
    // However, we still want it to be data-driven (so that e.g. you can
    // run individual rows), so we store it here and create it once at startup
    // rather than loop through a list in changeSignals.
    // TODO: clean this up when we have a callback that is run after all rows
    // have been run for a test function: QTBUG-134198
    std::unique_ptr<QQuickApplicationHelper> changeSignalTestAppHelper;
};

tst_QQuickRectangleShape::tst_QQuickRectangleShape()
    : QQmlDataTest(QT_QMLTEST_DATADIR, QQmlDataTest::FailOnWarningsPolicy::FailOnWarnings)
{
}

void tst_QQuickRectangleShape::initTestCase()
{
    QQmlDataTest::initTestCase();
    changeSignalTestAppHelper.reset(new QQuickApplicationHelper(this, "default.qml"));
}

void tst_QQuickRectangleShape::changeSignals_data()
{
    QTest::addColumn<QVariant>("propertyValue");
    QTest::addColumn<QMetaMethod>("changeSignal");

    // radius and bevel are more complex and therefore tested separately.
    QTest::newRow("width") << QVariant::fromValue(150)
        << QMetaMethod::fromSignal(&QQuickItem::widthChanged);
    QTest::newRow("height") << QVariant::fromValue(100)
        << QMetaMethod::fromSignal(&QQuickItem::heightChanged);
    QTest::newRow("drawTop") << QVariant::fromValue(false)
        << QMetaMethod::fromSignal(&QQuickRectangleShape::drawTopChanged);
    QTest::newRow("drawRight") << QVariant::fromValue(false)
        << QMetaMethod::fromSignal(&QQuickRectangleShape::drawRightChanged);
    QTest::newRow("drawBottom") << QVariant::fromValue(false)
        << QMetaMethod::fromSignal(&QQuickRectangleShape::drawBottomChanged);
    QTest::newRow("drawLeft") << QVariant::fromValue(false)
        << QMetaMethod::fromSignal(&QQuickRectangleShape::drawLeftChanged);
    QTest::newRow("strokeColor") << QVariant::fromValue(QColorConstants::Svg::purple)
        << QMetaMethod::fromSignal(&QQuickRectangleShape::strokeColorChanged);
    QTest::newRow("strokeWidth") << QVariant::fromValue(0)
        << QMetaMethod::fromSignal(&QQuickRectangleShape::strokeWidthChanged);
    QTest::newRow("fillColor") << QVariant::fromValue(QColorConstants::Svg::purple)
        << QMetaMethod::fromSignal(&QQuickRectangleShape::fillColorChanged);
    QTest::newRow("joinStyle") << QVariant::fromValue(QQuickShapePath::RoundJoin)
        << QMetaMethod::fromSignal(&QQuickRectangleShape::joinStyleChanged);
    QTest::newRow("capStyle") << QVariant::fromValue(QQuickShapePath::RoundCap)
        << QMetaMethod::fromSignal(&QQuickRectangleShape::capStyleChanged);
    QTest::newRow("strokeStyle") << QVariant::fromValue(QQuickShapePath::DashLine)
        << QMetaMethod::fromSignal(&QQuickRectangleShape::strokeStyleChanged);
    QTest::newRow("dashOffset") << QVariant::fromValue(4)
        << QMetaMethod::fromSignal(&QQuickRectangleShape::dashOffsetChanged);
    QTest::newRow("dashPattern") << QVariant::fromValue(QList<qreal>{1, 2})
        << QMetaMethod::fromSignal(&QQuickRectangleShape::dashPatternChanged);
    QTest::newRow("borderMode") << QVariant::fromValue(QQuickRectangleShape::BorderMode::Outside)
        << QMetaMethod::fromSignal(&QQuickRectangleShape::borderModeChanged);
}

void tst_QQuickRectangleShape::changeSignals()
{
    QFETCH(const QVariant, propertyValue);
    QFETCH(const QMetaMethod, changeSignal);

    QVERIFY2(changeSignalTestAppHelper->ready, changeSignalTestAppHelper->failureMessage());
    QQuickWindow *window = changeSignalTestAppHelper->window;
    window->show();
    QVERIFY(QTest::qWaitForWindowExposed(window));

    auto *rectangleShape = window->property("rectangleShape").value<QQuickRectangleShape *>();
    QVERIFY(rectangleShape);
    const QSignalSpy signalSpy(rectangleShape, changeSignal);
    QVERIFY(signalSpy.isValid());
    QVERIFY(rectangleShape->setProperty(QTest::currentDataTag(), propertyValue));
    QCOMPARE(signalSpy.count(), 1);
}

void tst_QQuickRectangleShape::radii()
{
    QQuickApplicationHelper helper(this, "default.qml");
    QVERIFY2(helper.ready, helper.failureMessage());
    QQuickWindow *window = helper.window;
    window->show();
    QVERIFY(QTest::qWaitForWindowExposed(window));

    auto *rectangleShape = window->property("rectangleShape").value<QQuickRectangleShape *>();
    QVERIFY(rectangleShape);

    const QSignalSpy radiusSignalSpy(rectangleShape, &QQuickRectangleShape::radiusChanged);
    QVERIFY(radiusSignalSpy.isValid());
    const QSignalSpy topLeftRadiusSignalSpy(rectangleShape, &QQuickRectangleShape::topLeftRadiusChanged);
    QVERIFY(topLeftRadiusSignalSpy.isValid());
    const QSignalSpy topRightRadiusSignalSpy(rectangleShape, &QQuickRectangleShape::topRightRadiusChanged);
    QVERIFY(topRightRadiusSignalSpy.isValid());
    const QSignalSpy bottomLeftRadiusSignalSpy(rectangleShape, &QQuickRectangleShape::bottomLeftRadiusChanged);
    QVERIFY(bottomLeftRadiusSignalSpy.isValid());
    const QSignalSpy bottomRightRadiusSignalSpy(rectangleShape, &QQuickRectangleShape::bottomRightRadiusChanged);
    QVERIFY(bottomRightRadiusSignalSpy.isValid());

    // Setting radius should affect all of the other radii properties that aren't explicitly set.
    rectangleShape->setRadius(1);
    QCOMPARE(rectangleShape->radius(), 1);
    QCOMPARE(radiusSignalSpy.count(), 1);
    QCOMPARE(rectangleShape->topLeftRadius(), 1);
    QCOMPARE(topLeftRadiusSignalSpy.count(), 1);
    QCOMPARE(rectangleShape->topRightRadius(), 1);
    QCOMPARE(topRightRadiusSignalSpy.count(), 1);
    QCOMPARE(rectangleShape->bottomLeftRadius(), 1);
    QCOMPARE(bottomLeftRadiusSignalSpy.count(), 1);
    QCOMPARE(rectangleShape->bottomRightRadius(), 1);
    QCOMPARE(bottomRightRadiusSignalSpy.count(), 1);

    // Setting individual radii should override their values.
    rectangleShape->setTopLeftRadius(2);
    QCOMPARE(rectangleShape->topLeftRadius(), 2);
    QCOMPARE(topLeftRadiusSignalSpy.count(), 2);

    rectangleShape->setTopRightRadius(3);
    QCOMPARE(rectangleShape->topRightRadius(), 3);
    QCOMPARE(topRightRadiusSignalSpy.count(), 2);

    rectangleShape->setBottomLeftRadius(4);
    QCOMPARE(rectangleShape->bottomLeftRadius(), 4);
    QCOMPARE(bottomLeftRadiusSignalSpy.count(), 2);

    rectangleShape->setBottomRightRadius(5);
    QCOMPARE(rectangleShape->bottomRightRadius(), 5);
    QCOMPARE(bottomRightRadiusSignalSpy.count(), 2);

    // Setting radius should no longer have any effect on the other radii properties.
    rectangleShape->setRadius(100);
    QCOMPARE(rectangleShape->radius(), 100);
    QCOMPARE(radiusSignalSpy.count(), 2);
    QCOMPARE(rectangleShape->topLeftRadius(), 2);
    QCOMPARE(topLeftRadiusSignalSpy.count(), 2);
    QCOMPARE(rectangleShape->topRightRadius(), 3);
    QCOMPARE(topRightRadiusSignalSpy.count(), 2);
    QCOMPARE(rectangleShape->bottomLeftRadius(), 4);
    QCOMPARE(bottomLeftRadiusSignalSpy.count(), 2);
    QCOMPARE(rectangleShape->bottomRightRadius(), 5);
    QCOMPARE(bottomRightRadiusSignalSpy.count(), 2);

    // Neither should resetting it.
    rectangleShape->resetRadius();
    QCOMPARE(rectangleShape->radius(), 10);
    QCOMPARE(radiusSignalSpy.count(), 3);
    QCOMPARE(rectangleShape->topLeftRadius(), 2);
    QCOMPARE(topLeftRadiusSignalSpy.count(), 2);
    QCOMPARE(rectangleShape->topRightRadius(), 3);
    QCOMPARE(topRightRadiusSignalSpy.count(), 2);
    QCOMPARE(rectangleShape->bottomLeftRadius(), 4);
    QCOMPARE(bottomLeftRadiusSignalSpy.count(), 2);
    QCOMPARE(rectangleShape->bottomRightRadius(), 5);
    QCOMPARE(bottomRightRadiusSignalSpy.count(), 2);

    // Resetting individual radii should return their values to that of radius.
    rectangleShape->resetTopLeftRadius();
    QCOMPARE(rectangleShape->topLeftRadius(), 10);
    QCOMPARE(topLeftRadiusSignalSpy.count(), 3);

    rectangleShape->resetTopRightRadius();
    QCOMPARE(rectangleShape->topRightRadius(), 10);
    QCOMPARE(topRightRadiusSignalSpy.count(), 3);

    rectangleShape->resetBottomLeftRadius();
    QCOMPARE(rectangleShape->bottomLeftRadius(), 10);
    QCOMPARE(bottomLeftRadiusSignalSpy.count(), 3);

    rectangleShape->resetBottomRightRadius();
    QCOMPARE(rectangleShape->bottomRightRadius(), 10);
    QCOMPARE(bottomRightRadiusSignalSpy.count(), 3);
}

void tst_QQuickRectangleShape::bevel()
{
    QQuickApplicationHelper helper(this, "default.qml");
    QVERIFY2(helper.ready, helper.failureMessage());
    QQuickWindow *window = helper.window;
    window->show();
    QVERIFY(QTest::qWaitForWindowExposed(window));

    auto *rectangleShape = window->property("rectangleShape").value<QQuickRectangleShape *>();
    QVERIFY(rectangleShape);

    const QSignalSpy bevelSignalSpy(rectangleShape, &QQuickRectangleShape::bevelChanged);
    QVERIFY(bevelSignalSpy.isValid());
    const QSignalSpy topLeftBevelSignalSpy(rectangleShape, &QQuickRectangleShape::topLeftBevelChanged);
    QVERIFY(topLeftBevelSignalSpy.isValid());
    const QSignalSpy topRightBevelSignalSpy(rectangleShape, &QQuickRectangleShape::topRightBevelChanged);
    QVERIFY(topRightBevelSignalSpy.isValid());
    const QSignalSpy bottomLeftBevelSignalSpy(rectangleShape, &QQuickRectangleShape::bottomLeftBevelChanged);
    QVERIFY(bottomLeftBevelSignalSpy.isValid());
    const QSignalSpy bottomRightBevelSignalSpy(rectangleShape, &QQuickRectangleShape::bottomRightBevelChanged);
    QVERIFY(bottomRightBevelSignalSpy.isValid());

    // Setting bevel should affect all of the other bevel properties that aren't explicitly set.
    rectangleShape->setBevel(true);
    QCOMPARE(rectangleShape->hasBevel(), true);
    QCOMPARE(bevelSignalSpy.count(), 1);
    QCOMPARE(rectangleShape->hasTopLeftBevel(), true);
    QCOMPARE(topLeftBevelSignalSpy.count(), 1);
    QCOMPARE(rectangleShape->hasTopRightBevel(), true);
    QCOMPARE(topRightBevelSignalSpy.count(), 1);
    QCOMPARE(rectangleShape->hasBottomLeftBevel(), true);
    QCOMPARE(bottomLeftBevelSignalSpy.count(), 1);
    QCOMPARE(rectangleShape->hasBottomRightBevel(), true);
    QCOMPARE(bottomRightBevelSignalSpy.count(), 1);

    // Setting individual bevels should override their values.
    rectangleShape->setTopLeftBevel(false);
    QCOMPARE(rectangleShape->hasTopLeftBevel(), false);
    QCOMPARE(topLeftBevelSignalSpy.count(), 2);

    rectangleShape->setTopRightBevel(false);
    QCOMPARE(rectangleShape->hasTopRightBevel(), false);
    QCOMPARE(topRightBevelSignalSpy.count(), 2);

    rectangleShape->setBottomLeftBevel(false);
    QCOMPARE(rectangleShape->hasBottomLeftBevel(), false);
    QCOMPARE(bottomLeftBevelSignalSpy.count(), 2);

    rectangleShape->setBottomRightBevel(false);
    QCOMPARE(rectangleShape->hasBottomRightBevel(), false);
    QCOMPARE(bottomRightBevelSignalSpy.count(), 2);

    // Setting bevel should no longer have any effect on the other bevel properties.
    // (we only have two possible values to change between, and since the individual
    // properties are already false, we need to change its value twice to properly test it).
    rectangleShape->setBevel(false);
    rectangleShape->setBevel(true);
    QCOMPARE(rectangleShape->hasBevel(), true);
    QCOMPARE(bevelSignalSpy.count(), 3);
    QCOMPARE(rectangleShape->hasTopLeftBevel(), false);
    QCOMPARE(topLeftBevelSignalSpy.count(), 2);
    QCOMPARE(rectangleShape->hasTopRightBevel(), false);
    QCOMPARE(topRightBevelSignalSpy.count(), 2);
    QCOMPARE(rectangleShape->hasBottomLeftBevel(), false);
    QCOMPARE(bottomLeftBevelSignalSpy.count(), 2);
    QCOMPARE(rectangleShape->hasBottomRightBevel(), false);
    QCOMPARE(bottomRightBevelSignalSpy.count(), 2);

    // Neither should resetting it.
    rectangleShape->resetBevel();
    QCOMPARE(rectangleShape->hasBevel(), false);
    QCOMPARE(bevelSignalSpy.count(), 4);
    // (These were false to begin with, but we test it anyway; at least the signal count
    // will help to catch regressions)
    QCOMPARE(rectangleShape->hasTopLeftBevel(), false);
    QCOMPARE(topLeftBevelSignalSpy.count(), 2);
    QCOMPARE(rectangleShape->hasTopRightBevel(), false);
    QCOMPARE(topRightBevelSignalSpy.count(), 2);
    QCOMPARE(rectangleShape->hasBottomLeftBevel(), false);
    QCOMPARE(bottomLeftBevelSignalSpy.count(), 2);
    QCOMPARE(rectangleShape->hasBottomRightBevel(), false);
    QCOMPARE(bottomRightBevelSignalSpy.count(), 2);

    // Resetting individual bevels should return their values to that of bevel.
    // First, set bevel to true so we have a state change that we can observe.
    rectangleShape->setBevel(true);
    QCOMPARE(bevelSignalSpy.count(), 5);

    rectangleShape->resetTopLeftBevel();
    QCOMPARE(rectangleShape->hasTopLeftBevel(), true);
    QCOMPARE(topLeftBevelSignalSpy.count(), 3);

    rectangleShape->resetTopRightBevel();
    QCOMPARE(rectangleShape->hasTopRightBevel(), true);
    QCOMPARE(topRightBevelSignalSpy.count(), 3);

    rectangleShape->resetBottomLeftBevel();
    QCOMPARE(rectangleShape->hasBottomLeftBevel(), true);
    QCOMPARE(bottomLeftBevelSignalSpy.count(), 3);

    rectangleShape->resetBottomRightBevel();
    QCOMPARE(rectangleShape->hasBottomRightBevel(), true);
    QCOMPARE(bottomRightBevelSignalSpy.count(), 3);
}

QTEST_MAIN(tst_QQuickRectangleShape)

#include "tst_qquickrectangleshape.moc"
