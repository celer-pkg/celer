// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtTest/QtTest>

#include <QtGraphsWidgets/q3dbarswidgetitem.h>

class tst_input: public QObject
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
    void setQuery();

private:
    Q3DBarsWidgetItem *m_input;
    QQuickWidget *m_quickWidget = nullptr;
};

void tst_input::initTestCase()
{
}

void tst_input::cleanupTestCase()
{
}

void tst_input::init()
{
    m_input = new Q3DBarsWidgetItem();
    m_quickWidget = new QQuickWidget;
    m_input->setWidget(m_quickWidget);
}

void tst_input::cleanup()
{
    delete m_input;
    delete m_quickWidget;
}

void tst_input::construct()
{
    Q3DBarsWidgetItem *input = new Q3DBarsWidgetItem();
    QVERIFY(input);
    delete input;
}

void tst_input::initialProperties()
{
    QVERIFY(m_input);

    QCOMPARE(m_input->isZoomAtTargetEnabled(), true);
    QCOMPARE(m_input->isZoomEnabled(), true);
    QCOMPARE(m_input->isRotationEnabled(), true);
    QCOMPARE(m_input->isSelectionEnabled(), true);
}

void tst_input::initializeProperties()
{
    QVERIFY(m_input);

    m_input->setZoomAtTargetEnabled(false);
    m_input->setZoomEnabled(false);
    m_input->setRotationEnabled(false);
    m_input->setSelectionEnabled(false);

    QCOMPARE(m_input->isZoomAtTargetEnabled(), false);
    QCOMPARE(m_input->isZoomEnabled(), false);
    QCOMPARE(m_input->isRotationEnabled(), false);
    QCOMPARE(m_input->isSelectionEnabled(), false);
}

void tst_input::setQuery()
{
    QSignalSpy spy(m_input, &Q3DGraphsWidgetItem::queriedGraphPositionChanged);
    m_input->scene()->setGraphPositionQuery(QPoint());

    //signal was emitted one time
    QCOMPARE(spy.count(), 1);
    QList<QVariant> arguments = spy.takeFirst();
    QVERIFY(arguments.at(0).typeName() == QStringLiteral("QVector3D"));
}
// TODO: QTRD-3380 (mouse events)

QTEST_MAIN(tst_input)
#include "tst_input.moc"
