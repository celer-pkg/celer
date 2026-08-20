// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtTest/qtest.h>

#include <QtSerialBus/qcanuniqueiddescription.h>
#include <QtSerialBus/private/qcanuniqueiddescription_p.h>

#include "qcanuniqueiddescription_helpers.h"

QT_USE_NAMESPACE

class tst_QCanUniqueIdDescription : public QObject
{
Q_OBJECT

private slots:
    void init();

    void construct();
    void copy();
    void move();
    void comparison();
    void isValid();

private:
    QCanUniqueIdDescription d;
};

void tst_QCanUniqueIdDescription::init()
{
    d = QCanUniqueIdDescription();
    d.setSource(QtCanBus::DataSource::FrameId);
    d.setStartBit(0);
    d.setBitLength(29);
    d.setEndian(QSysInfo::Endian::LittleEndian);
}

void tst_QCanUniqueIdDescription::construct()
{
    QCanUniqueIdDescription desc;
    QVERIFY(!desc.isValid());
    QCOMPARE(desc.source(), QtCanBus::DataSource::FrameId);
    QCOMPARE(desc.endian(), QSysInfo::Endian::LittleEndian);
    QCOMPARE(desc.startBit(), 0);
    QCOMPARE(desc.bitLength(), 0);
}

void tst_QCanUniqueIdDescription::copy()
{
    QVERIFY(!QCanUniqueIdDescriptionPrivate::get(d)->isShared());

    QCanUniqueIdDescription d1(d);
    QVERIFY(equals(d1, d));
    QVERIFY(QCanUniqueIdDescriptionPrivate::get(d)->isShared());
    QVERIFY(QCanUniqueIdDescriptionPrivate::get(d1)->isShared());

    QCanUniqueIdDescription d2 = d;
    QVERIFY(equals(d2, d));
    QVERIFY(QCanUniqueIdDescriptionPrivate::get(d)->isShared());
    QVERIFY(QCanUniqueIdDescriptionPrivate::get(d2)->isShared());
}

void tst_QCanUniqueIdDescription::move()
{
    QVERIFY(!QCanUniqueIdDescriptionPrivate::get(d)->isShared());

    // copy fields manually to avoid having a shared private.
    QCanUniqueIdDescription otherD;
    otherD.setSource(d.source());
    otherD.setEndian(d.endian());
    otherD.setStartBit(d.startBit());
    otherD.setBitLength(d.bitLength());

    QCanUniqueIdDescription d1(std::move(d));
    QVERIFY(equals(d1, otherD));
    QVERIFY(!QCanUniqueIdDescriptionPrivate::get(d1)->isShared());

    QCanUniqueIdDescription d2 = std::move(d1);
    QVERIFY(equals(d2, otherD));
    QVERIFY(!QCanUniqueIdDescriptionPrivate::get(d2)->isShared());

    d = std::move(d2);
    QVERIFY(equals(d, otherD));
    QVERIFY(!QCanUniqueIdDescriptionPrivate::get(d)->isShared());
}

void tst_QCanUniqueIdDescription::comparison()
{
    {
        QCanUniqueIdDescription d1 = d;
        QVERIFY(equals(d1, d));
        // also check that calling const methods does not detach
        d1.isValid();
        d1.source();
        d1.endian();
        d1.startBit();
        d1.bitLength();
        QVERIFY(QCanUniqueIdDescriptionPrivate::get(d)->isShared());
        QVERIFY(QCanUniqueIdDescriptionPrivate::get(d1)->isShared());
    }
    {
        QCanUniqueIdDescription d1 = d;
        d1.setSource(QtCanBus::DataSource::Payload);
        QVERIFY(!equals(d1, d));
        QVERIFY(!QCanUniqueIdDescriptionPrivate::get(d)->isShared());
        QVERIFY(!QCanUniqueIdDescriptionPrivate::get(d1)->isShared());
    }
    {
        QCanUniqueIdDescription d1 = d;
        d1.setEndian(QSysInfo::Endian::BigEndian);
        QVERIFY(!equals(d1, d));
        QVERIFY(!QCanUniqueIdDescriptionPrivate::get(d)->isShared());
        QVERIFY(!QCanUniqueIdDescriptionPrivate::get(d1)->isShared());
    }
    {
        QCanUniqueIdDescription d1 = d;
        d1.setStartBit(1);
        QVERIFY(!equals(d1, d));
        QVERIFY(!QCanUniqueIdDescriptionPrivate::get(d)->isShared());
        QVERIFY(!QCanUniqueIdDescriptionPrivate::get(d1)->isShared());
    }
    {
        QCanUniqueIdDescription d1 = d;
        d1.setBitLength(10);
        QVERIFY(!equals(d1, d));
        QVERIFY(!QCanUniqueIdDescriptionPrivate::get(d)->isShared());
        QVERIFY(!QCanUniqueIdDescriptionPrivate::get(d1)->isShared());
    }
}

void tst_QCanUniqueIdDescription::isValid()
{
    QVERIFY(d.isValid());

    // 0 length - invalid
    QCanUniqueIdDescription desc;
    QVERIFY(!desc.isValid());

    constexpr auto uidLength = sizeof(QtCanBus::UniqueId) * 8;

    // length in range - valid
    desc.setBitLength(uidLength - 1);
    QVERIFY(desc.isValid());

    // too large length - invalid
    desc.setBitLength(uidLength + 1);
    QVERIFY(!desc.isValid());
}

QTEST_MAIN(tst_QCanUniqueIdDescription)

#include "tst_qcanuniqueiddescription.moc"
