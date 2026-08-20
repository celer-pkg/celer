// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qtestcase.h"
#include <QtGraphs/QValueAxis>
#include <QtGraphs/private/qgraphsview_p.h>

#include <QtTest/QtTest>

class tst_valueaxis : public QObject
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
    void addAndDelete();

private:
    QValueAxis *m_axis;
};

void tst_valueaxis::initTestCase() {}

void tst_valueaxis::cleanupTestCase() {}

void tst_valueaxis::init()
{
    m_axis = new QValueAxis();
}

void tst_valueaxis::cleanup()
{
    delete m_axis;
}

void tst_valueaxis::construct()
{
    QValueAxis *axis = new QValueAxis();
    QVERIFY(axis);
    delete axis;
}

void tst_valueaxis::initialProperties()
{
    QVERIFY(m_axis);

    QCOMPARE(m_axis->min(), 0);
    QCOMPARE(m_axis->max(), 10);
    QCOMPARE(m_axis->labelFormat(), "");
    QCOMPARE(m_axis->labelDecimals(), -1);
    QCOMPARE(m_axis->subTickCount(), 0);
    QCOMPARE(m_axis->tickAnchor(), 0.0);
    QCOMPARE(m_axis->tickInterval(), 0.0);
    QCOMPARE(m_axis->zoom(), 1.0);
    QCOMPARE(m_axis->pan(), 0.0);
}

void tst_valueaxis::initializeProperties()
{
    QVERIFY(m_axis);

    QSignalSpy spy0(m_axis, &QValueAxis::minChanged);
    QSignalSpy spy1(m_axis, &QValueAxis::maxChanged);
    QSignalSpy spy2(m_axis, &QValueAxis::labelFormatChanged);
    QSignalSpy spy3(m_axis, &QValueAxis::labelDecimalsChanged);
    QSignalSpy spy4(m_axis, &QValueAxis::subTickCountChanged);
    QSignalSpy spy5(m_axis, &QValueAxis::tickAnchorChanged);
    QSignalSpy spy6(m_axis, &QValueAxis::tickIntervalChanged);

    m_axis->setMin(5);
    m_axis->setMax(100);
    m_axis->setLabelFormat("d");
    m_axis->setLabelDecimals(2);
    m_axis->setSubTickCount(2);
    m_axis->setTickAnchor(0.5);
    m_axis->setTickInterval(0.5);
    m_axis->setZoom(2.0);
    m_axis->setPan(1.0);

    QCOMPARE(m_axis->min(), 5);
    QCOMPARE(m_axis->max(), 100);
    QCOMPARE(m_axis->labelFormat(), "d");
    QCOMPARE(m_axis->labelDecimals(), 2);
    QCOMPARE(m_axis->subTickCount(), 2);
    QCOMPARE(m_axis->tickAnchor(), 0.5);
    QCOMPARE(m_axis->tickInterval(), 0.5);
    QCOMPARE(m_axis->zoom(), 2.0);
    QCOMPARE(m_axis->pan(), 1.0);

    m_axis->setLabelFormat("%.2f cakes");
    QCOMPARE(m_axis->labelFormat(), "%.2f cakes");

    //Constuct a string same way we do in axisRenderer.
    QByteArray format = m_axis->labelFormat().toLatin1();
    QString formatTest = QString::asprintf(format.constData(), m_axis->min());

    QCOMPARE(formatTest, "5.00 cakes");

    QCOMPARE(spy0.size(), 1);
    QCOMPARE(spy1.size(), 1);
    QCOMPARE(spy2.size(), 2);
    QCOMPARE(spy3.size(), 1);
    QCOMPARE(spy4.size(), 1);
    QCOMPARE(spy5.size(), 1);
    QCOMPARE(spy6.size(), 1);
}

void tst_valueaxis::invalidProperties()
{
    QVERIFY(m_axis);

    m_axis->setMin(100);
    m_axis->setMax(0);
    m_axis->setSubTickCount(-1);

    QCOMPARE(m_axis->min(), 0);
    QCOMPARE(m_axis->max(), 0);
    QCOMPARE(m_axis->subTickCount(), 0);
}

void tst_valueaxis::addAndDelete()
{
    QValueAxis *xAxis = new QValueAxis();
    QValueAxis *yAxis = new QValueAxis();
    QGraphsView view;
    view.setAxisX(xAxis);
    view.setAxisY(yAxis);
    QVERIFY(view.axisX());
    QVERIFY(view.axisY());
    // Axis destructors should remove them from the GraphsView
    delete xAxis;
    QVERIFY(!view.axisX());
    QVERIFY(view.axisY());
    delete yAxis;
    QVERIFY(!view.axisX());
    QVERIFY(!view.axisY());
}

QTEST_MAIN(tst_valueaxis)
#include "tst_valueaxis.moc"
