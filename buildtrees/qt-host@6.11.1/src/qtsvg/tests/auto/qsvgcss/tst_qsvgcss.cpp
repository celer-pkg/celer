// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only


#include <QtTest/QTest>

#include <QtSvg/private/qsvgcsshandler_p.h>
#include <QtCore/QXmlStreamAttributes>
#include <QtGui/private/qcssparser_p.h>

using namespace Qt::Literals::StringLiterals;

class tst_QSvgCss : public QObject
{
Q_OBJECT

public:
    tst_QSvgCss();
    virtual ~tst_QSvgCss();

private slots:
    void testParsedDeclarations_data();
    void testParsedDeclarations();
};

tst_QSvgCss::tst_QSvgCss()
{
}

tst_QSvgCss::~tst_QSvgCss()
{
}

void tst_QSvgCss::testParsedDeclarations_data()
{
    QTest::addColumn<QString>("css");
    QTest::addColumn<QString>("expectedProperty");
    QTest::addColumn<QString>("expectedValue");

    QTest::newRow("value") << u"fill: red;"_s << u"fill"_s << u"red"_s;
    QTest::newRow("url") << u"fill: url(#id1);"_s << u"fill"_s << u"url(#id1)"_s;
    QTest::newRow("function") << u"transform: translate(50px, 50px)"_s << u"transform"_s << u"translate(50px, 50px)"_s;
    QTest::newRow("list of numbers") << u"stroke-dasharray: 2, 5, 10, 3;"_s << u"stroke-dasharray"_s << u"2 ; 5 ; 10 ; 3"_s;
    QTest::newRow("known identifier auto") << u"offset-rotate: auto"_s << u"offset-rotate"_s << u"auto"_s;
    QTest::newRow("known identifier none") << u"display: none"_s << u"display"_s << u"none"_s;
    QTest::newRow("multiple values 1") << u"animation: transformAnim 2000ms infinite"_s << u"animation"_s << u"transformAnim 2000ms infinite"_s;
    QTest::newRow("multiple values percentage") << u"transform-origin: 50% 25%"_s << u"transform-origin"_s << u"50% 25%"_s;
    QTest::newRow("multiple values 2") << u"offset-rotate: auto 90deg"_s << u"offset-rotate"_s << u"auto 90deg"_s;
    QTest::newRow("multiple values 3") << u"offset-rotate: reverse auto"_s << u"offset-rotate"_s << u"reverse auto"_s;
    QTest::newRow("multiple urls") << u"fill: url(#id1) url(#id2)"_s << u"fill"_s << u"url(#id1) url(#id2)"_s;
}

void tst_QSvgCss::testParsedDeclarations()
{
    QFETCH(QString, css);
    QFETCH(QString, expectedProperty);
    QFETCH(QString, expectedValue);

    css.prepend("dummy {");
    css.append("}");
    QCss::Parser parser(css);

    QCss::StyleSheet sheet;
    parser.parse(&sheet);

    QCss::StyleRule rule = (!sheet.styleRules.isEmpty()) ?
                               sheet.styleRules.at(0) : *sheet.nameIndex.begin();
    QCOMPARE(rule.declarations.size(), 1);

    QList<QCss::Declaration> decls = rule.declarations;
    QSvgCssHandler handler;
    QXmlStreamAttributes attributes;
    handler.parseCSStoXMLAttrs(decls, attributes);

    QCOMPARE(attributes.size(), 1);
    QCOMPARE(attributes.first().name(), expectedProperty);
    QCOMPARE(attributes.first().value(), expectedValue);
}

QTEST_MAIN(tst_QSvgCss)
#include "tst_qsvgcss.moc"
