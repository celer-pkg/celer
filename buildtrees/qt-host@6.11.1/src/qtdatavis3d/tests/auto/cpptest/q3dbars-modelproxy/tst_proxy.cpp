// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtTest/QtTest>

#include <QtDataVisualization/QItemModelBarDataProxy>
#include <QtDataVisualization/Q3DBars>
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
}

void tst_proxy::cleanup()
{
    delete m_proxy;
}

void tst_proxy::construct()
{
    QItemModelBarDataProxy *proxy = new QItemModelBarDataProxy();
    QVERIFY(proxy);
    delete proxy;

    QTableWidget *table = new QTableWidget();

    proxy = new QItemModelBarDataProxy(table->model());
    QVERIFY(proxy);
    delete proxy;

    proxy = new QItemModelBarDataProxy(table->model(), "val");
    QVERIFY(proxy);
    QCOMPARE(proxy->rowRole(), QString(""));
    QCOMPARE(proxy->columnRole(), QString(""));
    QCOMPARE(proxy->valueRole(), QString("val"));
    QCOMPARE(proxy->rotationRole(), QString(""));
    QCOMPARE(proxy->rowCategories().size(), 0);
    QCOMPARE(proxy->columnCategories().size(), 0);
    delete proxy;

    proxy = new QItemModelBarDataProxy(table->model(), "row", "col", "val");
    QVERIFY(proxy);
    QCOMPARE(proxy->rowRole(), QString("row"));
    QCOMPARE(proxy->columnRole(), QString("col"));
    QCOMPARE(proxy->valueRole(), QString("val"));
    QCOMPARE(proxy->rotationRole(), QString(""));
    QCOMPARE(proxy->rowCategories().size(), 0);
    QCOMPARE(proxy->columnCategories().size(), 0);
    delete proxy;

    proxy = new QItemModelBarDataProxy(table->model(), "row", "col", "val", "rot");
    QVERIFY(proxy);
    QCOMPARE(proxy->rowRole(), QString("row"));
    QCOMPARE(proxy->columnRole(), QString("col"));
    QCOMPARE(proxy->valueRole(), QString("val"));
    QCOMPARE(proxy->rotationRole(), QString("rot"));
    QCOMPARE(proxy->rowCategories().size(), 0);
    QCOMPARE(proxy->columnCategories().size(), 0);
    delete proxy;

    proxy = new QItemModelBarDataProxy(table->model(), "row", "col", "val",
                                       QStringList() << "rowCat", QStringList() << "colCat");
    QVERIFY(proxy);
    QCOMPARE(proxy->rowRole(), QString("row"));
    QCOMPARE(proxy->columnRole(), QString("col"));
    QCOMPARE(proxy->valueRole(), QString("val"));
    QCOMPARE(proxy->rotationRole(), QString(""));
    QCOMPARE(proxy->rowCategories().size(), 1);
    QCOMPARE(proxy->columnCategories().size(), 1);
    delete proxy;

    proxy = new QItemModelBarDataProxy(table->model(), "row", "col", "val", "rot",
                                       QStringList() << "rowCat", QStringList() << "colCat");
    QVERIFY(proxy);
    QCOMPARE(proxy->rowRole(), QString("row"));
    QCOMPARE(proxy->columnRole(), QString("col"));
    QCOMPARE(proxy->valueRole(), QString("val"));
    QCOMPARE(proxy->rotationRole(), QString("rot"));
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
    QCOMPARE(m_proxy->multiMatchBehavior(), QItemModelBarDataProxy::MMBLast);
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

    QCOMPARE(m_proxy->columnLabels().size(), 0);
    QCOMPARE(m_proxy->rowCount(), 0);
    QCOMPARE(m_proxy->rowLabels().size(), 0);
    QVERIFY(!m_proxy->series());

    QCOMPARE(m_proxy->type(), QAbstractDataProxy::DataTypeBar);
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
    m_proxy->setMultiMatchBehavior(QItemModelBarDataProxy::MMBAverage);
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
    QCOMPARE(m_proxy->multiMatchBehavior(), QItemModelBarDataProxy::MMBAverage);
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
}

void tst_proxy::multiMatch()
{
    if (!CpptestUtil::isOpenGLSupported())
        QSKIP("OpenGL not supported on this platform");

    Q3DBars graph;

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

    QBar3DSeries *series = new QBar3DSeries(m_proxy);

    graph.addSeries(series);

    QCoreApplication::processEvents();
    QCOMPARE(graph.valueAxis()->max(), 6.5f);
    m_proxy->setMultiMatchBehavior(QItemModelBarDataProxy::MMBFirst);
    QCoreApplication::processEvents();
    QCOMPARE(graph.valueAxis()->max(), 3.5f);
    m_proxy->setMultiMatchBehavior(QItemModelBarDataProxy::MMBLast);
    QCoreApplication::processEvents();
    QCOMPARE(graph.valueAxis()->max(), 6.5f);
    m_proxy->setMultiMatchBehavior(QItemModelBarDataProxy::MMBAverage);
    QCoreApplication::processEvents();
    QCOMPARE(graph.valueAxis()->max(), 5.0f);
    m_proxy->setMultiMatchBehavior(QItemModelBarDataProxy::MMBCumulative);
    QCoreApplication::processEvents();
    QCOMPARE(graph.valueAxis()->max(), 15.0f);

    QCOMPARE(m_proxy->columnLabels().size(), 1);
    QCOMPARE(m_proxy->rowCount(), 1);
    QCOMPARE(m_proxy->rowLabels().size(), 1);
    QVERIFY(m_proxy->series());

    m_proxy = 0; // Proxy gets deleted as graph gets deleted
}

QTEST_MAIN(tst_proxy)
#include "tst_proxy.moc"
