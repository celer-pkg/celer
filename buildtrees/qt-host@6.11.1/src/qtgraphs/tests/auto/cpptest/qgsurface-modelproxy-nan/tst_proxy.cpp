// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtTest/QtTest>

#include <QtGraphs/QItemModelSurfaceDataProxy>
#include <QtGraphsWidgets/q3dsurfacewidgetitem.h>

#include "cpptestutil.h"

class tst_proxy: public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    void dataContainingNaNFirstRow();
    void dataContainingNaNLastRow();
    void dataContainingNaNFirstLastRow();
};

void tst_proxy::initTestCase()
{
}

void tst_proxy::cleanupTestCase()
{
}

void tst_proxy::init()
{
}

void tst_proxy::cleanup()
{
}

void tst_proxy::dataContainingNaNFirstRow()
{
    if (!CpptestUtil::isOpenGLSupported())
        QSKIP("OpenGL not supported on this platform");

    const int size = 10;
    const int missingRow = 0;

    QItemModelSurfaceDataProxy *proxy  = new QItemModelSurfaceDataProxy();
    QSurface3DSeries *series = new QSurface3DSeries(proxy);
    Q3DSurfaceWidgetItem *graph = new Q3DSurfaceWidgetItem();
    QQuickWidget quickWidget;
    graph->setWidget(&quickWidget);

    graph->addSeries(series);

    // X
    QSurfaceDataArray array;
    array.reserve(size);
    for (int i = 0; i < size; i++) {
        QSurfaceDataRow row(size);
        for (int j = 0; j < size; j++) {
            row[j].setPosition(QVector3D((i == missingRow) ? std::numeric_limits<float>::quiet_NaN()
                                                              : static_cast<float>(i),
                                            qSin(static_cast<float>(i)), static_cast<float>(j)));
        }
        array << row;
    }
    proxy->resetArray(array);
    QVERIFY(!qIsNaN(graph->axisX()->min()));
    QVERIFY(!qIsNaN(graph->axisX()->max()));
    QVERIFY(!qIsNaN(graph->axisY()->min()));
    QVERIFY(!qIsNaN(graph->axisY()->max()));
    QVERIFY(!qIsNaN(graph->axisZ()->min()));
    QVERIFY(!qIsNaN(graph->axisZ()->max()));

    // Y
    for (int i = 0; i < size; i++) {
        QSurfaceDataRow row(size);
        for (int j = 0; j < size; j++) {
            row[j].setPosition(QVector3D(static_cast<float>(i),
                                            (i == missingRow) ? std::numeric_limits<float>::quiet_NaN()
                                                              : qSin(static_cast<float>(i)),
                                             static_cast<float>(j)));
        }
        array << row;
    }
    proxy->resetArray(array);
    QVERIFY(!qIsNaN(graph->axisX()->min()));
    QVERIFY(!qIsNaN(graph->axisX()->max()));
    QVERIFY(!qIsNaN(graph->axisY()->min()));
    QVERIFY(!qIsNaN(graph->axisY()->max()));
    QVERIFY(!qIsNaN(graph->axisZ()->min()));
    QVERIFY(!qIsNaN(graph->axisZ()->max()));

    // Z
    for (int i = 0; i < size; i++) {
        QSurfaceDataRow row(size);
        for (int j = 0; j < size; j++) {
            row[j].setPosition(QVector3D(static_cast<float>(i),
                                            qSin(static_cast<float>(i)),
                                            (i == missingRow) ? std::numeric_limits<float>::quiet_NaN()
                                                              : static_cast<float>(j)));
        }
        array << row;
    }
    proxy->resetArray(array);
    QVERIFY(!qIsNaN(graph->axisX()->min()));
    QVERIFY(!qIsNaN(graph->axisX()->max()));
    QVERIFY(!qIsNaN(graph->axisY()->min()));
    QVERIFY(!qIsNaN(graph->axisY()->max()));
    QVERIFY(!qIsNaN(graph->axisZ()->min()));
    QVERIFY(!qIsNaN(graph->axisZ()->max()));

    delete graph;

}

void tst_proxy::dataContainingNaNLastRow()
{
    if (!CpptestUtil::isOpenGLSupported())
        QSKIP("OpenGL not supported on this platform");

    const int size = 10;
    const int missingRow = size -  1;
    QItemModelSurfaceDataProxy *proxy  = new QItemModelSurfaceDataProxy();
    QSurface3DSeries *series = new QSurface3DSeries(proxy);
    Q3DSurfaceWidgetItem *graph = new Q3DSurfaceWidgetItem();
    QQuickWidget quickWidget;
    graph->setWidget(&quickWidget);
    graph->addSeries(series);

    // X
    QSurfaceDataArray array;
    array.reserve(size);
    for (int i = 0; i < size; i++) {
        QSurfaceDataRow row(size);
        for (int j = 0; j < size; j++) {
            row[j].setPosition(QVector3D((i == missingRow) ? std::numeric_limits<float>::quiet_NaN()
                                                              : static_cast<float>(i),
                                            qSin(static_cast<float>(i)), static_cast<float>(j)));
        }
        array << row;
    }
    proxy->resetArray(array);
    QVERIFY(!qIsNaN(graph->axisX()->min()));
    QVERIFY(!qIsNaN(graph->axisX()->max()));
    QVERIFY(!qIsNaN(graph->axisY()->min()));
    QVERIFY(!qIsNaN(graph->axisY()->max()));
    QVERIFY(!qIsNaN(graph->axisZ()->min()));
    QVERIFY(!qIsNaN(graph->axisZ()->max()));

    // Y
    for (int i = 0; i < size; i++) {
        QSurfaceDataRow row(size);
        for (int j = 0; j < size; j++) {
            row[j].setPosition(QVector3D(static_cast<float>(i),
                                            (i == missingRow) ? std::numeric_limits<float>::quiet_NaN()
                                                              : qSin(static_cast<float>(i)),
                                             static_cast<float>(j)));
        }
        array << row;
    }
    proxy->resetArray(array);
    QVERIFY(!qIsNaN(graph->axisX()->min()));
    QVERIFY(!qIsNaN(graph->axisX()->max()));
    QVERIFY(!qIsNaN(graph->axisY()->min()));
    QVERIFY(!qIsNaN(graph->axisY()->max()));
    QVERIFY(!qIsNaN(graph->axisZ()->min()));
    QVERIFY(!qIsNaN(graph->axisZ()->max()));

    // Z
    for (int i = 0; i < size; i++) {
        QSurfaceDataRow row(size);
        for (int j = 0; j < size; j++) {
            row[j].setPosition(QVector3D(static_cast<float>(i),
                                            qSin(static_cast<float>(i)),
                                            (i == missingRow) ? std::numeric_limits<float>::quiet_NaN()
                                                              : static_cast<float>(j)));
        }
        array << row;
    }
    proxy->resetArray(array);
    QVERIFY(!qIsNaN(graph->axisX()->min()));
    QVERIFY(!qIsNaN(graph->axisX()->max()));
    QVERIFY(!qIsNaN(graph->axisY()->min()));
    QVERIFY(!qIsNaN(graph->axisY()->max()));
    QVERIFY(!qIsNaN(graph->axisZ()->min()));
    QVERIFY(!qIsNaN(graph->axisZ()->max()));
    delete graph;
}

void tst_proxy::dataContainingNaNFirstLastRow()
{
    if (!CpptestUtil::isOpenGLSupported())
        QSKIP("OpenGL not supported on this platform");

    const int size = 10;
    const int rowFirst = 0;
    const int rowLast = size -  1;
    QItemModelSurfaceDataProxy *proxy = new QItemModelSurfaceDataProxy();
    QSurface3DSeries *series = new QSurface3DSeries(proxy);
    Q3DSurfaceWidgetItem *graph = new Q3DSurfaceWidgetItem();
    QQuickWidget quickWidget;
    graph->setWidget(&quickWidget);
    graph->addSeries(series);

    // X
    QSurfaceDataArray array;
    array.reserve(size);
    for (int i = 0; i < size; i++) {
        bool missingRow = (i == rowFirst || i == rowLast);
        QSurfaceDataRow row(size);
        for (int j = 0; j < size; j++) {
            row[j].setPosition(QVector3D(missingRow ? std::numeric_limits<float>::quiet_NaN()
                                                       : static_cast<float>(i),
                                            qSin(static_cast<float>(i)), static_cast<float>(j)));
        }
        array << row;
    }
    proxy->resetArray(array);
    QVERIFY(!qIsNaN(graph->axisX()->min()));
    QVERIFY(!qIsNaN(graph->axisX()->max()));
    QVERIFY(!qIsNaN(graph->axisY()->min()));
    QVERIFY(!qIsNaN(graph->axisY()->max()));
    QVERIFY(!qIsNaN(graph->axisZ()->min()));
    QVERIFY(!qIsNaN(graph->axisZ()->max()));

    // Y
    for (int i = 0; i < size; i++) {
        bool missingRow = (i == rowFirst || i == rowLast);
        QSurfaceDataRow row(size);
        for (int j = 0; j < size; j++) {
            row[j].setPosition(QVector3D(static_cast<float>(i),
                                            missingRow ? std::numeric_limits<float>::quiet_NaN()
                                                       : qSin(static_cast<float>(i)),
                                             static_cast<float>(j)));
        }
        array << row;
    }
    proxy->resetArray(array);
    QVERIFY(!qIsNaN(graph->axisX()->min()));
    QVERIFY(!qIsNaN(graph->axisX()->max()));
    QVERIFY(!qIsNaN(graph->axisY()->min()));
    QVERIFY(!qIsNaN(graph->axisY()->max()));
    QVERIFY(!qIsNaN(graph->axisZ()->min()));
    QVERIFY(!qIsNaN(graph->axisZ()->max()));

    // Z
    for (int i = 0; i < size; i++) {
        bool missingRow = (i == rowFirst || i == rowLast);
        QSurfaceDataRow row(size);
        for (int j = 0; j < size; j++) {
            row[j].setPosition(QVector3D(static_cast<float>(i),
                                            qSin(static_cast<float>(i)),
                                            missingRow ? std::numeric_limits<float>::quiet_NaN()
                                                       : static_cast<float>(j)));
        }
        array << row;
    }
    proxy->resetArray(array);
    QVERIFY(!qIsNaN(graph->axisX()->min()));
    QVERIFY(!qIsNaN(graph->axisX()->max()));
    QVERIFY(!qIsNaN(graph->axisY()->min()));
    QVERIFY(!qIsNaN(graph->axisY()->max()));
    QVERIFY(!qIsNaN(graph->axisZ()->min()));
    QVERIFY(!qIsNaN(graph->axisZ()->max()));
    delete graph;
}

QTEST_MAIN(tst_proxy)
#include "tst_proxy.moc"
