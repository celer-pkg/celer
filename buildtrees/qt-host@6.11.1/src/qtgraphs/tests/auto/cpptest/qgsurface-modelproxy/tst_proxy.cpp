// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtTest/QtTest>

#include <QtGraphs/QItemModelSurfaceDataProxy>
#include <QtGraphsWidgets/q3dsurfacewidgetitem.h>
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
    m_proxy = new QItemModelSurfaceDataProxy();
    m_series = new QSurface3DSeries(m_proxy);
}

void tst_proxy::cleanup()
{
    delete m_series;
    m_proxy = 0; // proxy gets deleted with series
}


void tst_proxy::construct()
{
    QItemModelSurfaceDataProxy *proxy = new QItemModelSurfaceDataProxy();
    QVERIFY(proxy);
    QSurface3DSeries *series = new QSurface3DSeries(proxy);
    QVERIFY(series);
    delete series;
    proxy = 0; // proxy gets deleted with series

    QTableWidget table;

    proxy = new QItemModelSurfaceDataProxy(table.model());
    series = new QSurface3DSeries(proxy);
    QVERIFY(proxy);
    QVERIFY(series);
    delete series;
    proxy = 0; // proxy gets deleted with series

    proxy = new QItemModelSurfaceDataProxy(table.model(), "y");
    QVERIFY(proxy);
    series = new QSurface3DSeries(proxy);
    QVERIFY(series);
    QCOMPARE(proxy->rowRole(), QString(""));
    QCOMPARE(proxy->columnRole(), QString(""));
    QCOMPARE(proxy->xPosRole(), QString(""));
    QCOMPARE(proxy->yPosRole(), QString("y"));
    QCOMPARE(proxy->zPosRole(), QString(""));
    QCOMPARE(proxy->rowCategories().size(), 0);
    QCOMPARE(proxy->columnCategories().size(), 0);
    delete series;
    proxy = 0; // proxy gets deleted with series

    proxy = new QItemModelSurfaceDataProxy(table.model(), "row", "column", "y");
    QVERIFY(proxy);
    series = new QSurface3DSeries(proxy);
    QVERIFY(series);
    QCOMPARE(proxy->rowRole(), QString("row"));
    QCOMPARE(proxy->columnRole(), QString("column"));
    QCOMPARE(proxy->xPosRole(), QString("column"));
    QCOMPARE(proxy->yPosRole(), QString("y"));
    QCOMPARE(proxy->zPosRole(), QString("row"));
    QCOMPARE(proxy->rowCategories().size(), 0);
    QCOMPARE(proxy->columnCategories().size(), 0);
    delete series;
    proxy = 0; // proxy gets deleted with series

    proxy = new QItemModelSurfaceDataProxy(table.model(), "row", "column", "x", "y", "z");
    QVERIFY(proxy);
    series = new QSurface3DSeries(proxy);
    QVERIFY(series);
    QCOMPARE(proxy->rowRole(), QString("row"));
    QCOMPARE(proxy->columnRole(), QString("column"));
    QCOMPARE(proxy->xPosRole(), QString("x"));
    QCOMPARE(proxy->yPosRole(), QString("y"));
    QCOMPARE(proxy->zPosRole(), QString("z"));
    QCOMPARE(proxy->rowCategories().size(), 0);
    QCOMPARE(proxy->columnCategories().size(), 0);
    delete series;
    proxy = 0; // proxy gets deleted with series

    proxy = new QItemModelSurfaceDataProxy(table.model(), "row", "column", "y",
                                           QStringList() << "rowCat", QStringList() << "colCat");
    QVERIFY(proxy);
    series = new QSurface3DSeries(proxy);
    QVERIFY(series);
    QCOMPARE(proxy->rowRole(), QString("row"));
    QCOMPARE(proxy->columnRole(), QString("column"));
    QCOMPARE(proxy->xPosRole(), QString("column"));
    QCOMPARE(proxy->yPosRole(), QString("y"));
    QCOMPARE(proxy->zPosRole(), QString("row"));
    QCOMPARE(proxy->rowCategories().size(), 1);
    QCOMPARE(proxy->columnCategories().size(), 1);
    delete series;
    proxy = 0; // proxy gets deleted with series

    proxy = new QItemModelSurfaceDataProxy(table.model(), "row", "column", "x", "y", "z",
                                           QStringList() << "rowCat", QStringList() << "colCat");
    QVERIFY(proxy);
    series = new QSurface3DSeries(proxy);
    QVERIFY(series);
    QCOMPARE(proxy->rowRole(), QString("row"));
    QCOMPARE(proxy->columnRole(), QString("column"));
    QCOMPARE(proxy->xPosRole(), QString("x"));
    QCOMPARE(proxy->yPosRole(), QString("y"));
    QCOMPARE(proxy->zPosRole(), QString("z"));
    QCOMPARE(proxy->rowCategories().size(), 1);
    QCOMPARE(proxy->columnCategories().size(), 1);
    delete series;
    proxy = 0; // proxy gets deleted with series
}

void tst_proxy::initialProperties()
{
    QVERIFY(m_proxy);
    QVERIFY(m_series);

    QCOMPARE(m_proxy->autoColumnCategories(), true);
    QCOMPARE(m_proxy->autoRowCategories(), true);
    QCOMPARE(m_proxy->columnCategories(), QStringList());
    QCOMPARE(m_proxy->columnRole(), QString());
    QCOMPARE(m_proxy->columnRolePattern(), QRegularExpression());
    QCOMPARE(m_proxy->columnRoleReplace(), QString());
    QVERIFY(!m_proxy->itemModel());
    QCOMPARE(m_proxy->multiMatchBehavior(), QItemModelSurfaceDataProxy::MultiMatchBehavior::Last);
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

    QCOMPARE(m_proxy->type(), QAbstractDataProxy::DataType::Surface);
}

void tst_proxy::initializeProperties()
{
    QVERIFY(m_proxy);
    QVERIFY(m_series);

    QSignalSpy itemModelSpy(m_proxy, &QItemModelSurfaceDataProxy::itemModelChanged);
    QSignalSpy rowRoleSpy(m_proxy, &QItemModelSurfaceDataProxy::rowRoleChanged);
    QSignalSpy columnRoleSpy(m_proxy, &QItemModelSurfaceDataProxy::columnRoleChanged);
    QSignalSpy xPosRoleSpy(m_proxy, &QItemModelSurfaceDataProxy::xPosRoleChanged);
    QSignalSpy yPosRoleSpy(m_proxy, &QItemModelSurfaceDataProxy::yPosRoleChanged);
    QSignalSpy zPosRoleSpy(m_proxy, &QItemModelSurfaceDataProxy::zPosRoleChanged);
    QSignalSpy rowCategoriesSpy(m_proxy, &QItemModelSurfaceDataProxy::rowCategoriesChanged);
    QSignalSpy columnCategoriesSpy(m_proxy, &QItemModelSurfaceDataProxy::columnCategoriesChanged);
    QSignalSpy useModelcategoriesSpy(m_proxy, &QItemModelSurfaceDataProxy::useModelCategoriesChanged);
    QSignalSpy autorowCategoriesSpy(m_proxy, &QItemModelSurfaceDataProxy::autoRowCategoriesChanged);
    QSignalSpy autoColumncategoriesSpy(m_proxy, &QItemModelSurfaceDataProxy::autoColumnCategoriesChanged);
    QSignalSpy rowRolePatternSpy(m_proxy, &QItemModelSurfaceDataProxy::rowRolePatternChanged);
    QSignalSpy columnRolePatternSpy(m_proxy, &QItemModelSurfaceDataProxy::columnRolePatternChanged);
    QSignalSpy xPosRolePatternSpy(m_proxy, &QItemModelSurfaceDataProxy::xPosRolePatternChanged);
    QSignalSpy yPosRolePatternSpy(m_proxy, &QItemModelSurfaceDataProxy::yPosRolePatternChanged);
    QSignalSpy zPosRolePatternSpy(m_proxy, &QItemModelSurfaceDataProxy::zPosRolePatternChanged);
    QSignalSpy rowRoleReplaceSpy(m_proxy, &QItemModelSurfaceDataProxy::rowRoleReplaceChanged);
    QSignalSpy columnRoleReplaceSpy(m_proxy, &QItemModelSurfaceDataProxy::columnRoleReplaceChanged);
    QSignalSpy xPosRoleReplaceSpy(m_proxy, &QItemModelSurfaceDataProxy::xPosRoleReplaceChanged);
    QSignalSpy yPosRoleReplaceSpy(m_proxy, &QItemModelSurfaceDataProxy::yPosRoleReplaceChanged);
    QSignalSpy zPosRoleReplaceSpy(m_proxy, &QItemModelSurfaceDataProxy::zPosRoleReplaceChanged);
    QSignalSpy multiMatchSpy(m_proxy, &QItemModelSurfaceDataProxy::multiMatchBehaviorChanged);

    QTableWidget table;

    m_proxy->setAutoColumnCategories(false);
    m_proxy->setAutoRowCategories(false);
    m_proxy->setColumnCategories(QStringList() << "col1" << "col2");
    m_proxy->setColumnRole("column");
    m_proxy->setColumnRolePattern(QRegularExpression("/^.*-(\\d\\d)$/"));
    m_proxy->setColumnRoleReplace("\\\\1");
    m_proxy->setItemModel(table.model());
    m_proxy->setMultiMatchBehavior(QItemModelSurfaceDataProxy::MultiMatchBehavior::Average);
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
    QCOMPARE(m_proxy->multiMatchBehavior(), QItemModelSurfaceDataProxy::MultiMatchBehavior::Average);
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

    QCOMPARE(itemModelSpy.size(), 1);
    QCOMPARE(rowRoleSpy.size(), 1);
    QCOMPARE(columnRoleSpy.size(), 1);
    QCOMPARE(xPosRoleSpy.size(), 1);
    QCOMPARE(yPosRoleSpy.size(), 1);
    QCOMPARE(zPosRoleSpy.size(), 1);
    QCOMPARE(rowCategoriesSpy.size(), 1);
    QCOMPARE(columnCategoriesSpy.size(), 1);
    QCOMPARE(useModelcategoriesSpy.size(), 1);
    QCOMPARE(autorowCategoriesSpy.size(), 1);
    QCOMPARE(autoColumncategoriesSpy.size(), 1);
    QCOMPARE(rowRolePatternSpy.size(), 1);
    QCOMPARE(columnRolePatternSpy.size(), 1);
    QCOMPARE(xPosRolePatternSpy.size(), 1);
    QCOMPARE(yPosRolePatternSpy.size(), 1);
    QCOMPARE(zPosRolePatternSpy.size(), 1);
    QCOMPARE(rowRoleReplaceSpy.size(), 1);
    QCOMPARE(columnRoleReplaceSpy.size(), 1);
    QCOMPARE(xPosRoleReplaceSpy.size(), 1);
    QCOMPARE(yPosRoleReplaceSpy.size(), 1);
    QCOMPARE(zPosRoleReplaceSpy.size(), 1);
    QCOMPARE(multiMatchSpy.size(), 1);
}

void tst_proxy::multiMatch()
{
    if (!CpptestUtil::isOpenGLSupported())
        QSKIP("OpenGL not supported on this platform");

    QQuickWidget quickWidget;
    Q3DSurfaceWidgetItem graph;
    graph.setWidget(&quickWidget);
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

    graph.addSeries(m_series);

    QCoreApplication::processEvents();
    QCOMPARE(graph.axisY()->max(), 10.5f);
    m_proxy->setMultiMatchBehavior(QItemModelSurfaceDataProxy::MultiMatchBehavior::First);
    QCoreApplication::processEvents();
    QCOMPARE(graph.axisY()->max(), 5.5f);
    m_proxy->setMultiMatchBehavior(QItemModelSurfaceDataProxy::MultiMatchBehavior::Last);
    QCoreApplication::processEvents();
    QCOMPARE(graph.axisY()->max(), 10.5f);
    m_proxy->setMultiMatchBehavior(QItemModelSurfaceDataProxy::MultiMatchBehavior::Average);
    QCoreApplication::processEvents();
    QCOMPARE(graph.axisY()->max(), 8.0f);
    m_proxy->setMultiMatchBehavior(QItemModelSurfaceDataProxy::MultiMatchBehavior::CumulativeY);
    QCoreApplication::processEvents();
    QCOMPARE(graph.axisY()->max(), 16.0f);

    QCOMPARE(m_proxy->columnCount(), 2);
    QCOMPARE(m_proxy->rowCount(), 3);

    m_series = 0;
    m_proxy = 0; // Graph deletes proxy
}

QTEST_MAIN(tst_proxy)
#include "tst_proxy.moc"
