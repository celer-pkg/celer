// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only


#include <QtTest/QtTest>
#include <QAxScriptManager>
#include <QAxScript>

using namespace Qt::StringLiterals;

class tst_QAxScriptManager : public QObject
{
    Q_OBJECT

public:
    struct Script {
        QString language;
        QString name;
        QString code;
    };

private slots:
    void functions_data();
    void functions();

    void scriptNames_data();
    void scriptNames();

    void script_data();
    void script();

    void call_data();
    void call();
};

void tst_QAxScriptManager::functions_data()
{
    const auto scriptCode_js = Script{u"JScript"_s, u"test1"_s, uR"JS(
    function js1() {
        return 'JScript 1';
    }
    function js2(value) {
        return 'JScript 2';
    }
    )JS"_s};
    const auto scriptCode_vb = Script{u"VBScript"_s, u"test2"_s, uR"VB(
    Function vb1()
        vb1 = "VBScript 1"
    End Function

    Function vb2(value)
        vb2 = "VBScript 2"
    End Function
    )VB"_s};

    QTest::addColumn<QList<Script>>("scripts");
    QTest::addColumn<QStringList>("functions");
    QTest::addColumn<QStringList>("signatures");

    QTest::addRow("js")     << QList{scriptCode_js}
                            << QStringList{"js1", "js2"}
                            << QStringList{"js1()", "js2(QVariant)"};
    QTest::addRow("vb")     << QList{scriptCode_vb}
                            << QStringList{"vb1", "vb2"}
                            << QStringList{"vb1()", "vb2(QVariant)"};
    QTest::addRow("both")   << QList{scriptCode_js, scriptCode_vb}
                            << QStringList{"js1", "js2", "vb1", "vb2"}
                            << QStringList{"js1()", "js2(QVariant)", "vb1()", "vb2(QVariant)"};
}

void tst_QAxScriptManager::functions()
{
    // QStringList functions(QAxScript::FunctionFlags = QAxScript::FunctionNames) const;

    QFETCH(QList<Script>, scripts);
    QFETCH(QStringList, functions);
    QFETCH(QStringList, signatures);

    QAxScriptManager scriptManager;
    for (const auto &script : scripts) {
        QVERIFY2(scriptManager.load(script.code, script.name, script.language),
                 "Unable to load script (CoInitializeEx() called?)");
    }
    functions.sort();
    QStringList actualFunctions = scriptManager.functions();
    actualFunctions.sort();
    QCOMPARE(actualFunctions, functions);

    QStringList actualSignatures = scriptManager.functions(QAxScript::FunctionSignatures);
    actualSignatures.sort();
    QCOMPARE(actualSignatures, signatures);
}

void tst_QAxScriptManager::scriptNames_data()
{
    functions_data();
}

void tst_QAxScriptManager::scriptNames()
{
    // QStringList scriptNames() const;
    QFETCH(QList<Script>, scripts);
    QFETCH(QStringList, functions);

    QAxScriptManager scriptManager;
    QStringList expectedNames;
    for (const auto &script : scripts) {
        expectedNames << script.name;
        QVERIFY2(scriptManager.load(script.code, script.name, script.language),
                 "Unable to load script (CoInitializeEx() called?)");
    }
    expectedNames.sort();
    QStringList actualNames = scriptManager.scriptNames();
    actualNames.sort();
    QCOMPARE(actualNames, expectedNames);
}

void tst_QAxScriptManager::script_data()
{
    functions_data();
}

void tst_QAxScriptManager::script()
{
    QFETCH(QList<Script>, scripts);
    QFETCH(QStringList, functions);

    QAxScriptManager scriptManager;
    QStringList expectedNames;
    for (const auto &script : scripts) {
        expectedNames << script.name;
        QVERIFY2(scriptManager.load(script.code, script.name, script.language),
                 "Unable to load script (CoInitializeEx() called?)");
    }

    for (const auto &script : scripts) {
        QAxScript *axscript = scriptManager.script(script.name);
        QVERIFY(axscript);
        QVERIFY(axscript->scriptEngine());
        QVERIFY(axscript->scriptEngine()->isValid());
        const auto scriptFunctions = axscript->functions();
        for (const auto &scriptFunction : scriptFunctions) {
            auto index = functions.indexOf(scriptFunction);
            QCOMPARE_NE(index, -1);
            functions.remove(index);
        }

        QCOMPARE(axscript->scriptEngine()->scriptLanguage(), script.language);
    }

    // verify that all functions were found across the different QAxScript objects
    QVERIFY(functions.isEmpty());
}

void tst_QAxScriptManager::call_data()
{
    functions_data();
}

void tst_QAxScriptManager::call()
{
    QFETCH(QList<Script>, scripts);
    QFETCH(QStringList, functions);
    QFETCH(QStringList, signatures);

    QAxScriptManager scriptManager;
    for (const auto &script : scripts) {
        QAxScript *axScript = scriptManager.load(script.code, script.name, script.language);

        QVERIFY2(axScript, "Unable to load script (CoInitializeEx() called?)");
        QVERIFY(axScript->scriptEngine());
        QVERIFY(axScript->scriptEngine()->hasIntrospection());
    }

    // QAxScriptManager::call finds the script based on function name...
    for (const auto &function : std::as_const(functions)) {
        QVariant result = scriptManager.call(function);
        QCOMPARE(result.metaType(), QMetaType::fromType<QString>());
    }
    // ...or fully qualified function signature
    for (const auto &function : std::as_const(signatures)) {
        QVariant result = scriptManager.call(function);
        QCOMPARE(result.metaType(), QMetaType::fromType<QString>());
    }
}

QTEST_MAIN(tst_QAxScriptManager)
#include "tst_qaxscriptmanager.moc"
