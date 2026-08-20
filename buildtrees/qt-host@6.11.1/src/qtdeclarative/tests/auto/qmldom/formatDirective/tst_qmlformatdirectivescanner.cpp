// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtTest/QTest>
#include <QtQmlDom/private/qqmldomformatdirectivescanner_p.h>
#include <private/qqmljslexer_p.h>
#include <private/qqmljsparser_p.h>

using namespace QmlFormat;

class tst_qmlformatdirectivescanner : public QObject
{
    Q_OBJECT
private slots:
    void directiveFromComment_data();
    void directiveFromComment();

    void parseComments_data();
    void parseComments();
};

void tst_qmlformatdirectivescanner::directiveFromComment_data()
{
    QTest::addColumn<QString>("commentLine");
    QTest::addColumn<std::optional<Directive>>("expectedDirective");

    QTest::newRow("valid-off") << "// qmlformat off"
                               << std::make_optional(Directive::Off);
    QTest::newRow("valid-on") << "// qmlformat on"
                              << std::make_optional(Directive::On);
    QTest::newRow("invalid-missing-cmd")
            << "// qmlformat" << std::optional<Directive>();
    QTest::newRow("invalid-missing-qmlformat")
            << "// format on" << std::optional<Directive>();
    QTest::newRow("invalid-empty") << "//" << std::optional<Directive>();
    QTest::newRow("invalid-random-text")
            << "// some random text" << std::optional<Directive>();
    QTest::newRow("invalid-inline") << "import QtQuick // qmlformat off"
                                    << std::optional<Directive>();
}

void tst_qmlformatdirectivescanner::directiveFromComment()
{
    QFETCH(QString, commentLine);
    QFETCH(std::optional<Directive>, expectedDirective);

    const auto directive = ::directiveFromComment(commentLine);
    QCOMPARE(directive, expectedDirective);
}

void tst_qmlformatdirectivescanner::parseComments_data()
{
    QTest::addColumn<QString>("qmlCode");
    QTest::addColumn<qsizetype>("expectedDisabledRegionCount");
    QTest::addColumn<QStringList>("expectedDisabledCodes");

    {
        // Test a basic QML file with a single qmlformat off directive
        const QString qmlCode = ""
                                "// qmlformat off\n"
                                "import QtQuick\n"
                                "Item {\n"
                                "    property int x: 42\n"
                                " // qmlformat on\n\n\n"
                                "}\n";

        const QString expectedDisabledCode = "// qmlformat off\n"
                                             "import QtQuick\n"
                                             "Item {\n"
                                             "    property int x: 42\n"
                                             " // qmlformat on\n\n";
        QTest::newRow("basic") << qmlCode << qsizetype(1) << QStringList{ expectedDisabledCode };
    }
    {
        const QString qmlCode = ""
                                "// qmlformat off\n\n\n"
                                "import QtQuick\n"
                                "Item {\n"
                                "    property int x: 42 \n\n"
                                "// qmlformat on\n"
                                "    property int y: 24\n"
                                "// qmlformat off\n"
                                "}\n";

        const QString expectedDisabledCodeFirst = "// qmlformat off\n\n\n"
                                                  "import QtQuick\n"
                                                  "Item {\n"
                                                  "    property int x: 42 \n\n"
                                                  "// qmlformat on\n";
        QTest::newRow("multiple") << qmlCode << qsizetype(2)
                                  << QStringList{ expectedDisabledCodeFirst,
                                                  "// qmlformat off\n}\n" };
    }
    {
        const QString qmlCode = ""
                                "// qmlformat on\n"
                                "import QtQuick\n"
                                "Item {\n"
                                "    property int x: 42\n"
                                "    property int y: 24\n"
                                "}\n";

        QTest::newRow("only-on") << qmlCode << qsizetype(0) << QStringList{};
    }
    {
        const QString qmlCode = ""
                                "// qmlformat off\n"
                                "import QtQuick\n"
                                "Item {\n"
                                "    property int x: 42\n"
                                "    property int y: 24\n"
                                "}\n";

        QTest::newRow("only-off") << qmlCode << qsizetype(1) << QStringList{ qmlCode };
    }
    {
        const QString qmlCode = ""
                                "import QtQuick // qmlformat off\n"
                                "Item {\n"
                                "    property int x: 42\n"
                                "    property int y: 24\n"
                                "}\n";

        QTest::newRow("inline-directive") << qmlCode << qsizetype(0) << QStringList{};
    }
    {
        const QString qmlCode = ""
                                "// qmlformat off\n"
                                "import QtQuick \n"
                                "Item {\n"
                                "    property int x: 42\n"
                                "    property int y: 24\n"
                                "}\n"
                                "// qmlformat on";

        QTest::newRow("eof") << qmlCode << qsizetype(1) << QStringList{ qmlCode };
    }
    {
        const QString expectedCode = ""
                                "// qmlformat off\n"
                                "import QtQuick // qmlformat off\n"
                                "// qmlformat off\n"
                                "Item {\n"
                                "    property int x: 42\n"
                                "    property int y: 24\n"
                                "}\n"
                                "// qmlformat on\n";
        const QString qmlCode = expectedCode + "// qmlformat on";

        QTest::newRow("nested") << qmlCode << qsizetype(1) << QStringList{ expectedCode };
    }
    {
        const QString qmlCodeWinNewline = ""
                                          "// qmlformat off\r\n"
                                          "import QtQuick\r\n"
                                          "// qmlformat on\r\n"
                                          "Item{}\r\n";
        const QString expectedDisabledCode = ""
                                             "// qmlformat off\r\n"
                                             "import QtQuick\r\n"
                                             "// qmlformat on\r\n";
        QTest::newRow("winNewLine") << qmlCodeWinNewline << qsizetype(1)
                                << QStringList{ expectedDisabledCode };
    }
}

void tst_qmlformatdirectivescanner::parseComments()
{
    QFETCH(QString, qmlCode);
    QFETCH(qsizetype, expectedDisabledRegionCount);
    QFETCH(QStringList, expectedDisabledCodes);

    QCOMPARE(expectedDisabledRegionCount, expectedDisabledCodes.size());

    QQmlJS::Engine engine;
    QQmlJS::Lexer lexer(&engine);
    lexer.setCode(qmlCode, 1);

    QQmlJS::Parser parser(&engine);
    QVERIFY(parser.parse());

    auto disabledRegions = ::identifyDisabledRegions(qmlCode, engine.comments());
    QCOMPARE(qsizetype(disabledRegions.size()), expectedDisabledRegionCount);
    int i = -1;
    for (const auto &loc : disabledRegions.values()) {
        const auto disabledCode = qmlCode.mid(loc.offset, loc.length);
        QCOMPARE(disabledCode, expectedDisabledCodes.at(++i));
    }
}

#ifndef NO_QTEST_MAIN
QTEST_MAIN(tst_qmlformatdirectivescanner)
#endif

#include "tst_qmlformatdirectivescanner.moc"
