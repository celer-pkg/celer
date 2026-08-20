// Copyright (C) 2018 Witekio.
// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtTest>

#include <QtCoap/qcoapoption.h>

class tst_QCoapOption : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void constructAndAssign();
    void constructWithQByteArray();
    void constructWithQString();
    void constructWithInteger();
    void constructWithUtf8Characters();
};

void tst_QCoapOption::constructAndAssign()
{
    QCoapOption option1;
    QCOMPARE(option1.name(), QCoapOption::Invalid);
    QCOMPARE(option1.uintValue(), 0u);
    QVERIFY(option1.stringValue().isEmpty());
    QVERIFY(option1.opaqueValue().isEmpty());

    QCoapOption option2(QCoapOption::Size1, 1);
    QCOMPARE(option2.name(), QCoapOption::Size1);
    QCOMPARE(option2.uintValue(), 1u);

    // Copy-construction
    QCoapOption option3(option2);
    QCOMPARE(option3.name(), QCoapOption::Size1);
    QCOMPARE(option3.uintValue(), 1u);

    // Move-construction
    QCoapOption option4(std::move(option2));
    QCOMPARE(option4.name(), QCoapOption::Size1);
    QCOMPARE(option4.uintValue(), 1u);

    // Copy-assignment
    option4 = option1;
    QCOMPARE(option4.name(), QCoapOption::Invalid);
    QCOMPARE(option4.uintValue(), 0u);

    // Move-assignment
    option4 = std::move(option3);
    QCOMPARE(option4.name(), QCoapOption::Size1);
    QCOMPARE(option4.uintValue(), 1u);

    // Assign to a moved-from
    option2 = option4;
    QCOMPARE(option2.name(), QCoapOption::Size1);
    QCOMPARE(option2.uintValue(), 1u);
}

void tst_QCoapOption::constructWithQByteArray()
{
    QByteArray ba = "some data";
    QCoapOption option(QCoapOption::LocationPath, ba);

    QCOMPARE(option.opaqueValue(), ba);
}

void tst_QCoapOption::constructWithQString()
{
    QString str = "some data";
    QCoapOption option(QCoapOption::LocationPath, str);

    QCOMPARE(option.opaqueValue(), str.toUtf8());
}

void tst_QCoapOption::constructWithInteger()
{
    quint32 value = 64000;
    QCoapOption option(QCoapOption::Size1, value);

    QCOMPARE(option.uintValue(), value);
}

void tst_QCoapOption::constructWithUtf8Characters()
{
    QByteArray ba = "\xc3\xa9~\xce\xbb\xe2\x82\xb2";
    QCoapOption option(QCoapOption::LocationPath, ba);

    QCOMPARE(option.opaqueValue(), ba);
}

QTEST_APPLESS_MAIN(tst_QCoapOption)

#include "tst_qcoapoption.moc"
