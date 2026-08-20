// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtTest/QtTest>

#include <QtGraphs/QItemModelScatterDataProxy>
#include <QtGraphsWidgets/q3dscatterwidgetitem.h>
#include <QtWidgets/QTableWidget>

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

    void addModel();

private:
    QItemModelScatterDataProxy *m_proxy;
    QScatter3DSeries *m_series;
};

void tst_proxy::initTestCase()
{
}

void tst_proxy::cleanupTestCase()
{
}

void tst_proxy::init()
{
    m_proxy = new QItemModelScatterDataProxy();
    m_series = new QScatter3DSeries(m_proxy);
}

void tst_proxy::cleanup()
{
    delete m_proxy;
    m_series = 0;
}

void tst_proxy::construct()
{
    QItemModelScatterDataProxy *proxy = new QItemModelScatterDataProxy();
    QScatter3DSeries *series = new QScatter3DSeries(proxy);
    QVERIFY(proxy);
    QVERIFY(series);
    delete proxy;
    delete series;

    QTableWidget *table = new QTableWidget();

    proxy = new QItemModelScatterDataProxy(table->model());
    series = new QScatter3DSeries(proxy);
    QVERIFY(proxy);
    QVERIFY(series);
    delete proxy;
    delete series;

    proxy = new QItemModelScatterDataProxy(table->model(), "x", "y", "z");
    series = new QScatter3DSeries(proxy);
    QVERIFY(proxy);
    QVERIFY(series);
    QCOMPARE(proxy->xPosRole(), QString("x"));
    QCOMPARE(proxy->yPosRole(), QString("y"));
    QCOMPARE(proxy->zPosRole(), QString("z"));
    QCOMPARE(proxy->rotationRole(), QString(""));
    QCOMPARE(proxy->scaleRole(), QString(""));
    delete proxy;
    delete series;
}

void tst_proxy::initialProperties()
{
    QVERIFY(m_proxy);
    QVERIFY(m_series);

    QVERIFY(!m_proxy->itemModel());
    QCOMPARE(m_proxy->rotationRole(), QString());
    QCOMPARE(m_proxy->rotationRolePattern(), QRegularExpression());
    QCOMPARE(m_proxy->rotationRoleReplace(), QString());
    QCOMPARE(m_proxy->scaleRole(), QString());
    QCOMPARE(m_proxy->scaleRolePattern(), QRegularExpression());
    QCOMPARE(m_proxy->scaleRoleReplace(), QString());
    QCOMPARE(m_proxy->xPosRole(), QString());
    QCOMPARE(m_proxy->xPosRolePattern(), QRegularExpression());
    QCOMPARE(m_proxy->xPosRoleReplace(), QString());
    QCOMPARE(m_proxy->yPosRole(), QString());
    QCOMPARE(m_proxy->yPosRolePattern(), QRegularExpression());
    QCOMPARE(m_proxy->yPosRoleReplace(), QString());
    QCOMPARE(m_proxy->zPosRole(), QString());
    QCOMPARE(m_proxy->zPosRolePattern(), QRegularExpression());
    QCOMPARE(m_proxy->zPosRoleReplace(), QString());

    QCOMPARE(m_proxy->itemCount(), 0);
    QCOMPARE(m_proxy->series()->scaleArray().size(), 0);

    QCOMPARE(m_proxy->type(), QAbstractDataProxy::DataType::Scatter);
}

void tst_proxy::initializeProperties()
{
    QVERIFY(m_proxy);
    QVERIFY(m_series);

    QSignalSpy itemModelSpy(m_proxy, &QItemModelScatterDataProxy::itemModelChanged);
    QSignalSpy xPosRoleSpy(m_proxy, &QItemModelScatterDataProxy::xPosRoleChanged);
    QSignalSpy yPosRoleSpy(m_proxy, &QItemModelScatterDataProxy::yPosRoleChanged);
    QSignalSpy zPosRoleSpy(m_proxy, &QItemModelScatterDataProxy::zPosRoleChanged);
    QSignalSpy rotationRoleSpy(m_proxy, &QItemModelScatterDataProxy::rotationRoleChanged);
    QSignalSpy scaleRoleSpy(m_proxy, &QItemModelScatterDataProxy::scaleRoleChanged);
    QSignalSpy xPosRolePatternSpy(m_proxy, &QItemModelScatterDataProxy::xPosRolePatternChanged);
    QSignalSpy yPosRolePatternSpy(m_proxy, &QItemModelScatterDataProxy::yPosRolePatternChanged);
    QSignalSpy zPosRolePatternSpy(m_proxy, &QItemModelScatterDataProxy::zPosRolePatternChanged);
    QSignalSpy rotationRolePatternSpy(m_proxy, &QItemModelScatterDataProxy::rotationRolePatternChanged);
    QSignalSpy scaleRolePatternSpy(m_proxy, &QItemModelScatterDataProxy::scaleRolePatternChanged);
    QSignalSpy xPosRoleReplaceSpy(m_proxy, &QItemModelScatterDataProxy::xPosRoleReplaceChanged);
    QSignalSpy yPosRoleReplaceSpy(m_proxy, &QItemModelScatterDataProxy::yPosRoleReplaceChanged);
    QSignalSpy zPosRoleReplaceSpy(m_proxy, &QItemModelScatterDataProxy::zPosRoleReplaceChanged);
    QSignalSpy rotationRoleReplaceSpy(m_proxy, &QItemModelScatterDataProxy::rotationRoleReplaceChanged);
    QSignalSpy scaleRoleReplaceSpy(m_proxy, &QItemModelScatterDataProxy::scaleRoleReplaceChanged);

    QTableWidget table;

    m_proxy->setItemModel(table.model());
    m_proxy->setRotationRole("rotation");
    m_proxy->setRotationRolePattern(QRegularExpression("/-/"));
    m_proxy->setRotationRoleReplace("\\\\1");
    m_proxy->setScaleRole("scale");
    m_proxy->setScaleRolePattern(QRegularExpression("/-/"));
    m_proxy->setScaleRoleReplace("\\\\1");
    m_proxy->setXPosRole("X");
    m_proxy->setXPosRolePattern(QRegularExpression("/-/"));
    m_proxy->setXPosRoleReplace("\\\\1");
    m_proxy->setYPosRole("Y");
    m_proxy->setYPosRolePattern(QRegularExpression("/-/"));
    m_proxy->setYPosRoleReplace("\\\\1");
    m_proxy->setZPosRole("Z");
    m_proxy->setZPosRolePattern(QRegularExpression("/-/"));
    m_proxy->setZPosRoleReplace("\\\\1");

    QVERIFY(m_proxy->itemModel());
    QCOMPARE(m_proxy->rotationRole(), QString("rotation"));
    QCOMPARE(m_proxy->rotationRolePattern(), QRegularExpression("/-/"));
    QCOMPARE(m_proxy->rotationRoleReplace(), QString("\\\\1"));
    QCOMPARE(m_proxy->scaleRole(), QString("scale"));
    QCOMPARE(m_proxy->scaleRolePattern(), QRegularExpression("/-/"));
    QCOMPARE(m_proxy->scaleRoleReplace(), QString("\\\\1"));
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
    QCOMPARE(xPosRoleSpy.size(), 1);
    QCOMPARE(yPosRoleSpy.size(), 1);
    QCOMPARE(zPosRoleSpy.size(), 1);
    QCOMPARE(rotationRoleSpy.size(), 1);
    QCOMPARE(scaleRoleSpy.size(), 1);
    QCOMPARE(xPosRolePatternSpy.size(), 1);
    QCOMPARE(yPosRolePatternSpy.size(), 1);
    QCOMPARE(zPosRolePatternSpy.size(), 1);
    QCOMPARE(rotationRolePatternSpy.size(), 1);
    QCOMPARE(scaleRolePatternSpy.size(), 1);
    QCOMPARE(xPosRoleReplaceSpy.size(), 1);
    QCOMPARE(yPosRoleReplaceSpy.size(), 1);
    QCOMPARE(zPosRoleReplaceSpy.size(), 1);
    QCOMPARE(rotationRoleReplaceSpy.size(), 1);
    QCOMPARE(scaleRoleReplaceSpy.size(), 1);
}

void tst_proxy::addModel()
{
    QTableWidget table;
    QStringList rows;
    rows << "row 1";
    QStringList columns;
    columns << "col 1";
    const char *values[1][2] = {{"0/0/5.5/30", "0/0/10.5/30"}};

    table.setRowCount(2);
    table.setColumnCount(1);

    for (int col = 0; col < columns.size(); col++) {
        for (int row = 0; row < rows.size(); row++) {
            QModelIndex index = table.model()->index(col, row);
            table.model()->setData(index, values[col][row]);
        }
    }

    m_proxy->setItemModel(table.model());
    m_proxy->setXPosRole(table.model()->roleNames().value(Qt::DisplayRole));
    m_proxy->setZPosRole(table.model()->roleNames().value(Qt::DisplayRole));
    m_proxy->setXPosRolePattern(QRegularExpression(QStringLiteral("^(\\d*)\\/(\\d*)\\/\\d*[\\.\\,]?\\d*\\/\\d*[\\.\\,]?\\d*$")));
    m_proxy->setXPosRoleReplace(QStringLiteral("\\2"));
    m_proxy->setYPosRolePattern(QRegularExpression(QStringLiteral("^\\d*(\\/)(\\d*)\\/(\\d*[\\.\\,]?\\d*)\\/\\d*[\\.\\,]?\\d*$")));
    m_proxy->setYPosRoleReplace(QStringLiteral("\\3"));
    m_proxy->setZPosRolePattern(QRegularExpression(QStringLiteral("^(\\d*)(\\/)(\\d*)\\/\\d*[\\.\\,]?\\d*\\/\\d*[\\.\\,]?\\d*$")));
    m_proxy->setZPosRoleReplace(QStringLiteral("\\1"));
    QCoreApplication::processEvents();

    QCoreApplication::processEvents();

    QCOMPARE(m_proxy->itemCount(), 2);
    QCOMPARE(m_proxy->series()->scaleArray().size(), 0);
    QVERIFY(m_proxy->series());
    QCOMPARE(m_proxy->series(), m_series);

    delete m_series;
    m_proxy = 0; // proxy gets deleted with series
}

QTEST_MAIN(tst_proxy)
#include "tst_proxy.moc"
