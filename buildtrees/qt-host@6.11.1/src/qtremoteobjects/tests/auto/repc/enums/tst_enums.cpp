// Copyright (C) 2017-2020 Ford Motor Company
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "rep_enums_replica.h"

#include <QTest>

#include <QByteArray>
#include <QDataStream>

class tst_Enums : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void testMarshalling();
    void testEnumContainingENUM();
};

void tst_Enums::testMarshalling()
{
    QByteArray ba;
    QDataStream ds(&ba, QIODevice::ReadWrite);

    {
        const Qt::DayOfWeek format1 = Qt::Monday;
        const Qt::DayOfWeek format2 = Qt::Tuesday;
        const Qt::DayOfWeek format3 = Qt::Wednesday;
        const Qt::DayOfWeek format4 = Qt::Thursday;
        const Qt::DayOfWeek format5 = Qt::Friday;
        const Qt::DayOfWeek format6 = Qt::Saturday;
        const Qt::DayOfWeek format7 = Qt::Sunday;

        ds << int(format1) << int(format2) << int(format3) << int(format4)
           << int(format5) << int(format6) << int(format7);
    }

    ds.device()->seek(0);

    {
        int format1, format2, format3, format4, format5, format6, format7;

        ds >> format1 >> format2 >> format3 >> format4 >> format5 >> format6 >> format7;

        QCOMPARE(format1, int(Qt::Monday));
        QCOMPARE(format2, int(Qt::Tuesday));
        QCOMPARE(format3, int(Qt::Wednesday));
        QCOMPARE(format4, int(Qt::Thursday));
        QCOMPARE(format5, int(Qt::Friday));
        QCOMPARE(format6, int(Qt::Saturday));
        QCOMPARE(format7, int(Qt::Sunday));
    }
}

void tst_Enums::testEnumContainingENUM()
{
    // We mostly just want to make sure the generation for this doesn't change,
    // so test using the type name and the value of the first entry:
    XENUMxEnum::XENUMx xenum = XENUMxEnum::Foo;
    QCOMPARE(int(xenum), 0);
    ENUM_Enum::ENUM_ enum_ = ENUM_Enum::Foo;
    QCOMPARE(int(enum_), 0);
}

QTEST_APPLESS_MAIN(tst_Enums)

#include "tst_enums.moc"
