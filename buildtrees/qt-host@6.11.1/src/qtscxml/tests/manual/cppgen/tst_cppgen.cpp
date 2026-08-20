// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtTest/QtTest>

#include <QtScxml/scxmlcppdumper.h>

using namespace Scxml;

class TestCppGen: public QObject
{
    Q_OBJECT

private slots:
    void idMangling_data();
    void idMangling();
};

void TestCppGen::idMangling_data()
{
    QTest::addColumn<QString>("unmangled");
    QTest::addColumn<QString>("mangled");

    QTest::newRow("One:Two") << "One:Two" << "One_colon_Two";
    QTest::newRow("one-piece") << "one-piece" << "one_dash_piece";
    QTest::newRow("two_words") << "two_words" << "two__words";
    QTest::newRow("me@work") << "me@work" << "me_at_work";
}

void TestCppGen::idMangling()
{
    QFETCH(QString, unmangled);
    QFETCH(QString, mangled);

    QCOMPARE(CppDumper::mangleId(unmangled), mangled);
}

QTEST_MAIN(TestCppGen)
#include "tst_cppgen.moc"
