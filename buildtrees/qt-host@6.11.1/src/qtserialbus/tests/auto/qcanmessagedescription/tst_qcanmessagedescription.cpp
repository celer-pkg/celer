// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtTest/qtest.h>

#include <QtSerialBus/qcanmessagedescription.h>
#include <QtSerialBus/qcansignaldescription.h>
#include <QtSerialBus/private/qcanmessagedescription_p.h>

#include "qcanmessagedescription_helpers.h"

QT_USE_NAMESPACE

class tst_QCanMessageDescription : public QObject
{
Q_OBJECT

private slots:
    void init();

    void construct();
    void copy();
    void move();
    void comparison();
    void isValid();
    void signalForName();

private:
    QCanMessageDescription d;
};

void tst_QCanMessageDescription::init()
{
    QCanSignalDescription s0;
    s0.setName("s0");
    s0.setBitLength(8);

    QCanSignalDescription s1;
    s1.setName("s1");
    s1.setStartBit(8);
    s1.setBitLength(8);

    d = QCanMessageDescription();
    d.setName("test");
    d.setUniqueId(QtCanBus::UniqueId{123});
    d.setSize(2);
    d.setSignalDescriptions({ s0, s1 });
}

void tst_QCanMessageDescription::construct()
{
    QCanMessageDescription desc;
    QVERIFY(!desc.isValid());
    QCOMPARE(desc.uniqueId(), QtCanBus::UniqueId{0});
    QCOMPARE(desc.name(), QString());
    QCOMPARE(desc.size(), 0);
    QCOMPARE(desc.transmitter(), QString());
    QCOMPARE(desc.comment(), QString());
    QVERIFY(desc.signalDescriptions().isEmpty());
}

void tst_QCanMessageDescription::copy()
{
    QVERIFY(!QCanMessageDescriptionPrivate::get(d)->isShared());

    QCanMessageDescription d1(d);
    QVERIFY(equals(d1, d));
    QVERIFY(QCanMessageDescriptionPrivate::get(d)->isShared());
    QVERIFY(QCanMessageDescriptionPrivate::get(d1)->isShared());

    QCanMessageDescription d2 = d;
    QVERIFY(equals(d2, d));
    QVERIFY(QCanMessageDescriptionPrivate::get(d)->isShared());
    QVERIFY(QCanMessageDescriptionPrivate::get(d2)->isShared());
}

void tst_QCanMessageDescription::move()
{
    QVERIFY(!QCanMessageDescriptionPrivate::get(d)->isShared());

    // copy fields manually to avoid having a shared private.
    QCanMessageDescription otherD;
    otherD.setName(d.name());
    otherD.setUniqueId(d.uniqueId());
    otherD.setSize(d.size());
    otherD.setSignalDescriptions(d.signalDescriptions());
    QVERIFY(equals(otherD, d));

    QCanMessageDescription d1(std::move(d));
    QVERIFY(equals(otherD, d1));
    QVERIFY(!QCanMessageDescriptionPrivate::get(d1)->isShared());

    QCanMessageDescription d2 = std::move(d1);
    QVERIFY(equals(otherD, d2));
    QVERIFY(!QCanMessageDescriptionPrivate::get(d2)->isShared());

    d = std::move(d2);
    QVERIFY(equals(otherD, d));
    QVERIFY(!QCanMessageDescriptionPrivate::get(d)->isShared());
}

void tst_QCanMessageDescription::comparison()
{
    {
        QCanMessageDescription d1 = d;
        QVERIFY(equals(d1, d));
        // also check that calling const methods does not detach
        d1.isValid();
        d1.uniqueId();
        d1.name();
        d1.size();
        d1.transmitter();
        d1.comment();
        d1.signalDescriptions();
        d1.signalDescriptionForName("s0");
        QVERIFY(QCanMessageDescriptionPrivate::get(d)->isShared());
        QVERIFY(QCanMessageDescriptionPrivate::get(d1)->isShared());
    }
    {
        QCanMessageDescription d1 = d;
        d1.setName("test1");
        QVERIFY(!equals(d1, d));
        QVERIFY(!QCanMessageDescriptionPrivate::get(d)->isShared());
        QVERIFY(!QCanMessageDescriptionPrivate::get(d1)->isShared());
    }
    {
        QCanMessageDescription d1 = d;
        d1.setUniqueId(QtCanBus::UniqueId{456});
        QVERIFY(!equals(d1, d));
        QVERIFY(!QCanMessageDescriptionPrivate::get(d)->isShared());
        QVERIFY(!QCanMessageDescriptionPrivate::get(d1)->isShared());
    }
    {
        QCanMessageDescription d1 = d;
        d1.setSize(3);
        QVERIFY(!equals(d1, d));
        QVERIFY(!QCanMessageDescriptionPrivate::get(d)->isShared());
        QVERIFY(!QCanMessageDescriptionPrivate::get(d1)->isShared());
    }
    {
        QCanMessageDescription d1 = d;
        d1.setTransmitter("t");
        QVERIFY(!equals(d1, d));
        QVERIFY(!QCanMessageDescriptionPrivate::get(d)->isShared());
        QVERIFY(!QCanMessageDescriptionPrivate::get(d1)->isShared());
    }
    {
        QCanMessageDescription d1 = d;
        d1.setComment("comment");
        QVERIFY(!equals(d1, d));
        QVERIFY(!QCanMessageDescriptionPrivate::get(d)->isShared());
        QVERIFY(!QCanMessageDescriptionPrivate::get(d1)->isShared());
    }
    {
        QCanMessageDescription d1 = d;
        d1.clearSignalDescriptions();
        QVERIFY(!equals(d1, d));
        QVERIFY(!QCanMessageDescriptionPrivate::get(d)->isShared());
        QVERIFY(!QCanMessageDescriptionPrivate::get(d1)->isShared());
    }
    QCanSignalDescription s;
    s.setName("s");
    s.setStartBit(16);
    s.setBitLength(8);
    {
        QCanMessageDescription d1 = d;
        d1.addSignalDescription(s);
        QVERIFY(!equals(d1, d));
        QVERIFY(!QCanMessageDescriptionPrivate::get(d)->isShared());
        QVERIFY(!QCanMessageDescriptionPrivate::get(d1)->isShared());
    }
    {
        QCanMessageDescription d1 = d;
        d1.setSignalDescriptions({ s });
        QVERIFY(!equals(d1, d));
        QVERIFY(!QCanMessageDescriptionPrivate::get(d)->isShared());
        QVERIFY(!QCanMessageDescriptionPrivate::get(d1)->isShared());
    }
}

void tst_QCanMessageDescription::isValid()
{
    QVERIFY(d.isValid());

    // one of the signal descriptions is invalid -> invalid
    QCanSignalDescription sd;
    sd.setName("sig");
    sd.setDataFormat(QtCanBus::DataFormat::Float);
    sd.setBitLength(8); // invalid for Float format
    d.addSignalDescription(sd);
    QVERIFY(!d.isValid());

    // no signal descriptions -> invalid
    d.clearSignalDescriptions();
    QVERIFY(!d.isValid());
}

void tst_QCanMessageDescription::signalForName()
{
    // valid name
    QCanSignalDescription s = d.signalDescriptionForName("s0");
    QVERIFY(s.isValid());

    // invalid name
    s = d.signalDescriptionForName("test");
    QVERIFY(!s.isValid());
}

QTEST_MAIN(tst_QCanMessageDescription)

#include "tst_qcanmessagedescription.moc"
