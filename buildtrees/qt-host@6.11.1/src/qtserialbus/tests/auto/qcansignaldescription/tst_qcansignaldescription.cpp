// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtTest/qtest.h>

#include <QtSerialBus/qcansignaldescription.h>
#include <QtSerialBus/private/qcansignaldescription_p.h>

#include "qcansignaldescription_helpers.h"

QT_USE_NAMESPACE

class tst_QCanSignalDescription : public QObject
{
Q_OBJECT

private slots:
    void construct();
    void copy();
    void move();
    void comparison();
    void isValid_data();
    void isValid();
};

void tst_QCanSignalDescription::construct()
{
    QCanSignalDescription d;
    QVERIFY(!d.isValid());
    QCOMPARE(d.name(), QString());
    QCOMPARE(d.physicalUnit(), QString());
    QCOMPARE(d.receiver(), QString());
    QCOMPARE(d.comment(), QString());
    QCOMPARE(d.dataSource(), QtCanBus::DataSource::Payload);
    QCOMPARE(d.dataEndian(), QSysInfo::Endian::BigEndian);
    QCOMPARE(d.dataFormat(), QtCanBus::DataFormat::SignedInteger);
    QCOMPARE(d.startBit(), 0);
    QCOMPARE(d.bitLength(), 0);
    QCOMPARE(d.factor(), qQNaN());
    QCOMPARE(d.offset(), qQNaN());
    QCOMPARE(d.scaling(), qQNaN());
    QCOMPARE(d.minimum(), qQNaN());
    QCOMPARE(d.maximum(), qQNaN());
    QCOMPARE(d.multiplexState(), QtCanBus::MultiplexState::None);
    QVERIFY(d.multiplexSignals().isEmpty());
}

void tst_QCanSignalDescription::copy()
{
    QCanSignalDescription d;
    d.setName("test");
    d.setPhysicalUnit("km/h");
    d.setReceiver("rcv");
    d.setBitLength(16);
    QVERIFY(!QCanSignalDescriptionPrivate::get(d)->isShared());

    QCanSignalDescription d1 = d;
    QVERIFY(QCanSignalDescriptionPrivate::get(d)->isShared());
    QVERIFY(QCanSignalDescriptionPrivate::get(d1)->isShared());
    QVERIFY(equals(d1, d));

    QCanSignalDescription d2(d);
    QVERIFY(QCanSignalDescriptionPrivate::get(d)->isShared());
    QVERIFY(QCanSignalDescriptionPrivate::get(d2)->isShared());
    QVERIFY(equals(d2, d));
}

void tst_QCanSignalDescription::move()
{
    QCanSignalDescription d;
    d.setName("test");
    d.setPhysicalUnit("km/h");
    d.setReceiver("rcv");
    d.setBitLength(16);
    QVERIFY(!QCanSignalDescriptionPrivate::get(d)->isShared());

    QCanSignalDescription d1 = std::move(d);
    QVERIFY(!QCanSignalDescriptionPrivate::get(d1)->isShared());

    QCanSignalDescription d2(std::move(d1));
    QVERIFY(!QCanSignalDescriptionPrivate::get(d2)->isShared());

    d = std::move(d2);
    QVERIFY(!QCanSignalDescriptionPrivate::get(d)->isShared());
}

void tst_QCanSignalDescription::comparison()
{
    QCanSignalDescription d;
    d.setName("test");
    d.setPhysicalUnit("km/h");
    d.setReceiver("rcv");
    d.setBitLength(16);
    d.addMultiplexSignal("sig", 1);

    {
        QCanSignalDescription d1 = d;
        QVERIFY(equals(d1, d));
        // also check that calling const methods does not detach
        d1.isValid();
        d1.name();
        d1.physicalUnit();
        d1.receiver();
        d1.comment();
        d1.dataSource();
        d1.dataEndian();
        d1.dataFormat();
        d1.startBit();
        d1.bitLength();
        d1.factor();
        d1.offset();
        d1.scaling();
        d1.minimum();
        d1.maximum();
        d1.multiplexState();
        d1.multiplexSignals();
        QVERIFY(QCanSignalDescriptionPrivate::get(d)->isShared());
        QVERIFY(QCanSignalDescriptionPrivate::get(d1)->isShared());
    }
    {
        QCanSignalDescription d1 = d;
        d1.setName("test1");
        QVERIFY(!equals(d1, d));
        QVERIFY(!QCanSignalDescriptionPrivate::get(d1)->isShared());
    }
    {
        QCanSignalDescription d1 = d;
        d1.setPhysicalUnit("");
        QVERIFY(!equals(d1, d));
        QVERIFY(!QCanSignalDescriptionPrivate::get(d1)->isShared());
    }
    {
        QCanSignalDescription d1 = d;
        d1.setReceiver("");
        QVERIFY(!equals(d1, d));
        QVERIFY(!QCanSignalDescriptionPrivate::get(d1)->isShared());
    }
    {
        QCanSignalDescription d1 = d;
        d1.setComment("comment");
        QVERIFY(!equals(d1, d));
        QVERIFY(!QCanSignalDescriptionPrivate::get(d1)->isShared());
    }
    {
        QCanSignalDescription d1 = d;
        d1.setDataSource(QtCanBus::DataSource::FrameId);
        QVERIFY(!equals(d1, d));
        QVERIFY(!QCanSignalDescriptionPrivate::get(d1)->isShared());
    }
    {
        QCanSignalDescription d1 = d;
        d1.setDataEndian(QSysInfo::Endian::LittleEndian);
        QVERIFY(!equals(d1, d));
        QVERIFY(!QCanSignalDescriptionPrivate::get(d1)->isShared());
    }
    {
        QCanSignalDescription d1 = d;
        d1.setDataFormat(QtCanBus::DataFormat::Float);
        QVERIFY(!equals(d1, d));
        QVERIFY(!QCanSignalDescriptionPrivate::get(d1)->isShared());
    }
    {
        QCanSignalDescription d1 = d;
        d1.setStartBit(1);
        QVERIFY(!equals(d1, d));
        QVERIFY(!QCanSignalDescriptionPrivate::get(d1)->isShared());
    }
    {
        QCanSignalDescription d1 = d;
        d1.setBitLength(32);
        QVERIFY(!equals(d1, d));
        QVERIFY(!QCanSignalDescriptionPrivate::get(d1)->isShared());
    }
    {
        QCanSignalDescription d1 = d;
        d1.setFactor(3.0);
        QVERIFY(!equals(d1, d));
        QVERIFY(!QCanSignalDescriptionPrivate::get(d1)->isShared());
    }
    {
        QCanSignalDescription d1 = d;
        d1.setOffset(5.5);
        QVERIFY(!equals(d1, d));
        QVERIFY(!QCanSignalDescriptionPrivate::get(d1)->isShared());
    }
    {
        QCanSignalDescription d1 = d;
        d1.setScaling(0.1);
        QVERIFY(!equals(d1, d));
        QVERIFY(!QCanSignalDescriptionPrivate::get(d1)->isShared());
    }
    {
        QCanSignalDescription d1 = d;
        d1.setRange(-10.5, 10.5);
        QVERIFY(!equals(d1, d));
        QVERIFY(!QCanSignalDescriptionPrivate::get(d1)->isShared());
    }
    {
        QCanSignalDescription d1 = d;
        d1.setMultiplexState(QtCanBus::MultiplexState::SwitchAndSignal);
        QVERIFY(!equals(d1, d));
        QVERIFY(!QCanSignalDescriptionPrivate::get(d1)->isShared());
    }
    {
        QCanSignalDescription d1 = d;
        d1.clearMultiplexSignals();
        QVERIFY(!equals(d1, d));
        QVERIFY(!QCanSignalDescriptionPrivate::get(d1)->isShared());
    }
    {
        QCanSignalDescription d1 = d;
        QCanSignalDescription::MultiplexSignalValues muxValues;
        muxValues.insert("s0", QCanSignalDescription::MultiplexValues{ {1, 1} });
        d1.setMultiplexSignals(muxValues);
        QVERIFY(!equals(d1, d));
        QVERIFY(!QCanSignalDescriptionPrivate::get(d1)->isShared());
    }
    {
        QCanSignalDescription d1 = d;
        d1.addMultiplexSignal("s0", 1);
        QVERIFY(!equals(d1, d));
        QVERIFY(!QCanSignalDescriptionPrivate::get(d1)->isShared());
    }
}

void tst_QCanSignalDescription::isValid_data()
{
    QTest::addColumn<QCanSignalDescription>("signalDescription");
    QTest::addColumn<bool>("expectedResult");

    QCanSignalDescription desc;
    desc.setName("s");
    desc.setBitLength(8);

    QTest::addRow("valid") << desc << true;

    {
        QCanSignalDescription d = desc;
        d.setName("");
        QTest::addRow("no name") << d << false;
    }
    {
        QCanSignalDescription d = desc;
        d.setBitLength(0);
        QTest::addRow("zero length") << d << false;
    }
    {
        QCanSignalDescription d = desc;
        d.setBitLength(65);
        QTest::addRow("bitLength too long") << d << false;
    }
    {
        QCanSignalDescription d = desc;
        d.setDataFormat(QtCanBus::DataFormat::Float);
        QTest::addRow("incorrect float length") << d << false;

        d.setBitLength(32);
        QTest::addRow("correct float length") << d << true;
    }
    {
        QCanSignalDescription d = desc;
        d.setDataFormat(QtCanBus::DataFormat::Double);
        d.setBitLength(32);
        QTest::addRow("incorrect double length") << d << false;

        d.setBitLength(64);
        QTest::addRow("correct double length") << d << true;
    }
    {
        QCanSignalDescription d = desc;
        d.setDataFormat(QtCanBus::DataFormat::AsciiString);
        d.setBitLength(11);
        QTest::addRow("incorrect ASCII length") << d << false;

        d.setBitLength(80);
        QTest::addRow("correct ASCII length") << d << true;
    }
}

void tst_QCanSignalDescription::isValid()
{
    QFETCH(QCanSignalDescription, signalDescription);
    QFETCH(bool, expectedResult);

    QCOMPARE(signalDescription.isValid(), expectedResult);
}

QTEST_MAIN(tst_QCanSignalDescription)

#include "tst_qcansignaldescription.moc"
