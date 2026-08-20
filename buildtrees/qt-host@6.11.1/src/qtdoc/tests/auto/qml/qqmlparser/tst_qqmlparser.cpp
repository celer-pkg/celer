// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <private/qqmljsengine_p.h>
#include <private/qqmljsparser_p.h>
#include <private/qqmljslexer_p.h>
#include <private/qqmljsastvisitor_p.h>
#include <private/qqmljsast_p.h>

#include <qtest.h>
#include <QDir>
#include <QDebug>
#include <cstdlib>

class tst_qqmlparser : public QObject
{
    Q_OBJECT
public:
    tst_qqmlparser();

private slots:
    void initTestCase();
#if !defined(QTEST_CROSS_COMPILED) // sources not available when cross compiled
    void qmlParser_data();
    void qmlParser();
#endif
    void invalidEscapeSequence();
    void stringLiteral();
    void noSubstitutionTemplateLiteral();
    void templateLiteral();

private:
    QStringList excludedDirs;

    QStringList findFiles(const QDir &);
};

namespace check {

using namespace QQmlJS;

class Check: public AST::Visitor
{
    QList<AST::Node *> nodeStack;

public:
    void operator()(AST::Node *node)
    {
        AST::Node::accept(node, this);
    }

    void checkNode(AST::Node *node)
    {
        if (! nodeStack.isEmpty()) {
            AST::Node *parent = nodeStack.last();
            const quint32 parentBegin = parent->firstSourceLocation().begin();
            const quint32 parentEnd = parent->lastSourceLocation().end();

            if (node->firstSourceLocation().begin() < parentBegin)
                qDebug() << "first source loc failed: node:" << node->kind << "at" << node->firstSourceLocation().startLine << "/" << node->firstSourceLocation().startColumn
                         << "parent" << parent->kind << "at" << parent->firstSourceLocation().startLine << "/" << parent->firstSourceLocation().startColumn;
            if (node->lastSourceLocation().end() > parentEnd)
                qDebug() << "first source loc failed: node:" << node->kind << "at" << node->lastSourceLocation().startLine << "/" << node->lastSourceLocation().startColumn
                         << "parent" << parent->kind << "at" << parent->lastSourceLocation().startLine << "/" << parent->lastSourceLocation().startColumn;

            QVERIFY(node->firstSourceLocation().begin() >= parentBegin);
            QVERIFY(node->lastSourceLocation().end() <= parentEnd);
        }
    }

    bool preVisit(AST::Node *node) override
    {
        checkNode(node);
        nodeStack.append(node);
        return true;
    }

    void postVisit(AST::Node *) override
    {
        nodeStack.removeLast();
    }

    void throwRecursionDepthError() override
    {
        QFAIL("Maximum statement or expression depth exceeded");
    }
};

}

tst_qqmlparser::tst_qqmlparser()
{
}

void tst_qqmlparser::initTestCase()
{
    // Add directories you want excluded here:
    // excludedDirs << "exclude/this/dir";
}

QStringList tst_qqmlparser::findFiles(const QDir &d)
{
    for (int ii = 0; ii < excludedDirs.size(); ++ii) {
        QString s = excludedDirs.at(ii);
        if (d.absolutePath().endsWith(s))
            return QStringList();
    }

    QStringList rv;

    QStringList files = d.entryList(QStringList() << QLatin1String("*.qml") << QLatin1String("*.js"),
                                    QDir::Files);
    for (const auto &file : files)
        rv << d.absoluteFilePath(file);

    QStringList dirs = d.entryList(QDir::Dirs | QDir::NoDotAndDotDot |
                                   QDir::NoSymLinks);
    for (const auto &dir : dirs) {
        QDir sub = d;
        sub.cd(dir);
        rv << findFiles(sub);
    }

    return rv;
}

/*
This test checks all the qml and js files in the QtQml UI source tree
and ensures that the subnode's source locations are inside parent node's source locations
*/

#if !defined(QTEST_CROSS_COMPILED) // sources not available when cross compiled
void tst_qqmlparser::qmlParser_data()
{
    QTest::addColumn<QString>("file");

    QString examples = QLatin1String(SRCDIR) + "/../../../../examples/";
    QString tests = QLatin1String(SRCDIR) + "/../../../../tests/";

    QStringList files;
    files << findFiles(QDir(examples));
    files << findFiles(QDir(tests));

    for (const auto &file : files)
        QTest::newRow(qPrintable(file)) << file;
}
#endif

#if !defined(QTEST_CROSS_COMPILED) // sources not available when cross compiled
void tst_qqmlparser::qmlParser()
{
    QFETCH(QString, file);

    using namespace QQmlJS;

    QString code;

    QFile f(file);
    if (f.open(QFile::ReadOnly))
        code = QString::fromUtf8(f.readAll());

    const bool qmlMode = file.endsWith(QLatin1String(".qml"));

    Engine engine;
    Lexer lexer(&engine);
    lexer.setCode(code, 1, qmlMode);
    Parser parser(&engine);
    bool ok = qmlMode ? parser.parse() : parser.parseProgram();

    if (ok) {
        check::Check chk;
        chk(parser.rootNode());
    }
}
#endif

void tst_qqmlparser::invalidEscapeSequence()
{
    using namespace QQmlJS;

    Engine engine;
    Lexer lexer(&engine);
    lexer.setCode(QLatin1String("\"\\"), 1);
    Parser parser(&engine);
    parser.parse();
}

void tst_qqmlparser::stringLiteral()
{
    using namespace QQmlJS;

    Engine engine;
    Lexer lexer(&engine);
    QLatin1String code("'hello string'");
    lexer.setCode(code , 1);
    Parser parser(&engine);
    QVERIFY(parser.parseExpression());
    AST::ExpressionNode *expression = parser.expression();
    QVERIFY(expression);
    auto *literal = QQmlJS::AST::cast<QQmlJS::AST::StringLiteral *>(expression);
    QVERIFY(literal);
    QCOMPARE(literal->value, u"hello string");
    QCOMPARE(literal->firstSourceLocation().begin(), 0u);
    QCOMPARE(literal->lastSourceLocation().end(), quint32(code.size()));
}

void tst_qqmlparser::noSubstitutionTemplateLiteral()
{
    using namespace QQmlJS;

    Engine engine;
    Lexer lexer(&engine);
    QLatin1String code("`hello template`");
    lexer.setCode(code, 1);
    Parser parser(&engine);
    QVERIFY(parser.parseExpression());
    AST::ExpressionNode *expression = parser.expression();
    QVERIFY(expression);

    auto *literal = QQmlJS::AST::cast<QQmlJS::AST::TemplateLiteral *>(expression);
    QVERIFY(literal);

    QCOMPARE(literal->value, u"hello template");
    QCOMPARE(literal->firstSourceLocation().begin(), 0u);
    QCOMPARE(literal->lastSourceLocation().end(), quint32(code.size()));
}

void tst_qqmlparser::templateLiteral()
{
    using namespace QQmlJS;

    Engine engine;
    Lexer lexer(&engine);
    QLatin1String code("`one plus one equals ${1+1}!`");
    lexer.setCode(code, 1);
    Parser parser(&engine);
    QVERIFY(parser.parseExpression());
    AST::ExpressionNode *expression = parser.expression();
    QVERIFY(expression);

    auto *templateLiteral = QQmlJS::AST::cast<QQmlJS::AST::TemplateLiteral *>(expression);
    QVERIFY(templateLiteral);

    QCOMPARE(templateLiteral->firstSourceLocation().begin(), 0u);
    auto *e = templateLiteral->expression;
    QVERIFY(e);
}

QTEST_MAIN(tst_qqmlparser)

#include "tst_qqmlparser.moc"
