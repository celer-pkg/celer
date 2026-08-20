// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtTest/QtTest>

#include <QtBluetooth/qlowenergyservice.h>


/*
 * This is a very simple test despite the complexity of QLowEnergyService.
 * It mostly aims to test the static API behaviors of the class. The connection
 * orientated elements are test by the test for QLowEnergyController as it
 * is impossible to test the two classes separately from each other.
 */

class tst_QLowEnergyService : public QObject
{
    Q_OBJECT

private slots:
    void tst_flags();
};

void tst_QLowEnergyService::tst_flags()
{
    QLowEnergyService::ServiceTypes flag1(QLowEnergyService::PrimaryService);
    QLowEnergyService::ServiceTypes flag2(QLowEnergyService::IncludedService);
    QLowEnergyService::ServiceTypes result;

    // test QFlags &operator|=(QFlags f)
    result = flag1 | flag2;
    QVERIFY(result.testFlag(QLowEnergyService::PrimaryService));
    QVERIFY(result.testFlag(QLowEnergyService::IncludedService));

    // test QFlags &operator|=(Enum f)
    result = flag1 | QLowEnergyService::IncludedService;
    QVERIFY(result.testFlag(QLowEnergyService::PrimaryService));
    QVERIFY(result.testFlag(QLowEnergyService::IncludedService));

    // test Q_DECLARE_OPERATORS_FOR_FLAGS(QLowEnergyService::ServiceTypes)
    result = QLowEnergyService::IncludedService | flag1;
    QVERIFY(result.testFlag(QLowEnergyService::PrimaryService));
    QVERIFY(result.testFlag(QLowEnergyService::IncludedService));
}

QTEST_MAIN(tst_QLowEnergyService)

#include "tst_qlowenergyservice.moc"
