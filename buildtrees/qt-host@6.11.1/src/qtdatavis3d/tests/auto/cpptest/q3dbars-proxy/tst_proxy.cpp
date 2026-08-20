// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtTest/QtTest>

#include <QtDataVisualization/QBarDataProxy>

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

private:
    QBarDataProxy *m_proxy;
};

void tst_proxy::initTestCase()
{
}

void tst_proxy::cleanupTestCase()
{
}

void tst_proxy::init()
{
    m_proxy = new QBarDataProxy();
}

void tst_proxy::cleanup()
{
    delete m_proxy;
}

void tst_proxy::construct()
{
    QBarDataProxy *proxy = new QBarDataProxy();
    QVERIFY(proxy);
    delete proxy;
}

void tst_proxy::initialProperties()
{
    QVERIFY(m_proxy);

    QCOMPARE(m_proxy->columnLabels().size(), 0);
    QCOMPARE(m_proxy->rowCount(), 0);
    QCOMPARE(m_proxy->rowLabels().size(), 0);
    QVERIFY(!m_proxy->series());

    QCOMPARE(m_proxy->type(), QAbstractDataProxy::DataTypeBar);
}

void tst_proxy::initializeProperties()
{
    QVERIFY(m_proxy);

    m_proxy->setColumnLabels(QStringList() << "1" << "2" << "3");
    QBarDataRow *data = new QBarDataRow;
    *data << 1.0f << 3.0f << 7.5f;
    m_proxy->addRow(data);
    m_proxy->setRowLabels(QStringList() << "1");

    QCOMPARE(m_proxy->columnLabels().size(), 3);
    QCOMPARE(m_proxy->rowCount(), 1);
    QCOMPARE(m_proxy->rowLabels().size(), 1);
}

QTEST_MAIN(tst_proxy)
#include "tst_proxy.moc"
