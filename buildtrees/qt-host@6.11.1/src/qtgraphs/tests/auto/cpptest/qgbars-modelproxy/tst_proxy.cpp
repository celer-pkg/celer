// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtTest/QtTest>

#include <QtGraphs/QItemModelBarDataProxy>
#include <QtGraphsWidgets/q3dbarswidgetitem.h>
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
    QItemModelBarDataProxy *m_proxy;
    QBar3DSeries *m_series;
};

void tst_proxy::initTestCase()
{
}

void tst_proxy::cleanupTestCase()
{
}

void tst_proxy::init()
{
    m_proxy = new QItemModelBarDataProxy();
    m_series = new QBar3DSeries(m_proxy);
}

void tst_proxy::cleanup()
{
    delete m_series;
    m_proxy = 0;
}

void tst_proxy::construct()
{
    QItemModelBarDataProxy *proxy = new QItemModelBarDataProxy();
    QBar3DSeries *series = new QBar3DSeries(proxy);
    QVERIFY(proxy);
    QVERIFY(series);
    delete series;
    proxy = 0;

    QTableWidget *table = new QTableWidget();

    proxy = new QItemModelBarDataProxy(table->model());
    series = new QBar3DSeries(proxy);
    QVERIFY(proxy);
    QVERIFY(series);
    delete series;
    proxy = 0;

    proxy = new QItemModelBarDataProxy(table->model(), "val");
    series = new QBar3DSeries(proxy);
    QVERIFY(proxy);
    QVERIFY(series);
    QCOMPARE(proxy->rowRole(), QString(""));
    QCOMPARE(proxy->columnRole(), QString(""));
    QCOMPARE(proxy->valueRole(), QString("val"));
    QCOMPARE(proxy->rotationRole(), QString(""));
    QCOMPARE(proxy->rowCategories().size(), 0);
    QCOMPARE(proxy->columnCategories().size(), 0);
    delete series;
    proxy = 0;

    proxy = new QItemModelBarDataProxy(table->model(), "row", "col", "val");
    series = new QBar3DSeries(proxy);
    QVERIFY(proxy);
    QCOMPARE(proxy->rowRole(), QString("row"));
    QCOMPARE(proxy->columnRole(), QString("col"));
    QCOMPARE(proxy->valueRole(), QString("val"));
    QCOMPARE(proxy->rotationRole(), QString(""));
    QCOMPARE(proxy->rowCategories().size(), 0);
    QCOMPARE(proxy->columnCategories().size(), 0);
    delete series;
    proxy = 0;

    proxy = new QItemModelBarDataProxy(table->model(), "row", "col", "val", "rot");
    series = new QBar3DSeries(proxy);
    QVERIFY(proxy);
    QVERIFY(series);
    QCOMPARE(proxy->rowRole(), QString("row"));
    QCOMPARE(proxy->columnRole(), QString("col"));
    QCOMPARE(proxy->valueRole(), QString("val"));
    QCOMPARE(proxy->rotationRole(), QString("rot"));
    QCOMPARE(proxy->rowCategories().size(), 0);
    QCOMPARE(proxy->columnCategories().size(), 0);
    delete series;
    proxy = 0;

    proxy = new QItemModelBarDataProxy(table->model(), "row", "col", "val",
                                       QStringList() << "rowCat", QStringList() << "colCat");
    series = new QBar3DSeries(proxy);
    QVERIFY(proxy);
    QVERIFY(series);
    QCOMPARE(proxy->rowRole(), QString("row"));
    QCOMPARE(proxy->columnRole(), QString("col"));
    QCOMPARE(proxy->valueRole(), QString("val"));
    QCOMPARE(proxy->rotationRole(), QString(""));
    QCOMPARE(proxy->rowCategories().size(), 1);
    QCOMPARE(proxy->columnCategories().size(), 1);
    delete series;
    proxy = 0;

    proxy = new QItemModelBarDataProxy(table->model(), "row", "col", "val", "rot",
                                       QStringList() << "rowCat", QStringList() << "colCat");
    series = new QBar3DSeries(proxy);
    QVERIFY(proxy);
    QVERIFY(series);
    QCOMPARE(proxy->rowRole(), QString("row"));
    QCOMPARE(proxy->columnRole(), QString("col"));
    QCOMPARE(proxy->valueRole(), QString("val"));
    QCOMPARE(proxy->rotationRole(), QString("rot"));
    QCOMPARE(proxy->rowCategories().size(), 1);
    QCOMPARE(proxy->columnCategories().size(), 1);
    delete series;
    proxy = 0;
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
    QCOMPARE(m_proxy->multiMatchBehavior(), QItemModelBarDataProxy::MultiMatchBehavior::Last);
    QCOMPARE(m_proxy->rotationRole(), QString());
    QCOMPARE(m_proxy->rotationRolePattern(), QRegularExpression());
    QCOMPARE(m_proxy->rotationRoleReplace(), QString());
    QCOMPARE(m_proxy->rowCategories(), QStringList());
    QCOMPARE(m_proxy->rowRole(), QString());
    QCOMPARE(m_proxy->rowRolePattern(), QRegularExpression());
    QCOMPARE(m_proxy->rowRoleReplace(), QString());
    QCOMPARE(m_proxy->useModelCategories(), false);
    QCOMPARE(m_proxy->valueRole(), QString());
    QCOMPARE(m_proxy->valueRolePattern(), QRegularExpression());
    QCOMPARE(m_proxy->valueRoleReplace(), QString());

    QCOMPARE(m_proxy->series()->columnLabels().size(), 0);
    QCOMPARE(m_proxy->rowCount(), 0);
    QCOMPARE(m_proxy->series()->rowLabels().size(), 0);

    QCOMPARE(m_proxy->type(), QAbstractDataProxy::DataType::Bar);
}

void tst_proxy::initializeProperties()
{
    QVERIFY(m_proxy);
    QVERIFY(m_series);

    QSignalSpy itemModelSpy(m_proxy, &QItemModelBarDataProxy::itemModelChanged);
    QSignalSpy rowRoleSpy(m_proxy, &QItemModelBarDataProxy::rowRoleChanged);
    QSignalSpy valueRoleSpy(m_proxy, &QItemModelBarDataProxy::valueRoleChanged);
    QSignalSpy rotationRoleSpy(m_proxy, &QItemModelBarDataProxy::rotationRoleChanged);
    QSignalSpy rowCategoriesSpy(m_proxy, &QItemModelBarDataProxy::rowCategoriesChanged);
    QSignalSpy columnCategoriesSpy(m_proxy, &QItemModelBarDataProxy::columnCategoriesChanged);
    QSignalSpy useModelCategoriesSpy(m_proxy, &QItemModelBarDataProxy::useModelCategoriesChanged);
    QSignalSpy autoRowCategoriesSpy(m_proxy, &QItemModelBarDataProxy::autoRowCategoriesChanged);
    QSignalSpy autoColumnCategoriesSpy(m_proxy, &QItemModelBarDataProxy::autoColumnCategoriesChanged);
    QSignalSpy rowRolePatternSpy(m_proxy, &QItemModelBarDataProxy::rowRoleReplaceChanged);
    QSignalSpy ColumnRolePatterSpy(m_proxy, &QItemModelBarDataProxy::columnRoleReplaceChanged);
    QSignalSpy rotationRolePatternSpy(m_proxy, &QItemModelBarDataProxy::rotationRolePatternChanged);
    QSignalSpy rowRoleReplaceSpy(m_proxy, &QItemModelBarDataProxy::rowRoleReplaceChanged);
    QSignalSpy columnRoleReplaceSpy(m_proxy, &QItemModelBarDataProxy::columnRoleReplaceChanged);
    QSignalSpy valueRoleReplacedSpy(m_proxy, &QItemModelBarDataProxy::valueRoleReplaceChanged);
    QSignalSpy rotationRoleReplacedSpy(m_proxy, &QItemModelBarDataProxy::rotationRoleReplaceChanged);
    QSignalSpy multiMatchSpy(m_proxy, &QItemModelBarDataProxy::multiMatchBehaviorChanged);

    QTableWidget table;

    m_proxy->setAutoColumnCategories(false);
    m_proxy->setAutoRowCategories(false);
    m_proxy->setColumnCategories(QStringList() << "col1" << "col2");
    m_proxy->setColumnRole("column");
    m_proxy->setColumnRolePattern(QRegularExpression("/^.*-(\\d\\d)$/"));
    m_proxy->setColumnRoleReplace("\\\\1");
    m_proxy->setItemModel(table.model());
    m_proxy->setMultiMatchBehavior(QItemModelBarDataProxy::MultiMatchBehavior::Average);
    m_proxy->setRotationRole("rotation");
    m_proxy->setRotationRolePattern(QRegularExpression("/-/"));
    m_proxy->setRotationRoleReplace("\\\\1");
    m_proxy->setRowCategories(QStringList() << "row1" << "row2");
    m_proxy->setRowRole("row");
    m_proxy->setRowRolePattern(QRegularExpression("/^(\\d\\d\\d\\d).*$/"));
    m_proxy->setRowRoleReplace("\\\\1");
    m_proxy->setUseModelCategories(true);
    m_proxy->setValueRole("value");
    m_proxy->setValueRolePattern(QRegularExpression("/-/"));
    m_proxy->setValueRoleReplace("\\\\1");

    QCOMPARE(m_proxy->autoColumnCategories(), false);
    QCOMPARE(m_proxy->autoRowCategories(), false);
    QCOMPARE(m_proxy->columnCategories().size(), 2);
    QCOMPARE(m_proxy->columnRole(), QString("column"));
    QCOMPARE(m_proxy->columnRolePattern(), QRegularExpression("/^.*-(\\d\\d)$/"));
    QCOMPARE(m_proxy->columnRoleReplace(), QString("\\\\1"));
    QVERIFY(m_proxy->itemModel());
    QCOMPARE(m_proxy->multiMatchBehavior(), QItemModelBarDataProxy::MultiMatchBehavior::Average);
    QCOMPARE(m_proxy->rotationRole(), QString("rotation"));
    QCOMPARE(m_proxy->rotationRolePattern(), QRegularExpression("/-/"));
    QCOMPARE(m_proxy->rotationRoleReplace(), QString("\\\\1"));
    QCOMPARE(m_proxy->rowCategories().size(), 2);
    QCOMPARE(m_proxy->rowRole(), QString("row"));
    QCOMPARE(m_proxy->rowRolePattern(), QRegularExpression("/^(\\d\\d\\d\\d).*$/"));
    QCOMPARE(m_proxy->rowRoleReplace(), QString("\\\\1"));
    QCOMPARE(m_proxy->useModelCategories(), true);
    QCOMPARE(m_proxy->valueRole(), QString("value"));
    QCOMPARE(m_proxy->valueRolePattern(), QRegularExpression("/-/"));
    QCOMPARE(m_proxy->valueRoleReplace(), QString("\\\\1"));

    QCOMPARE(itemModelSpy.size(), 1);
    QCOMPARE(rowRoleSpy.size(), 1);
    QCOMPARE(valueRoleSpy.size(), 1);
    QCOMPARE(rotationRoleSpy.size(), 1);
    QCOMPARE(rowCategoriesSpy.size(), 1);
    QCOMPARE(columnCategoriesSpy.size(), 1);
    QCOMPARE(useModelCategoriesSpy.size(), 1);
    QCOMPARE(autoRowCategoriesSpy.size(), 1);
    QCOMPARE(autoColumnCategoriesSpy.size(), 1);
    QCOMPARE(rowRolePatternSpy.size(), 1);
    QCOMPARE(ColumnRolePatterSpy.size(), 1);
    QCOMPARE(rotationRolePatternSpy.size(), 1);
    QCOMPARE(rowRoleReplaceSpy.size(), 1);
    QCOMPARE(columnRoleReplaceSpy.size(), 1);
    QCOMPARE(valueRoleReplacedSpy.size(), 1);
    QCOMPARE(rotationRoleReplacedSpy.size(), 1);
    QCOMPARE(multiMatchSpy.size(), 1);
}

void tst_proxy::multiMatch()
{
    if (!CpptestUtil::isOpenGLSupported())
        QSKIP("OpenGL not supported on this platform");

    QQuickWidget quickWidget;
    Q3DBarsWidgetItem graph;
    graph.setWidget(&quickWidget);

    QTableWidget table;
    QStringList rows;
    rows << "row 1" << "row 2" << "row 3";
    QStringList columns;
    columns << "col 1";
    const char *values[1][3] = {{"0/0/3.5/30", "0/0/5.0/30", "0/0/6.5/30"}};

    table.setRowCount(1);
    table.setColumnCount(3);

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
    m_proxy->setValueRolePattern(QRegularExpression(QStringLiteral("^\\d*(\\/)(\\d*)\\/(\\d*[\\.\\,]?\\d*)\\/\\d*[\\.\\,]?\\d*$")));
    m_proxy->setValueRoleReplace(QStringLiteral("\\3"));
    m_proxy->setColumnRolePattern(QRegularExpression(QStringLiteral("^(\\d*)(\\/)(\\d*)\\/\\d*[\\.\\,]?\\d*\\/\\d*[\\.\\,]?\\d*$")));
    m_proxy->setColumnRoleReplace(QStringLiteral("\\1"));
    QCoreApplication::processEvents();

    graph.addSeries(m_series);

    QCoreApplication::processEvents();
    QCOMPARE(graph.valueAxis()->max(), 6.5f);
    m_proxy->setMultiMatchBehavior(QItemModelBarDataProxy::MultiMatchBehavior::First);
    QCoreApplication::processEvents();
    QCOMPARE(graph.valueAxis()->max(), 3.5f);
    m_proxy->setMultiMatchBehavior(QItemModelBarDataProxy::MultiMatchBehavior::Last);
    QCoreApplication::processEvents();
    QCOMPARE(graph.valueAxis()->max(), 6.5f);
    m_proxy->setMultiMatchBehavior(QItemModelBarDataProxy::MultiMatchBehavior::Average);
    QCoreApplication::processEvents();
    QCOMPARE(graph.valueAxis()->max(), 5.0f);
    m_proxy->setMultiMatchBehavior(QItemModelBarDataProxy::MultiMatchBehavior::Cumulative);
    QCoreApplication::processEvents();
    QCOMPARE(graph.valueAxis()->max(), 15.0f);

    QCOMPARE(m_proxy->series()->columnLabels().size(), 1);
    QCOMPARE(m_proxy->rowCount(), 1);
    QCOMPARE(m_proxy->series()->rowLabels().size(), 1);
    QVERIFY(m_proxy->series());

    m_series = 0;
    m_proxy = 0; // Proxy gets deleted as graph gets deleted
}

QTEST_MAIN(tst_proxy)
#include "tst_proxy.moc"
