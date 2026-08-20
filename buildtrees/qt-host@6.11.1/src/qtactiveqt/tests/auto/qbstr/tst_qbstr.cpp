// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtTest/QtTest>
#include <QtAxBase/private/qbstr_p.h>

QT_USE_NAMESPACE

using namespace Qt::StringLiterals;

typedef int (*SETOANOCACHE)(void);

void DisableBSTRCache()
{
    const HINSTANCE hLib = LoadLibraryA("OLEAUT32.DLL");
    if (hLib != nullptr) {
        const SETOANOCACHE SetOaNoCache =
                reinterpret_cast<SETOANOCACHE>(GetProcAddress(hLib, "SetOaNoCache"));
        if (SetOaNoCache != nullptr)
            SetOaNoCache();
        FreeLibrary(hLib);
    }
}

class tst_qbstr : public QObject
{
    Q_OBJECT
private slots:
    void initTestCase() { DisableBSTRCache(); }
    void constructor_nullInitializes()
    {
        const QBStr str;
        QVERIFY(!str.bstr());
    }

    void constructor_initializes_withLiteral()
    {
        const QBStr str{ L"hello world" };
        QCOMPARE_EQ(str.bstr(), "hello world"_L1);
    }

    void constructor_initializes_withQString()
    {
        const QString expected{ "hello world"_L1 };
        QBStr str{ expected };
        QCOMPARE_EQ(QStringView{ str.bstr() }, expected);
    }

    void copyConstructor_initializes_withNullString()
    {
        const QBStr null;
        const QBStr dest = null;
        QCOMPARE_EQ(dest.bstr(), nullptr);
    }

    void copyConstructor_initializes_withValidString()
    {
        const QBStr expected{ L"hello world" };
        const QBStr dest = expected;
        QCOMPARE_EQ(QStringView{ dest.bstr() }, expected.bstr());
    }

    void moveConstructor_initializes_withNullString()
    {
        const QBStr str{ QBStr{} };
        QCOMPARE_EQ(str.bstr(), nullptr);
    }

    void moveConstructor_initializes_withValidString()
    {
        QBStr source { L"hello world" };
        const QBStr str{ std::move(source) };
        QCOMPARE_EQ(str.bstr(), "hello world"_L1);
    }

    void assignment_assigns_withNullString()
    {
        const QBStr source{};
        const QBStr dest = source;
        QCOMPARE_EQ(dest.bstr(), nullptr);
    }

    void assignment_assigns_withValidString()
    {
        const QBStr source{ L"hello world" };
        const QBStr dest = source;
        QCOMPARE_EQ(dest.bstr(), "hello world"_L1);
    }

    void moveAssignment_assigns_withNullString()
    {
        QBStr source{};
        const QBStr dest = std::move(source);
        QCOMPARE_EQ(dest.bstr(), nullptr);
    }

    void moveAssignment_assigns_withValidString()
    {
        QBStr source{ L"hello world" };
        const QBStr dest = std::move(source);
        QCOMPARE_EQ(dest.bstr(), "hello world"_L1);
    }

    void release_returnsStringAndClearsValue()
    {
        QBStr str{ L"hello world" };
        BSTR val = str.release();
        QCOMPARE_EQ(str.bstr(), nullptr);
        QCOMPARE_EQ(val, "hello world"_L1);
        SysFreeString(val);
    }

    void str_returnsQString_withNullString()
    {
        const QBStr bstr{};
        const QString str = bstr.str();
        QVERIFY(str.isEmpty());
    }

    void str_returnsQString_withValidString()
    {
        const QBStr bstr{L"hello world"};
        const QString str = bstr.str();
        QCOMPARE_EQ(str, "hello world");
    }

    void operatorBool_returnsFalse_withNullString() {
        QVERIFY(!QBStr{});
    }

    void operatorBool_returnsTrue_withValidString()
    {
        QVERIFY(QBStr{ "hello world" });
    }
};

QTEST_MAIN(tst_qbstr)
#include "tst_qbstr.moc"
