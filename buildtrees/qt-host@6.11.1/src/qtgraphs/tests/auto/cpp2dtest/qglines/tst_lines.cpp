// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtGraphs/QLineSeries>
#include <QtGraphs/QValueAxis>
#include <QtQml/QQmlComponent>
#include <QtTest/QtTest>

class tst_lines : public QObject
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

private:
    QLineSeries *m_series;
};

void tst_lines::initTestCase() {}

void tst_lines::cleanupTestCase() {}

void tst_lines::init()
{
    m_series = new QLineSeries();
}

void tst_lines::cleanup()
{
    delete m_series;
}

void tst_lines::construct()
{
    QLineSeries *series = new QLineSeries();
    QVERIFY(series);
    delete series;
}

void tst_lines::initialProperties()
{
    QVERIFY(m_series);

    // Properties from QLineSeries
    QCOMPARE(m_series->width(), 2.0);
    QCOMPARE(m_series->capStyle(), Qt::PenCapStyle::SquareCap);
    QCOMPARE(m_series->pointDelegate(), nullptr);
    QCOMPARE(m_series->joinStyle(), Qt::PenJoinStyle::BevelJoin);
    QCOMPARE(m_series->lineStyle(), QLineSeries::LineStyle::Straight);
    QCOMPARE(m_series->strokeStyle(), QLineSeries::StrokeStyle::SolidLine);

    // Properties from QXYSeries
    QCOMPARE(m_series->color(), QColor(Qt::transparent));
    QCOMPARE(m_series->selectedColor(), QColor(Qt::transparent));
    QCOMPARE(m_series->isDraggable(), false);

    // Properties from QAbstractSeries
    QCOMPARE(m_series->name(), "");
    QCOMPARE(m_series->isVisible(), true);
    QCOMPARE(m_series->isSelectable(), false);
    QCOMPARE(m_series->isHoverable(), false);
    QCOMPARE(m_series->opacity(), 1.0);
    QCOMPARE(m_series->valuesMultiplier(), 1.0);
}

void tst_lines::initializeProperties()
{
    QVERIFY(m_series);

    QSignalSpy spy0(m_series, &QLineSeries::widthChanged);
    QSignalSpy spy1(m_series, &QLineSeries::capStyleChanged);
    QSignalSpy spy2(m_series, &QLineSeries::pointDelegateChanged);

    QSignalSpy spy3(m_series, &QLineSeries::colorChanged);
    QSignalSpy spy4(m_series, &QLineSeries::selectedColorChanged);
    QSignalSpy spy5(m_series, &QLineSeries::draggableChanged);

    QSignalSpy spy6(m_series, &QLineSeries::nameChanged);
    QSignalSpy spy7(m_series, &QLineSeries::visibleChanged);
    QSignalSpy spy8(m_series, &QLineSeries::selectableChanged);
    QSignalSpy spy9(m_series, &QLineSeries::hoverableChanged);
    QSignalSpy spy10(m_series, &QLineSeries::opacityChanged);
    QSignalSpy spy11(m_series, &QLineSeries::valuesMultiplierChanged);

    QSignalSpy spy12(m_series, &QLineSeries::joinStyleChanged);
    QSignalSpy spy13(m_series, &QLineSeries::lineStyleChanged);
    QSignalSpy spy14(m_series, &QLineSeries::strokeStyleChanged);
    QSignalSpy spy15(m_series, &QLineSeries::dashOffsetChanged);
    QSignalSpy spy16(m_series, &QLineSeries::dashPatternChanged);

    auto marker = new QQmlComponent(this);

    m_series->setWidth(5.0);
    m_series->setCapStyle(Qt::PenCapStyle::RoundCap);
    m_series->setPointDelegate(marker);

    m_series->setColor("#ff0000");
    m_series->setSelectedColor("#0000ff");
    m_series->setDraggable(true);

    m_series->setName("LineSeries");
    m_series->setVisible(false);
    m_series->setSelectable(true);
    m_series->setHoverable(true);
    m_series->setOpacity(0.5);
    m_series->setValuesMultiplier(0.5);

    m_series->setJoinStyle(Qt::PenJoinStyle::RoundJoin);
    m_series->setLineStyle(QLineSeries::LineStyle::StepLeft);
    m_series->setStrokeStyle(QLineSeries::StrokeStyle::DashLine);
    m_series->setDashOffset(1.0);
    m_series->setDashPattern({3, 2});

    QCOMPARE(m_series->width(), 5.0);
    QCOMPARE(m_series->capStyle(), Qt::PenCapStyle::RoundCap);
    QCOMPARE(m_series->pointDelegate(), marker);

    QCOMPARE(m_series->color(), "#ff0000");
    QCOMPARE(m_series->selectedColor(), "#0000ff");
    QCOMPARE(m_series->isDraggable(), true);

    QCOMPARE(m_series->name(), "LineSeries");
    QCOMPARE(m_series->isVisible(), false);
    QCOMPARE(m_series->isSelectable(), true);
    QCOMPARE(m_series->isHoverable(), true);
    QCOMPARE(m_series->opacity(), 0.5);
    QCOMPARE(m_series->valuesMultiplier(), 0.5);

    QCOMPARE(m_series->joinStyle(), Qt::PenJoinStyle::RoundJoin);
    QCOMPARE(m_series->lineStyle(), QLineSeries::LineStyle::StepLeft);
    QCOMPARE(m_series->strokeStyle(), QLineSeries::StrokeStyle::DashLine);
    QCOMPARE(m_series->dashOffset(), 1.0);
    QCOMPARE(m_series->dashPattern(), QVector<qreal>({3, 2}));

    QCOMPARE(spy0.size(), 1);
    QCOMPARE(spy1.size(), 1);
    QCOMPARE(spy2.size(), 1);

    QCOMPARE(spy3.size(), 1);
    QCOMPARE(spy4.size(), 1);
    QCOMPARE(spy5.size(), 1);

    QCOMPARE(spy6.size(), 1);
    QCOMPARE(spy7.size(), 1);
    QCOMPARE(spy8.size(), 1);
    QCOMPARE(spy9.size(), 1);
    QCOMPARE(spy10.size(), 1);
    QCOMPARE(spy11.size(), 1);

    QCOMPARE(spy12.size(), 1);
    QCOMPARE(spy13.size(), 1);
    QCOMPARE(spy14.size(), 1);
    QCOMPARE(spy15.size(), 1);
    QCOMPARE(spy16.size(), 1);
}

void tst_lines::invalidProperties()
{
    QVERIFY(m_series);

    m_series->setWidth(-10.0);
    m_series->setValuesMultiplier(2.0); // range 0...1

    QCOMPARE(m_series->width(), 0.0);
    QCOMPARE(m_series->valuesMultiplier(), 1.0);

    m_series->setValuesMultiplier(-1.0); // range 0...1
    QCOMPARE(m_series->valuesMultiplier(), 0.0);
}

QTEST_MAIN(tst_lines)
#include "tst_lines.moc"
