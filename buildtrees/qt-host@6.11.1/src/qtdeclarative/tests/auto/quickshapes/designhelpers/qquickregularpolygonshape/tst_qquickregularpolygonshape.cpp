// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtTest/QtTest>
#include <QtQuick/qquickview.h>
#include <QtQuickTest/quicktest.h>
#include <QtQuickTestUtils/private/viewtestutils_p.h>
#include <QtQuickTestUtils/private/visualtestutils_p.h>
#include <QtQuickShapesDesignHelpers/private/qquickregularpolygonshape_p.h>

class tst_QQuickRegularPolygonShape : public QQmlDataTest
{
    Q_OBJECT
public:
    tst_QQuickRegularPolygonShape();

private slots:
    void initTestCase() override;
    void basicShape();
    void changeSignals();
    void changeSignals_data();

private:
    QScopedPointer<QQuickView> window;
};

tst_QQuickRegularPolygonShape::tst_QQuickRegularPolygonShape()
    : QQmlDataTest(QT_QMLTEST_DATADIR) { }

void tst_QQuickRegularPolygonShape::initTestCase()
{
    QQmlDataTest::initTestCase();
    window.reset(QQuickViewTestUtils::createView());
}

void tst_QQuickRegularPolygonShape::basicShape()
{
    QQmlEngine engine;
    QQmlComponent c(&engine, testFileUrl("regularpolygonshape.qml"));
    QQuickRegularPolygonShape *shape = qobject_cast<QQuickRegularPolygonShape *>(c.create());
    QVERIFY(shape);

    QCOMPARE(shape->width(), 200);
    QCOMPARE(shape->height(), 200);
    QCOMPARE(shape->cornerRadius(), 10);
    QCOMPARE(shape->sideCount(), 6);
    QCOMPARE(shape->strokeWidth(), 1);
    QCOMPARE(shape->dashOffset(), 0);
    QCOMPARE(shape->capStyle(), QQuickShapePath::SquareCap);
    QCOMPARE(shape->joinStyle(), QQuickShapePath::BevelJoin);
    QCOMPARE(shape->strokeStyle(), QQuickShapePath::SolidLine);
    QCOMPARE(shape->fillColor(), QColor(Qt::white));
    QCOMPARE(shape->strokeColor(), QColor(Qt::black));
}

void tst_QQuickRegularPolygonShape::changeSignals_data()
{
    QTest::addColumn<QVariant>("propertyValue");
    QTest::addColumn<QMetaMethod>("changeSignal");

    QTest::newRow("cornerRadius") << QVariant::fromValue(5.0f)
                            << QMetaMethod::fromSignal(&QQuickRegularPolygonShape::cornerRadiusChanged);
    QTest::newRow("sideCount") << QVariant::fromValue(
            7) << QMetaMethod::fromSignal(&QQuickRegularPolygonShape::sideCountChanged);
    QTest::newRow("strokeColor") << QVariant::fromValue(QColor(
            Qt::blue)) << QMetaMethod::fromSignal(&QQuickRegularPolygonShape::strokeColorChanged);
    QTest::newRow("strokeWidth") << QVariant::fromValue(
            0) << QMetaMethod::fromSignal(&QQuickRegularPolygonShape::strokeWidthChanged);
    QTest::newRow("fillColor") << QVariant::fromValue(QColor(
            Qt::blue)) << QMetaMethod::fromSignal(&QQuickRegularPolygonShape::fillColorChanged);
    QTest::newRow("joinStyle") << QVariant::fromValue(QQuickShapePath::RoundJoin)
                               << QMetaMethod::fromSignal(
                                          &QQuickRegularPolygonShape::joinStyleChanged);
    QTest::newRow("capStyle") << QVariant::fromValue(QQuickShapePath::RoundCap)
                              << QMetaMethod::fromSignal(
                                         &QQuickRegularPolygonShape::capStyleChanged);
    QTest::newRow("strokeStyle") << QVariant::fromValue(QQuickShapePath::DashLine)
                                 << QMetaMethod::fromSignal(
                                            &QQuickRegularPolygonShape::strokeStyleChanged);
    QTest::newRow("dashOffset") << QVariant::fromValue(
            4) << QMetaMethod::fromSignal(&QQuickRegularPolygonShape::dashOffsetChanged);
    QTest::newRow("dashPattern") << QVariant::fromValue(QList<qreal>{
            1, 2 }) << QMetaMethod::fromSignal(&QQuickRegularPolygonShape::dashPatternChanged);
}

void tst_QQuickRegularPolygonShape::changeSignals()
{
    window->setSource(testFileUrl("default.qml"));
    window->show();
    QVERIFY(QTest::qWaitForWindowExposed(window.get()));
    QVERIFY(window->status() == QQuickView::Ready);

    QFETCH(const QVariant, propertyValue);
    QFETCH(const QMetaMethod, changeSignal);

    QQuickRegularPolygonShape *shape = QQuickVisualTestUtils::findItem<QQuickRegularPolygonShape>(
            window->rootObject(), "shape");
    QVERIFY(shape);
    const QSignalSpy signalSpy(shape, changeSignal);
    QVERIFY(signalSpy.isValid());
    QVERIFY(shape->setProperty(QTest::currentDataTag(), propertyValue));
    QCOMPARE(signalSpy.count(), 1);
}

QTEST_MAIN(tst_QQuickRegularPolygonShape)

#include "tst_qquickregularpolygonshape.moc"
