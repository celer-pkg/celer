// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtTest/QtTest>

#include <QtGraphs/QSurface3DSeries>
#include <QtGraphs/QSurfaceDataProxy>

class tst_proxy: public QObject
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
    void initialRow();

private:
    QSurfaceDataProxy *m_proxy;
    QSurface3DSeries *m_series;
};

void tst_proxy::initTestCase()
{
}

void tst_proxy::cleanupTestCase()
{
}

void tst_proxy::init()
{
    m_proxy = new QSurfaceDataProxy();
    m_series = new QSurface3DSeries(m_proxy);
}

void tst_proxy::cleanup()
{
    delete m_series;
    m_proxy = 0; // proxy gets deleted with series
}

void tst_proxy::construct()
{
    QSurfaceDataProxy *proxy = new QSurfaceDataProxy();
    QVERIFY(proxy);
    QSurface3DSeries *series = new QSurface3DSeries(proxy);
    QVERIFY(series);
    delete series;
    proxy = 0; // proxy gets deleted with series
}

void tst_proxy::initialProperties()
{
    QVERIFY(m_proxy);
    QVERIFY(m_series);

    QCOMPARE(m_proxy->columnCount(), 0);
    QCOMPARE(m_proxy->rowCount(), 0);

    QCOMPARE(m_proxy->type(), QAbstractDataProxy::DataType::Surface);
}

void tst_proxy::initializeProperties()
{
    QVERIFY(m_proxy);
    QVERIFY(m_series);

    QSignalSpy rowCountSpy(m_proxy, &QSurfaceDataProxy::rowCountChanged);
    QSignalSpy columnCountSpy(m_proxy, &QSurfaceDataProxy::columnCountChanged);
    QSignalSpy seriesSpy(m_proxy, &QSurfaceDataProxy::seriesChanged);

    QSignalSpy rowsAddedSpy(m_proxy, &QSurfaceDataProxy::rowsAdded);
    QSignalSpy rowsChangedSpy(m_proxy, &QSurfaceDataProxy::rowsChanged);
    QSignalSpy rowsRemovedSpy(m_proxy, &QSurfaceDataProxy::rowsRemoved);
    QSignalSpy rowsInsertedSpy(m_proxy, &QSurfaceDataProxy::rowsInserted);
    QSignalSpy itemChangedSpy(m_proxy, &QSurfaceDataProxy::itemChanged);
    QSignalSpy arrayResetSpy(m_proxy, &QSurfaceDataProxy::arrayReset);

    QSurfaceDataArray data;
    QSurfaceDataRow dataRow1;
    QSurfaceDataRow dataRow2;
    dataRow1 << QSurfaceDataItem(0.0f, 0.1f, 0.5f) << QSurfaceDataItem(1.0f, 0.5f, 0.5f);
    dataRow2 << QSurfaceDataItem(0.0f, 1.8f, 1.0f) << QSurfaceDataItem(1.0f, 1.2f, 1.0f);
    data << dataRow1 << dataRow2;

    m_proxy->resetArray(data);

    QCOMPARE(arrayResetSpy.size(), 1);
    QCOMPARE(rowCountSpy.size(), 1);
    QCOMPARE(columnCountSpy.size(), 1);

    QCOMPARE(m_proxy->columnCount(), 2);
    QCOMPARE(m_proxy->rowCount(), 2);

    QSurfaceDataRow datarow;
    datarow << QSurfaceDataItem(0.5f, 0.5f, 0.5f) << QSurfaceDataItem(1.0f, 1.0f, 1.0f);

    auto index = m_proxy->addRow(datarow);

    QCOMPARE(rowsAddedSpy.size(), 1);
    QCOMPARE(rowCountSpy.size(), 2);

    m_proxy->removeRows(index, 1);

    QCOMPARE(rowsRemovedSpy.size(), 1);
    QCOMPARE(rowCountSpy.size(), 3);

    QSurfaceDataRow insertRow;
    insertRow << QSurfaceDataItem(0.25f, 0.25f, 0.25f) << QSurfaceDataItem(1.0f, 0.8f, 0.25f);
    m_proxy->insertRow(1, insertRow);

    QCOMPARE(rowsInsertedSpy.size(), 1);
    QCOMPARE(rowCountSpy.size(), 4);
}

void tst_proxy::initialRow()
{
    QSurfaceDataProxy proxy;
    QSurface3DSeries *series = new QSurface3DSeries(&proxy);
    Q_UNUSED(series);

    QSurfaceDataRow row{QSurfaceDataItem{QVector3D{0, 0, 0}}, QSurfaceDataItem{QVector3D{1, 1, 1}}};
    proxy.addRow(QSurfaceDataRow(row));
    proxy.addRow(QSurfaceDataRow(row));
}

QTEST_MAIN(tst_proxy)
#include "tst_proxy.moc"
