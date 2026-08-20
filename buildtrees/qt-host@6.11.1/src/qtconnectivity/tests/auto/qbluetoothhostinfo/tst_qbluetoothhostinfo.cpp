// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtTest/QtTest>

#include <QDebug>
#include <QString>

#include <qbluetoothhostinfo.h>

QT_USE_NAMESPACE

class tst_QBluetoothHostInfo : public QObject
{
    Q_OBJECT

public:
    tst_QBluetoothHostInfo();
    ~tst_QBluetoothHostInfo();

private slots:
    void tst_address_data();
    void tst_address();

    void tst_name_data();
    void tst_name();

    void tst_construction_data();
    void tst_construction();

    void tst_copy();

    void tst_compare_data();
    void tst_compare();
};

tst_QBluetoothHostInfo::tst_QBluetoothHostInfo()
{
}

tst_QBluetoothHostInfo::~tst_QBluetoothHostInfo()
{
}

void tst_QBluetoothHostInfo::tst_address()
{
    QFETCH(QString, addressString);

    QBluetoothAddress address(addressString);
    QVERIFY(!address.isNull());
    QCOMPARE(address.toString(), addressString);

    QBluetoothHostInfo info;
    QBluetoothAddress result = info.address();
    QVERIFY(result.isNull());
    info.setAddress(address);
    QCOMPARE(info.address().toString(), addressString);

}

void tst_QBluetoothHostInfo::tst_address_data()
{
    QTest::addColumn<QString>("addressString");

    QTest::newRow("11:22:33:44:55:66") << QString("11:22:33:44:55:66");
    QTest::newRow("AA:BB:CC:DD:EE:FF") << QString("AA:BB:CC:DD:EE:FF");
    QTest::newRow("aa:bb:cc:dd:ee:ff") << QString("AA:BB:CC:DD:EE:FF");
    QTest::newRow("FF:FF:FF:FF:FF:FF") << QString("FF:FF:FF:FF:FF:FF");
}

void tst_QBluetoothHostInfo::tst_name()
{
    QFETCH(QString, name);

    QBluetoothHostInfo info;
    QString result = info.name();
    QVERIFY(result.isNull());
    QVERIFY(result.isEmpty());

    info.setName(name);
    QCOMPARE(info.name(), name);
}

void tst_QBluetoothHostInfo::tst_name_data()
{
    QTest::addColumn<QString>("name");

    QTest::newRow("empty/default name") << QString();
    QTest::newRow("empty name") << QString("");
    QTest::newRow("ABCD") << QString("ABCD");
    QTest::newRow("Very long name") << QString("ThisIsAVeryLongNameString-abcdefghijklmnopqrstuvwxyz");
    QTest::newRow("special chars") << QString("gh\nfg i-+.,/;");
}

void tst_QBluetoothHostInfo::tst_construction_data()
{
    QTest::addColumn<QString>("btAddress");
    QTest::addColumn<QString>("name");
    QTest::addColumn<bool>("validBtAddress");

    QTest::newRow("11:22:33:44:55:66") << QString("11:22:33:44:55:66") << QString() << true;
    QTest::newRow("AA:BB:CC:DD:EE:FF") << QString("AA:BB:CC:DD:EE:FF") << QString("") << true;
    QTest::newRow("aa:bb:cc:dd:ee:ff") << QString("AA:BB:CC:DD:EE:FF") << QString("foobar") << true;
    QTest::newRow("FF:FF:FF:FF:FF:FF") << QString("FF:FF:FF:FF:FF:FF") << QString("WeUseAlongStringAsName_FFFFFFFFFFFFFFFFFFFF") << true;
    QTest::newRow("00:00:00:00:00:00") << QString("00:00:00:00:00:00") << QString("foobar2") << false;
}

void tst_QBluetoothHostInfo::tst_construction()
{
    QFETCH(QString, btAddress);
    QFETCH(QString, name);
    QFETCH(bool, validBtAddress);

    QBluetoothAddress empty;
    QVERIFY(empty.isNull());

    QBluetoothHostInfo setter;
    QBluetoothAddress addr(btAddress);
    setter.setName(name);
    setter.setAddress(addr);
    QCOMPARE(setter.name(), name);
    QCOMPARE(setter.address().toString(), btAddress);
    QCOMPARE(setter.address().isNull(), !validBtAddress);

    setter.setAddress(empty);
    QCOMPARE(setter.name(), name);
    QCOMPARE(setter.address().toString(), QString("00:00:00:00:00:00"));
    QCOMPARE(setter.address().isNull(), true);

    setter.setName(QString());
    QCOMPARE(setter.name(), QString());
    QCOMPARE(setter.address().toString(), QString("00:00:00:00:00:00"));
    QCOMPARE(setter.address().isNull(), true);

    setter.setAddress(addr);
    QCOMPARE(setter.name(), QString());
    QCOMPARE(setter.address().toString(), btAddress);
    QCOMPARE(setter.address().isNull(), !validBtAddress);
}

void tst_QBluetoothHostInfo::tst_copy()
{
    QBluetoothHostInfo original;
    original.setAddress(QBluetoothAddress("11:22:33:44:55:66"));
    original.setName(QStringLiteral("FunkyName"));

    QBluetoothHostInfo assignConstructor(original);
    QCOMPARE(assignConstructor.name(), original.name());
    QCOMPARE(assignConstructor.address(), original.address());

    QBluetoothHostInfo assignOperator;
    assignOperator = original;
    QCOMPARE(assignOperator.name(), original.name());
    QCOMPARE(assignOperator.address(), original.address());
}

void tst_QBluetoothHostInfo::tst_compare_data()
{
    QTest::addColumn<QString>("btAddress1");
    QTest::addColumn<QString>("name1");
    QTest::addColumn<QString>("btAddress2");
    QTest::addColumn<QString>("name2");
    QTest::addColumn<bool>("sameHostInfo");

    QTest::newRow("11:22:33:44:55:66 - same") << QString("11:22:33:44:55:66") << QString("same")
                                       << QString("11:22:33:44:55:66") << QString("same")
                                       << true;
    QTest::newRow("11:22:33:44:55:66 - address") << QString("11:22:33:44:55:66") << QString("same")
                                       << QString("11:22:33:44:55:77") << QString("same")
                                       << false;
    QTest::newRow("11:22:33:44:55:66 - name") << QString("11:22:33:44:55:66") << QString("same")
                                       << QString("11:22:33:44:55:66") << QString("different")
                                       << false;
    QTest::newRow("11:22:33:44:55:66 - name/address") << QString("11:22:33:44:55:66") << QString("same")
                                       << QString("11:22:33:44:55:77") << QString("different")
                                       << false;
    QTest::newRow("empty") << QString() << QString() << QString() << QString() << true;
    QTest::newRow("empty left") << QString() << QString()
                                << QString("11:22:33:44:55:66") << QString("same") << false;
    QTest::newRow("empty right") << QString("11:22:33:44:55:66") << QString("same")
                                 << QString() << QString() << false;
    QTest::newRow("00:00:00:00:00:00") << QString("00:00:00:00:00:00") << QString("foobar1")
                                       << QString("") << QString("foobar1") << true;
    QTest::newRow("00:00:00:00:00:00") << QString("00:00:00:00:00:00") << QString("foobar1")
                                       << QString("") << QString("foobar2") << false;
    QTest::newRow("00:00:00:00:00:00") << QString("00:00:00:00:00:00") << QString("")
                                       << QString("") << QString("") << true;
}

void tst_QBluetoothHostInfo::tst_compare()
{
    QFETCH(QString, btAddress1);
    QFETCH(QString, name1);
    QFETCH(QString, btAddress2);
    QFETCH(QString, name2);
    QFETCH(bool, sameHostInfo);

    QVERIFY(QBluetoothHostInfo() == QBluetoothHostInfo());

    QBluetoothHostInfo info1;
    info1.setAddress(QBluetoothAddress(btAddress1));
    info1.setName(name1);

    QBluetoothHostInfo info2;
    info2.setAddress(QBluetoothAddress(btAddress2));
    info2.setName(name2);

    QCOMPARE(info1 == info2, sameHostInfo);
    QCOMPARE(info1 != info2, !sameHostInfo);
}

QTEST_MAIN(tst_QBluetoothHostInfo)

#include "tst_qbluetoothhostinfo.moc"
