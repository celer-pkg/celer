// Copyright (C) 2016 The Qt Company Ltd.
// Copyright (C) 2016 Intel Corporation.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <qdebug.h>
#include <QTest>

#include <qglobal.h>

#include <q20utility.h>

#ifdef Q_OS_WIN
#include <qt_windows.h>
#endif

class tst_QGetPutEnv : public QObject
{
Q_OBJECT
private slots:
    void init();

    void getSetCheck();
    void encoding();
    void intValue_data();
    void intValue();

private:
    QByteArray uniqueEnvVarName;
};

void tst_QGetPutEnv::init()
{
    QUuid uuid = QUuid::createUuid();
    uniqueEnvVarName = "QT_TEST_ENV_VAR_" + uuid.toByteArray(QUuid::Id128);
}

void tst_QGetPutEnv::getSetCheck()
{
    const char *varName = uniqueEnvVarName.constData();

    bool ok;

    QVERIFY(!qEnvironmentVariableIsSet(varName));
    QVERIFY(qEnvironmentVariableIsEmpty(varName));
    QCOMPARE(qEnvironmentVariableIntegerValue(varName), std::nullopt);
    ok = true;
    QCOMPARE(qEnvironmentVariableIntValue(varName), 0);
    QCOMPARE(qEnvironmentVariableIntValue(varName, &ok), 0);
    QVERIFY(!ok);
    QByteArray result = qgetenv(varName);
    QVERIFY(result.isNull());
    QString sresult = qEnvironmentVariable(varName);
    QVERIFY(sresult.isNull());
    sresult = qEnvironmentVariable(varName, "hello");
    QCOMPARE(sresult, QString("hello"));

#ifndef Q_OS_WIN
    QVERIFY(qputenv(varName, "")); // deletes varName instead of making it empty, on Windows

    QVERIFY(qEnvironmentVariableIsSet(varName));
    QVERIFY(qEnvironmentVariableIsEmpty(varName));
    QCOMPARE(qEnvironmentVariableIntegerValue(varName), std::nullopt);
    ok = true;
    QCOMPARE(qEnvironmentVariableIntValue(varName), 0);
    QCOMPARE(qEnvironmentVariableIntValue(varName, &ok), 0);
    QVERIFY(!ok);

    result = qgetenv(varName);
    QVERIFY(!result.isNull());
    QCOMPARE(result, QByteArray());
    sresult = qEnvironmentVariable(varName);
    QVERIFY(!sresult.isNull());
    QCOMPARE(sresult, QString());
    sresult = qEnvironmentVariable(varName, "hello");
    QVERIFY(!sresult.isNull());
    QCOMPARE(sresult, QString());
#endif

    constexpr char varValueFullString[] = "supervalue123";
    const auto varValueQBA = QByteArray::fromRawData(varValueFullString, sizeof varValueFullString - 4);
    QCOMPARE_EQ(varValueQBA, "supervalue");

    QVERIFY(qputenv(varName, varValueQBA));

    QVERIFY(qEnvironmentVariableIsSet(varName));
    QVERIFY(!qEnvironmentVariableIsEmpty(varName));
    QCOMPARE(qEnvironmentVariableIntegerValue(varName), std::nullopt);
    ok = true;
    QCOMPARE(qEnvironmentVariableIntValue(varName), 0);
    QCOMPARE(qEnvironmentVariableIntValue(varName, &ok), 0);
    QVERIFY(!ok);
    result = qgetenv(varName);
    QCOMPARE(result, QByteArrayLiteral("supervalue"));
    sresult = qEnvironmentVariable(varName);
    QCOMPARE(sresult, QString("supervalue"));
    sresult = qEnvironmentVariable(varName, "hello");
    QCOMPARE(sresult, QString("supervalue"));

    qputenv(varName,QByteArray());

    // Now test qunsetenv
    QVERIFY(qunsetenv(varName));
    QVERIFY(!qEnvironmentVariableIsSet(varName)); // note: might fail on some systems!
    QVERIFY(qEnvironmentVariableIsEmpty(varName));
    QCOMPARE(qEnvironmentVariableIntegerValue(varName), std::nullopt);
    ok = true;
    QCOMPARE(qEnvironmentVariableIntValue(varName), 0);
    QCOMPARE(qEnvironmentVariableIntValue(varName, &ok), 0);
    QVERIFY(!ok);

    result = qgetenv(varName);
    QVERIFY(result.isNull());
    sresult = qEnvironmentVariable(varName);
    QVERIFY(sresult.isNull());
    sresult = qEnvironmentVariable(varName, "hello");
    QCOMPARE(sresult, QString("hello"));
}

void tst_QGetPutEnv::encoding()
{
    // The test string is:
    //  U+0061      LATIN SMALL LETTER A
    //  U+00E1      LATIN SMALL LETTER A WITH ACUTE
    //  U+03B1      GREEK SMALL LETTER ALPHA
    //  U+0430      CYRILLIC SMALL LETTER A
    // This has letters in three different scripts, so no locale besides
    // UTF-8 is able handle them all.
    // The LATIN SMALL LETTER A WITH ACUTE is NFC for NFD:
    //  U+0061 U+0301   LATIN SMALL LETTER A + COMBINING ACUTE ACCENT

    const char *varName = uniqueEnvVarName.constData();

    static const wchar_t rawvalue[] = { 'a', 0x00E1, 0x03B1, 0x0430, 0 };
    QString value = QString::fromWCharArray(rawvalue);

#if defined(Q_OS_WIN)
    std::wstring wvarName = QString::fromUtf8(varName).toStdWString();
    _wputenv_s(wvarName.data(), rawvalue);
#else
    // confirm the locale is UTF-8
    if (value.toLocal8Bit() != "a\xc3\xa1\xce\xb1\xd0\xb0")
        QSKIP("Locale is not UTF-8, cannot test");

    qputenv(varName, QFile::encodeName(value));
#endif

    QVERIFY(qEnvironmentVariableIsSet(varName));
    QCOMPARE(qEnvironmentVariable(varName), value);
}

void tst_QGetPutEnv::intValue_data()
{
    QTest::addColumn<QByteArray>("value");
    QTest::addColumn<qint64>("expected");
    QTest::addColumn<bool>("ok");

    // some repetition from what is tested in getSetCheck()
    QTest::newRow("empty") << QByteArray() << qint64(0) << false;
    QTest::newRow("spaces-heading") << QByteArray(" \n\r\t1") << qint64(1) << true;
    QTest::newRow("spaces-trailing") << QByteArray("1 \n\r\t") << qint64(1) << true;
    QTest::newRow("junk-heading") << QByteArray("x1") << qint64(0) << false;
    QTest::newRow("junk-trailing") << QByteArray("1x") << qint64(0) << false;

    auto addRow = [](const char *text, qint64 expected, bool ok) {
        QTest::newRow(text) << QByteArray(text) << expected << ok;
    };
    addRow("auto", 0, false);
    addRow("1auto", 0, false);
    addRow("0", 0, true);
    addRow("+0", 0, true);
    addRow("1", 1, true);
    addRow("+1", 1, true);
    addRow("09", 0, false);
    addRow("010", 8, true);
    addRow("0x10", 16, true);
    addRow("0x", 0, false);
    addRow("0xg", 0, false);
    addRow("0x1g", 0, false);
    addRow("000000000000000000000000000000000000000000000000001", 1, true);
    addRow("+000000000000000000000000000000000000000000000000001", 1, true);
    addRow("000000000000000000000000000000000000000000000000001g", 0, false);
    addRow("-0", 0, true);
    addRow("-1", -1, true);
    addRow("-010", -8, true);
    addRow("-000000000000000000000000000000000000000000000000001", -1, true);

    auto addNumWithBase = [](auto num, int base) {
        QByteArray text;
        {
            QTextStream s(&text);
            s.setIntegerBase(base);
            s << Qt::showbase << num;
        }
        QTestData &row = QTest::addRow("%s", text.constData()) << text;
        bool ok = true;
        if constexpr (std::is_same_v<decltype(num), quint64>)
            ok = num <= quint64(LLONG_MAX);
        if (ok)
            row << qint64(num) << true;
        else
            row << qint64(0) << false;
    };
    for (int base : {10, 8, 16}) {
        addNumWithBase(INT_MAX, base);
        addNumWithBase(qlonglong(INT_MAX) + 1, base);
        addNumWithBase(UINT_MAX, base);
        addNumWithBase(INT_MIN, base);
        addNumWithBase(qlonglong(INT_MIN) - 1 , base);
        addNumWithBase(LLONG_MAX, base);
        addNumWithBase(LLONG_MIN, base);
        addNumWithBase(ULLONG_MAX, base);
    };
}

void tst_QGetPutEnv::intValue()
{
    const int maxlen = (sizeof(qint64) * CHAR_BIT + 2) / 3;
    const char *varName = uniqueEnvVarName.constData();

    QFETCH(QByteArray, value);
    QFETCH(qint64, expected);
    QFETCH(bool, ok);

    bool actualOk = !ok;
    bool is32Bit = q20::in_range<int>(expected);

    // Self-test: confirm that it was like the docs said it should be
    if (value.size() < maxlen) {
        // 32-bit
        QCOMPARE(value.toInt(&actualOk, 0), is32Bit ? expected : 0);
        QCOMPARE(actualOk, ok && is32Bit);
        QCOMPARE(value.toLongLong(&actualOk, 0), expected);
        QCOMPARE(actualOk, ok);
    }

    actualOk = !ok;
    QVERIFY(qputenv(varName, value));
    QCOMPARE(qEnvironmentVariableIntValue(varName), is32Bit ? expected : 0);
    QCOMPARE(qEnvironmentVariableIntValue(varName, &actualOk), is32Bit ? expected : 0);
    QCOMPARE(actualOk, ok && is32Bit);
    if (ok)
        QCOMPARE(qEnvironmentVariableIntegerValue(varName), expected);
    else
        QCOMPARE(qEnvironmentVariableIntegerValue(varName), std::nullopt);
}

QTEST_MAIN(tst_QGetPutEnv)
#include "tst_qgetputenv.moc"
