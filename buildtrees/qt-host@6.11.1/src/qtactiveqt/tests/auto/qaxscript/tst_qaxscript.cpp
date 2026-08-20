// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only


#include <QtTest/QtTest>
#include <QAxScriptManager>
#include <QAxScript>

using namespace Qt::StringLiterals;

class tst_QAxScript : public QObject
{
    Q_OBJECT

private slots:
    void call();
    void scriptReturnValue();
    void scriptOutParameters();
    void error_data();
    void error();
};

void tst_QAxScript::call()
{
    QAxScriptManager scriptManager;
    const auto scriptCode_js = uR"JS(
    function test1() {
        return 'JScript';
    }
    )JS"_s;
    const auto scriptCode_vb = uR"VB(
    Function test2()
        test2 = "VBScript"
    End Function
    )VB"_s;
    QAxScript *jsscript = scriptManager.load(scriptCode_js, u"JS"_s, u"JScript"_s);
    QVERIFY2(jsscript, "Unable to load script (CoInitializeEx() called?)");
    QVERIFY(jsscript->scriptEngine()->hasIntrospection());
    QAxScript *vbscript = scriptManager.load(scriptCode_vb, u"VB"_s, u"VBScript"_s);
    QVERIFY2(vbscript, "Unable to load script (CoInitializeEx() called?)");
    QVERIFY(vbscript->scriptEngine()->hasIntrospection());

    QCOMPARE(jsscript->call("test1()"), "JScript");
    QTest::ignoreMessage(QtWarningMsg, "QAxBase::dynamicCallHelper: test1(): No such method in  [unknown]");
    QTest::ignoreMessage(QtWarningMsg, "\tCandidates are:");
    QCOMPARE(vbscript->call("test1()"), QVariant());
    QCOMPARE(vbscript->call("test2()"), "VBScript");
    QTest::ignoreMessage(QtWarningMsg, "QAxBase::dynamicCallHelper: test2(): No such method in  [unknown]");
    QTest::ignoreMessage(QtWarningMsg, "\tCandidates are:");
    QCOMPARE(jsscript->call("test2()"), QVariant());
}

void tst_QAxScript::scriptReturnValue()
{
    QAxScriptManager scriptManager;
    const auto scriptCode = uR"JS(
    function foo() {
        return 'test';
    }
    )JS"_s;  // QTBUG-42289, fails when DISPATCH_PROPERTYGET is used.
    QAxScript *script = scriptManager.load(scriptCode, u"Test"_s, u"JScript"_s);
    QVERIFY2(script, "Unable to load script (CoInitializeEx() called?)");
    const QVariant result = script->call("foo()");
    QCOMPARE(result, QVariant(u"test"_s));
}

void tst_QAxScript::scriptOutParameters()
{
    QAxScriptManager scriptManager;
    const auto scriptCode = uR"VB(
    Function GetProductName(ByRef manufacturer, ByRef name, ByRef version)
        manufacturer = "The Qt Company"
        name = "ActiveQt"
        version = 650
        On Error Resume Next
        GetProductName = 42
    End Function
    )VB"_s;

    QAxScript *script = scriptManager.load(scriptCode, u"Test"_s, u"VBScript"_s);
    QVERIFY2(script, "Unable to load script (CoInitializeEx() called?)");

    QVariant returnValue;
    QList<QVariant> results = {{}, {}, {}};

    returnValue = script->scriptEngine()->dynamicCall("GetProductName(QVariant&,QVariant&,QVariant&)", results);
    QCOMPARE(returnValue, 42);
    QCOMPARE(results.size(), 3);
    QCOMPARE(results.at(0), "The Qt Company");
    QCOMPARE(results.at(1), "ActiveQt");
    QCOMPARE(results.at(2), 650);

    results = {{}, {}, {}};
    returnValue = script->call("GetProductName(QVariant&,QVariant&,QVariant&)", results);
    QCOMPARE(returnValue, 42);
    QCOMPARE(results.size(), 3);
    QCOMPARE(results.at(0), "The Qt Company");
    QCOMPARE(results.at(1), "ActiveQt");
    QCOMPARE(results.at(2), 650);
}

void tst_QAxScript::error_data()
{
    QTest::addColumn<QString>("scriptCode");
    QTest::addColumn<QString>("language");
    QTest::addColumn<int>("expectedErrorCode");
    QTest::addColumn<QString>("expectedDescription");
    QTest::addColumn<int>("expectedSourcePosition");
    QTest::addColumn<QString>("expectedSourceText");

    QTest::addRow("JS syntax error")
        << "function foo {}"
        << "JScript"
        << 0 << "Expected '('" << 0 << "function foo {}";

    QTest::addRow("VB syntax error")
        << "Funktion test"
        << "VBScript"
        << 0 << "Type mismatch: 'Funktion'" << 0 << "";
}

void tst_QAxScript::error()
{
    QFETCH(QString, scriptCode);
    QFETCH(QString, language);
    QFETCH(int, expectedErrorCode);
    QFETCH(QString, expectedDescription);
    QFETCH(int, expectedSourcePosition);
    QFETCH(QString, expectedSourceText);

    QAxScriptManager scriptManager;
    QAxScript script(u"Test"_s, &scriptManager);

    QAxScript *actualScript = nullptr;
    int actualCode;
    QString actualDescription;
    int actualSourcePosition;
    QString actualSourceText;

    connect(&scriptManager, &QAxScriptManager::error,
            this, [&](QAxScript *script){
        actualScript = script;
    });

    connect(&script, &QAxScript::error,
            this, [&](int code, const QString &description, int sourcePosition, const QString &sourceText){
        actualCode = code;
        actualDescription = description;
        actualSourcePosition = sourcePosition;
        actualSourceText = sourceText.trimmed();
    });

    script.load(scriptCode, language);

    QCOMPARE(actualScript, &script);
    QCOMPARE(actualCode, expectedErrorCode);
    QCOMPARE(actualDescription, expectedDescription);
    QCOMPARE(actualSourcePosition, expectedSourcePosition);
    QCOMPARE(actualSourceText, expectedSourceText);
}

QTEST_MAIN(tst_QAxScript)
#include "tst_qaxscript.moc"
