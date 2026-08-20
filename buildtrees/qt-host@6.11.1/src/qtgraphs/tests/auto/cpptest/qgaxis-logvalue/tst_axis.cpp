// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtTest/QtTest>

#include <QtGraphs/QLogValue3DAxisFormatter>

class tst_axis: public QObject
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
    QLogValue3DAxisFormatter *m_formatter;
};

void tst_axis::initTestCase()
{
}

void tst_axis::cleanupTestCase()
{
}

void tst_axis::init()
{
    m_formatter = new QLogValue3DAxisFormatter();
}

void tst_axis::cleanup()
{
    delete m_formatter;
}

void tst_axis::construct()
{
    QLogValue3DAxisFormatter *formatter = new QLogValue3DAxisFormatter();
    QVERIFY(formatter);
    delete formatter;
}

void tst_axis::initialProperties()
{
    QVERIFY(m_formatter);

    QCOMPARE(m_formatter->autoSubGrid(), true);
    QCOMPARE(m_formatter->base(), 10.0);
    QCOMPARE(m_formatter->edgeLabelsVisible(), true);
}

void tst_axis::initializeProperties()
{
    QVERIFY(m_formatter);

    QSignalSpy baseSpy(m_formatter, &QLogValue3DAxisFormatter::baseChanged);
    QSignalSpy subGridSpy(m_formatter, &QLogValue3DAxisFormatter::autoSubGridChanged);
    QSignalSpy edgeLabelVisibleSpy(m_formatter, &QLogValue3DAxisFormatter::edgeLabelsVisibleChanged);

    m_formatter->setAutoSubGrid(false);
    m_formatter->setBase(0.1);
    m_formatter->setEdgeLabelsVisible(false);

    QCOMPARE(m_formatter->autoSubGrid(), false);
    QCOMPARE(m_formatter->base(), 0.1);
    QCOMPARE(m_formatter->edgeLabelsVisible(), false);

    QCOMPARE(baseSpy.size(), 1);
    QCOMPARE(subGridSpy.size(), 1);
    QCOMPARE(edgeLabelVisibleSpy.size(), 1);
}

void tst_axis::invalidProperties()
{
    m_formatter->setBase(-1.0);
    QCOMPARE(m_formatter->base(), 10.0);

    m_formatter->setBase(1.0);
    QCOMPARE(m_formatter->base(), 10.0);
}

QTEST_MAIN(tst_axis)
#include "tst_axis.moc"
