// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qtestcase.h"
#include <QtGraphs/QBarCategoryAxis>
#include <QtTest/QtTest>

class tst_abstractaxis : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    void initialProperties();
    void initializeProperties();

    void showHide();

private:
    // QAbstractAxis is uncreatable, so testing is done through QBarCategoryAxis
    QBarCategoryAxis *m_axis;
};

void tst_abstractaxis::initTestCase() {}

void tst_abstractaxis::cleanupTestCase() {}

void tst_abstractaxis::init()
{
    m_axis = new QBarCategoryAxis();
}

void tst_abstractaxis::cleanup()
{
    delete m_axis;
}

void tst_abstractaxis::initialProperties()
{
    QVERIFY(m_axis);

    QCOMPARE(m_axis->isVisible(), true);
    QCOMPARE(m_axis->isLineVisible(), true);
    QCOMPARE(m_axis->labelsVisible(), true);
    QCOMPARE(m_axis->labelsAngle(), 0);
    QCOMPARE(m_axis->labelDelegate(), nullptr);
    QCOMPARE(m_axis->isGridVisible(), true);
    QCOMPARE(m_axis->isSubGridVisible(), true);
    QCOMPARE(m_axis->titleText(), QString());
    QCOMPARE(m_axis->titleColor(), QColor());
    QCOMPARE(m_axis->isTitleVisible(), true);
    QCOMPARE(m_axis->titleFont(), QFont());
    QCOMPARE(m_axis->alignment(), Qt::AlignAbsolute);
    QCOMPARE(m_axis->textElideMode(), Qt::ElideNone);
}

void tst_abstractaxis::initializeProperties()
{
    QVERIFY(m_axis);

    QSignalSpy spy0(m_axis, &QAbstractAxis::visibleChanged);
    QSignalSpy spy1(m_axis, &QAbstractAxis::lineVisibleChanged);
    QSignalSpy spy2(m_axis, &QAbstractAxis::labelsVisibleChanged);
    QSignalSpy spy3(m_axis, &QAbstractAxis::labelsAngleChanged);
    QSignalSpy spy4(m_axis, &QAbstractAxis::labelDelegateChanged);
    QSignalSpy spy5(m_axis, &QAbstractAxis::gridVisibleChanged);
    QSignalSpy spy6(m_axis, &QAbstractAxis::subGridVisibleChanged);
    QSignalSpy spy7(m_axis, &QAbstractAxis::titleTextChanged);
    QSignalSpy spy8(m_axis, &QAbstractAxis::titleColorChanged);
    QSignalSpy spy9(m_axis, &QAbstractAxis::titleVisibleChanged);
    QSignalSpy spy10(m_axis, &QAbstractAxis::titleFontChanged);
    QSignalSpy spy11(m_axis, &QAbstractAxis::alignmentChanged);
    QSignalSpy spy12(m_axis, &QAbstractAxis::textElideModeChanged);

    auto font = QFont("Arial", 20, 2, true);
    auto labelDelegate = new QQmlComponent(this);

    m_axis->setVisible(false);
    m_axis->setLineVisible(false);
    m_axis->setLabelsVisible(false);
    m_axis->setLabelsAngle(90.0);
    m_axis->setLabelDelegate(labelDelegate);
    m_axis->setGridVisible(false);
    m_axis->setSubGridVisible(false);
    m_axis->setTitleText("Title");
    m_axis->setTitleColor("#ff0000");
    m_axis->setTitleVisible(false);
    m_axis->setTitleFont(font);
    m_axis->setAlignment(Qt::AlignTop);
    m_axis->setTextElideMode(Qt::ElideRight);

    QCOMPARE(m_axis->isVisible(), false);
    QCOMPARE(m_axis->isLineVisible(), false);
    QCOMPARE(m_axis->labelsVisible(), false);
    QCOMPARE(m_axis->labelsAngle(), 90.0);
    QCOMPARE(m_axis->labelDelegate(), labelDelegate);
    QCOMPARE(m_axis->isGridVisible(), false);
    QCOMPARE(m_axis->isSubGridVisible(), false);
    QCOMPARE(m_axis->titleText(), "Title");
    QCOMPARE(m_axis->titleColor(), "#ff0000");
    QCOMPARE(m_axis->isTitleVisible(), false);
    QCOMPARE(m_axis->titleFont(), font);
    QCOMPARE(m_axis->alignment(), Qt::AlignTop);
    QCOMPARE(m_axis->textElideMode(), Qt::ElideRight);

    QCOMPARE(spy0.size(), 1);
    QCOMPARE(spy1.size(), 1);
    QCOMPARE(spy2.size(), 1);
    QCOMPARE(spy3.size(), 1);
    QCOMPARE(spy4.size(), 1);
    QCOMPARE(spy5.size(), 1);
    QCOMPARE(spy6.size(), 1);
    QCOMPARE(spy7.size(), 1);
    QCOMPARE(spy8.size(), 1);
    QCOMPARE(spy9.size(), 1);
    QCOMPARE(spy10.size(), 1);
    QCOMPARE(spy11.size(), 1);
    QCOMPARE(spy12.size(), 1);
}

void tst_abstractaxis::showHide()
{
    QVERIFY(m_axis);

    QSignalSpy spy(m_axis, &QAbstractAxis::visibleChanged);

    m_axis->hide();

    QCOMPARE(m_axis->isVisible(), false);
    QCOMPARE(spy.size(), 1);

    m_axis->show();

    QCOMPARE(m_axis->isVisible(), true);
    QCOMPARE(spy.size(), 2);
}

QTEST_MAIN(tst_abstractaxis)
#include "tst_abstractaxis.moc"
