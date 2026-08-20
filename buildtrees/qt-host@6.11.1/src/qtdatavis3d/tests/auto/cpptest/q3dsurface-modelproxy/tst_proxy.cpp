// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtTest/QtTest>

#include <QtDataVisualization/QItemModelSurfaceDataProxy>
#include <QtDataVisualization/Q3DSurface>
#include <QtWidgets/QTableWidget>

#include "cpptestutil.h"

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

    void multiMatch();

private:
    QItemModelSurfaceDataProxy *m_proxy;
};

void tst_proxy::initTestCase()
{
}

void tst_proxy::cleanupTestCase()
{
}

void tst_proxy::init()
{
    m_proxy = new QItemModelSurfaceDataProxy();
}

void tst_proxy::cleanup()
{
    delete m_proxy;
}


void tst_proxy::construct()
{
    QItemModelSurfaceDataProxy *proxy = new QItemModelSurfaceDataProxy();
    QVERIFY(proxy);
    delete proxy;

    QTableWidget table;

    proxy = new QItemModelSurfaceDataProxy(table.model());
    QVERIFY(proxy);
    delete proxy;

    proxy = new QItemModelSurfaceDataProxy(table.model(), "y");
    QVERIFY(proxy);
    QCOMPARE(proxy->rowRole(), QString(""));
    QCOMPARE(proxy->columnRole(), QString(""));
    QCOMPARE(proxy->xPosRole(), QString(""));
    QCOMPARE(proxy->yPosRole(), QString("y"));
    QCOMPARE(proxy->zPosRole(), QString(""));
    QCOMPARE(proxy->rowCategories().size(), 0);
    QCOMPARE(proxy->columnCategories().size(), 0);
    delete proxy;

    proxy = new QItemModelSurfaceDataProxy(table.model(), "row", "column", "y");
    QVERIFY(proxy);
    QCOMPARE(proxy->rowRole(), QString("row"));
    QCOMPARE(proxy->columnRole(), QString("column"));
    QCOMPARE(proxy->xPosRole(), QString("column"));
    QCOMPARE(proxy->yPosRole(), QString("y"));
    QCOMPARE(proxy->zPosRole(), QString("row"));
    QCOMPARE(proxy->rowCategories().size(), 0);
    QCOMPARE(proxy->columnCategories().size(), 0);
    delete proxy;

    proxy = new QItemModelSurfaceDataProxy(table.model(), "row", "column", "x", "y", "z");
    QVERIFY(proxy);
    QCOMPARE(proxy->rowRole(), QString("row"));
    QCOMPARE(proxy->columnRole(), QString("column"));
    QCOMPARE(proxy->xPosRole(), QString("x"));
    QCOMPARE(proxy->yPosRole(), QString("y"));
    QCOMPARE(proxy->zPosRole(), QString("z"));
    QCOMPARE(proxy->rowCategories().size(), 0);
    QCOMPARE(proxy->columnCategories().size(), 0);
    delete proxy;

    proxy = new QItemModelSurfaceDataProxy(table.model(), "row", "column", "y",
                                           QStringList() << "rowCat", QStringList() << "colCat");
    QVERIFY(proxy);
    QCOMPARE(proxy->rowRole(), QString("row"));
    QCOMPARE(proxy->columnRole(), QString("column"));
    QCOMPARE(proxy->xPosRole(), QString("column"));
    QCOMPARE(proxy->yPosRole(), QString("y"));
    QCOMPARE(proxy->zPosRole(), QString("row"));
    QCOMPARE(proxy->rowCategories().size(), 1);
    QCOMPARE(proxy->columnCategories().size(), 1);
    delete proxy;

    proxy = new QItemModelSurfaceDataProxy(table.model(), "row", "column", "x", "y", "z",
                                           QStringList() << "rowCat", QStringList() << "colCat");
    QVERIFY(proxy);
    QCOMPARE(proxy->rowRole(), QString("row"));
    QCOMPARE(proxy->columnRole(), QString("column"));
    QCOMPARE(proxy->xPosRole(), QString("x"));
    QCOMPARE(proxy->yPosRole(), QString("y"));
    QCOMPARE(proxy->zPosRole(), QString("z"));
    QCOMPARE(proxy->rowCategories().size(), 1);
    QCOMPARE(proxy->columnCategories().size(), 1);
    delete proxy;
}

void tst_proxy::initialProperties()
{
    QVERIFY(m_proxy);

    QCOMPARE(m_proxy->autoColumnCategories(), true);
    QCOMPARE(m_proxy->autoRowCategories(), true);
    QCOMPARE(m_proxy->columnCategories(), QStringList());
    QCOMPARE(m_proxy->columnRole(), QString());
    QCOMPARE(m_proxy->columnRolePattern(), QRegularExpression());
    QCOMPARE(m_proxy->columnRoleReplace(), QString());
    QVERIFY(!m_proxy->itemModel());
    QCOMPARE(m_proxy->multiMatchBehavior(), QItemModelSurfaceDataProxy::MMBLast);
    QCOMPARE(m_proxy->rowCategories(), QStringList());
    QCOMPARE(m_proxy->rowRole(), QString());
    QCOMPARE(m_proxy->rowRolePattern(), QRegularExpression());
    QCOMPARE(m_proxy->rowRoleReplace(), QString());
    QCOMPARE(m_proxy->useModelCategories(), false);
    QCOMPARE(m_proxy->xPosRole(), QString());
    QCOMPARE(m_proxy->xPosRolePattern(), QRegularExpression());
    QCOMPARE(m_proxy->xPosRoleReplace(), QString());
    QCOMPARE(m_proxy->yPosRole(), QString());
    QCOMPARE(m_proxy->yPosRolePattern(), QRegularExpression());
    QCOMPARE(m_proxy->yPosRoleReplace(), QString());
    QCOMPARE(m_proxy->zPosRole(), QString());
    QCOMPARE(m_proxy->zPosRolePattern(), QRegularExpression());
    QCOMPARE(m_proxy->zPosRoleReplace(), QString());

    QCOMPARE(m_proxy->columnCount(), 0);
    QCOMPARE(m_proxy->rowCount(), 0);
    QVERIFY(!m_proxy->series());

    QCOMPARE(m_proxy->type(), QAbstractDataProxy::DataTypeSurface);
}

void tst_proxy::initializeProperties()
{
    QVERIFY(m_proxy);

    QTableWidget table;

    m_proxy->setAutoColumnCategories(false);
    m_proxy->setAutoRowCategories(false);
    m_proxy->setColumnCategories(QStringList() << "col1" << "col2");
    m_proxy->setColumnRole("column");
    m_proxy->setColumnRolePattern(QRegularExpression("/^.*-(\\d\\d)$/"));
    m_proxy->setColumnRoleReplace("\\\\1");
    m_proxy->setItemModel(table.model());
    m_proxy->setMultiMatchBehavior(QItemModelSurfaceDataProxy::MMBAverage);
    m_proxy->setRowCategories(QStringList() << "row1" << "row2");
    m_proxy->setRowRole("row");
    m_proxy->setRowRolePattern(QRegularExpression("/^(\\d\\d\\d\\d).*$/"));
    m_proxy->setRowRoleReplace("\\\\1");
    m_proxy->setUseModelCategories(true);
    m_proxy->setXPosRole("X");
    m_proxy->setXPosRolePattern(QRegularExpression("/-/"));
    m_proxy->setXPosRoleReplace("\\\\1");
    m_proxy->setYPosRole("Y");
    m_proxy->setYPosRolePattern(QRegularExpression("/-/"));
    m_proxy->setYPosRoleReplace("\\\\1");
    m_proxy->setZPosRole("Z");
    m_proxy->setZPosRolePattern(QRegularExpression("/-/"));
    m_proxy->setZPosRoleReplace("\\\\1");

    QCOMPARE(m_proxy->autoColumnCategories(), false);
    QCOMPARE(m_proxy->autoRowCategories(), false);
    QCOMPARE(m_proxy->columnCategories().size(), 2);
    QCOMPARE(m_proxy->columnRole(), QString("column"));
    QCOMPARE(m_proxy->columnRolePattern(), QRegularExpression("/^.*-(\\d\\d)$/"));
    QCOMPARE(m_proxy->columnRoleReplace(), QString("\\\\1"));
    QVERIFY(m_proxy->itemModel());
    QCOMPARE(m_proxy->multiMatchBehavior(), QItemModelSurfaceDataProxy::MMBAverage);
    QCOMPARE(m_proxy->rowCategories().size(), 2);
    QCOMPARE(m_proxy->rowRole(), QString("row"));
    QCOMPARE(m_proxy->rowRolePattern(), QRegularExpression("/^(\\d\\d\\d\\d).*$/"));
    QCOMPARE(m_proxy->rowRoleReplace(), QString("\\\\1"));
    QCOMPARE(m_proxy->useModelCategories(), true);
    QCOMPARE(m_proxy->xPosRole(), QString("X"));
    QCOMPARE(m_proxy->xPosRolePattern(), QRegularExpression("/-/"));
    QCOMPARE(m_proxy->xPosRoleReplace(), QString("\\\\1"));
    QCOMPARE(m_proxy->yPosRole(), QString("Y"));
    QCOMPARE(m_proxy->yPosRolePattern(), QRegularExpression("/-/"));
    QCOMPARE(m_proxy->yPosRoleReplace(), QString("\\\\1"));
    QCOMPARE(m_proxy->zPosRole(), QString("Z"));
    QCOMPARE(m_proxy->zPosRolePattern(), QRegularExpression("/-/"));
    QCOMPARE(m_proxy->zPosRoleReplace(), QString("\\\\1"));
}

void tst_proxy::multiMatch()
{
    if (!CpptestUtil::isOpenGLSupported())
        QSKIP("OpenGL not supported on this platform");

    Q3DSurface graph;

    QTableWidget table;
    QStringList rows;
    rows << "row 1" << "row 2";
    QStringList columns;
    columns << "col 1" << "col 2" << "col 3" << "col 4";
    const char *values[4][2] = {{"0/0/5.5/30", "0/0/10.5/30"},
                               {"0/1/5.5/30", "0/1/0.5/30"},
                               {"1/0/5.5/30", "1/0/0.5/30"},
                               {"1/1/0.0/30", "1/1/0.0/30"}};

    table.setRowCount(2);
    table.setColumnCount(4);

    for (int col = 0; col < columns.size(); col++) {
        for (int row = 0; row < rows.size(); row++) {
            QModelIndex index = table.model()->index(col, row);
            table.model()->setData(index, values[col][row]);
        }
    }

    m_proxy->setItemModel(table.model());
    m_proxy->setRowRole(table.model()->roleNames().value(Qt::DisplayRole));
    m_proxy->setColumnRole(table.model()->roleNames().value(Qt::DisplayRole));
    m_proxy->setRowRolePattern(QRegularExpression(QStringLiteral("^(\\d*)\\/(\\d*)\\/\\d*[\\.\\,]?\\d*\\/\\d*[\\.\\,]?\\d*$")));
    m_proxy->setRowRoleReplace(QStringLiteral("\\2"));
    m_proxy->setYPosRolePattern(QRegularExpression(QStringLiteral("^\\d*(\\/)(\\d*)\\/(\\d*[\\.\\,]?\\d*)\\/\\d*[\\.\\,]?\\d*$")));
    m_proxy->setYPosRoleReplace(QStringLiteral("\\3"));
    m_proxy->setColumnRolePattern(QRegularExpression(QStringLiteral("^(\\d*)(\\/)(\\d*)\\/\\d*[\\.\\,]?\\d*\\/\\d*[\\.\\,]?\\d*$")));
    m_proxy->setColumnRoleReplace(QStringLiteral("\\1"));
    QCoreApplication::processEvents();

    QSurface3DSeries *series = new QSurface3DSeries(m_proxy);

    graph.addSeries(series);

    QCoreApplication::processEvents();
    QCOMPARE(graph.axisY()->max(), 10.5f);
    m_proxy->setMultiMatchBehavior(QItemModelSurfaceDataProxy::MMBFirst);
    QCoreApplication::processEvents();
    QCOMPARE(graph.axisY()->max(), 5.5f);
    m_proxy->setMultiMatchBehavior(QItemModelSurfaceDataProxy::MMBLast);
    QCoreApplication::processEvents();
    QCOMPARE(graph.axisY()->max(), 10.5f);
    m_proxy->setMultiMatchBehavior(QItemModelSurfaceDataProxy::MMBAverage);
    QCoreApplication::processEvents();
    QCOMPARE(graph.axisY()->max(), 8.0f);
    m_proxy->setMultiMatchBehavior(QItemModelSurfaceDataProxy::MMBCumulativeY);
    QCoreApplication::processEvents();
    QCOMPARE(graph.axisY()->max(), 16.0f);

    QCOMPARE(m_proxy->columnCount(), 2);
    QCOMPARE(m_proxy->rowCount(), 3);
    QVERIFY(m_proxy->series());

    m_proxy = 0; // Graph deletes proxy
}

QTEST_MAIN(tst_proxy)
#include "tst_proxy.moc"
