// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtTest/QtTest>

#include <QtDataVisualization/Q3DInputHandler>

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

private:
    Q3DInputHandler *m_input;
};

void tst_input::initTestCase()
{
}

void tst_input::cleanupTestCase()
{
}

void tst_input::init()
{
    m_input = new Q3DInputHandler();
}

void tst_input::cleanup()
{
    delete m_input;
}

void tst_input::construct()
{
    Q3DInputHandler *input = new Q3DInputHandler();
    QVERIFY(input);
    delete input;
}

void tst_input::initialProperties()
{
    QVERIFY(m_input);

    QCOMPARE(m_input->isRotationEnabled(), true);
    QCOMPARE(m_input->isSelectionEnabled(), true);
    QCOMPARE(m_input->isZoomAtTargetEnabled(), true);
    QCOMPARE(m_input->isZoomEnabled(), true);

    // Common (from QAbstract3DInputHandler)
    QCOMPARE(m_input->inputPosition(), QPoint(0, 0));
    QCOMPARE(m_input->inputView(), QAbstract3DInputHandler::InputViewNone);
    QVERIFY(!m_input->scene());
}

void tst_input::initializeProperties()
{
    QVERIFY(m_input);

    m_input->setRotationEnabled(false);
    m_input->setSelectionEnabled(false);
    m_input->setZoomAtTargetEnabled(false);
    m_input->setZoomEnabled(false);

    QCOMPARE(m_input->isRotationEnabled(), false);
    QCOMPARE(m_input->isSelectionEnabled(), false);
    QCOMPARE(m_input->isZoomAtTargetEnabled(), false);
    QCOMPARE(m_input->isZoomEnabled(), false);

    // Common (from QAbstract3DInputHandler)
    m_input->setInputPosition(QPoint(100, 100));
    m_input->setInputView(QAbstract3DInputHandler::InputViewOnPrimary);

    QCOMPARE(m_input->inputPosition(), QPoint(100, 100));
    QCOMPARE(m_input->inputView(), QAbstract3DInputHandler::InputViewOnPrimary);
}

// TODO: QTRD-3380 (mouse events)

QTEST_MAIN(tst_input)
#include "tst_input.moc"
