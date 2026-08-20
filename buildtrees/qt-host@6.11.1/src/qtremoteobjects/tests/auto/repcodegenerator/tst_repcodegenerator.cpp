// Copyright (C) 2017 Ford Motor Company
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "rep_preprocessortest_merged.h"
#include "rep_classinnamespace_merged.h"

#include <QTest>

class tst_RepCodeGenerator : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void testPreprocessorTestFile();
    void testNamespaceTestFile();
};

void tst_RepCodeGenerator::testPreprocessorTestFile()
{
    // could be a compile-time test, but let's just do something here
    QVERIFY(PREPROCESSORTEST_MACRO);
}

void tst_RepCodeGenerator::testNamespaceTestFile()
{
    Test::MyNamespace::MyNamespaceClassReplica testReplica;
    Test::MyNamespace::MyNamespaceClassSimpleSource testSource;
    Test::MyNamespace::MyNamespacePod testpod;
    [[maybe_unused]]
    Test::MyNamespace::MyNamespaceEnumEnum::MyNamespaceEnum testenum =
            Test::MyNamespace::MyNamespaceEnumEnum::Value1;
}

QTEST_APPLESS_MAIN(tst_RepCodeGenerator)

#include "tst_repcodegenerator.moc"
