// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtTest/QtTest>

#include <QtGraphs/QScatter3DSeries>

#include <QtGui/qquaternion.h>

class tst_series: public QObject
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

private:
    QScatter3DSeries *m_series;
};

void tst_series::initTestCase()
{
}

void tst_series::cleanupTestCase()
{
}

void tst_series::init()
{
    m_series = new QScatter3DSeries();
}

void tst_series::cleanup()
{
    delete m_series;
}

void tst_series::construct()
{
    QScatter3DSeries *series = new QScatter3DSeries();
    QVERIFY(series);
    delete series;

    QScatterDataProxy *proxy = new QScatterDataProxy();

    series = new QScatter3DSeries(proxy);
    QVERIFY(series);
    QCOMPARE(series->dataProxy(), proxy);
    delete series;
}

void tst_series::initialProperties()
{
    QVERIFY(m_series);

    QVERIFY(m_series->dataProxy());
    QCOMPARE(m_series->itemSize(), 0.0f);
    QCOMPARE(m_series->selectedItem(), m_series->invalidSelectionIndex());

    // Common properties. The ones identical between different series are tested in QBar3DSeries tests
    QCOMPARE(m_series->itemLabelFormat(), QString("@xLabel, @yLabel, @zLabel"));
    QCOMPARE(m_series->mesh(), QAbstract3DSeries::Mesh::Sphere);
    QCOMPARE(m_series->type(), QAbstract3DSeries::SeriesType::Scatter);
}

void tst_series::initializeProperties()
{
    QVERIFY(m_series);

    QSignalSpy dataProxySpy(m_series, &QScatter3DSeries::dataProxyChanged);
    QSignalSpy selectedItemSpy(m_series, &QScatter3DSeries::selectedItemChanged);
    QSignalSpy itemSizeSpy(m_series, &QScatter3DSeries::itemSizeChanged);
    QSignalSpy dataArraySpy(m_series, &QScatter3DSeries::dataArrayChanged);

    m_series->setDataProxy(new QScatterDataProxy());
    m_series->setItemSize(0.5f);
    m_series->setSelectedItem(0);

    QCOMPARE(m_series->itemSize(), 0.5f);
    QCOMPARE(m_series->selectedItem(), 0);

    QCOMPARE(dataProxySpy.size(), 1);
    QCOMPARE(itemSizeSpy.size(), 1);
    QCOMPARE(selectedItemSpy.size(), 1);

    QScatterDataArray data;
    data << QScatterDataItem(0.5f, 0.5f, 0.5f) << QScatterDataItem(1.0f, 1.0f, 1.0f);
    m_series->setDataArray(data);

    QCOMPARE(dataArraySpy.size(), 1);

    // Common properties. The ones identical between different series are tested in QBar3DSeries tests
    m_series->setMesh(QAbstract3DSeries::Mesh::Point);
    m_series->setMeshRotation(QQuaternion(1, 1, 10, 20));

    QCOMPARE(m_series->mesh(), QAbstract3DSeries::Mesh::Point);
    QCOMPARE(m_series->meshRotation(), QQuaternion(1, 1, 10, 20));
}

QTEST_MAIN(tst_series)
#include "tst_series.moc"
