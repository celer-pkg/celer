// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtTest/QtTest>
#include <QUuid>

#include <QDebug>

#include <qbluetoothuuid.h>

#if defined(Q_OS_DARWIN)
#include <QtCore/private/qcore_mac_p.h>
#endif

#if defined(Q_OS_UNIX)
#    include <arpa/inet.h>
#    include <netinet/in.h>
#endif

QT_USE_NAMESPACE

class tst_QBluetoothUuid : public QObject
{
    Q_OBJECT

public:
    tst_QBluetoothUuid();
    ~tst_QBluetoothUuid();

private slots:
    void initTestCase();

    void tst_construction();
    void tst_assignment();
    void tst_conversion_data();
    void tst_conversion();
    void tst_comparison_data();
    void tst_comparison();
};

tst_QBluetoothUuid::tst_QBluetoothUuid()
{
}

tst_QBluetoothUuid::~tst_QBluetoothUuid()
{
}

void tst_QBluetoothUuid::initTestCase()
{
}

void tst_QBluetoothUuid::tst_construction()
{
    {
        QBluetoothUuid nullUuid;

        QVERIFY(nullUuid.isNull());
    }

    {
        QBluetoothUuid uuid(QBluetoothUuid::ServiceClassUuid::PublicBrowseGroup);

        QVERIFY(!uuid.isNull());

        bool ok;
        quint16 uuid16;

        uuid16 = uuid.toUInt16(&ok);

        QVERIFY(ok);
        QCOMPARE(uuid16, static_cast<quint16>(QBluetoothUuid::ServiceClassUuid::PublicBrowseGroup));
    }

    {
        QBluetoothUuid uuid(QBluetoothUuid::ServiceClassUuid::PublicBrowseGroup);

        QBluetoothUuid copy(uuid);

        QCOMPARE(uuid.toUInt16(), copy.toUInt16());
    }

    {
        QBluetoothUuid uuid(QBluetoothUuid::ProtocolUuid::L2cap);

        QVERIFY(!uuid.isNull());

        bool ok;
        quint16 uuid16;

        uuid16 = uuid.toUInt16(&ok);

        QVERIFY(ok);
        QCOMPARE(uuid16, static_cast<quint16>(QBluetoothUuid::ProtocolUuid::L2cap));
    }

    {
        QUuid uuid(0x67c8770b, 0x44f1, 0x410a, 0xab, 0x9a, 0xf9, 0xb5, 0x44, 0x6f, 0x13, 0xee);
        QBluetoothUuid btUuid(uuid);
        QVERIFY(!btUuid.isNull());

        QString uuidString(btUuid.toString());
        QVERIFY(!uuidString.isEmpty());
        QCOMPARE(uuidString, QString("{67c8770b-44f1-410a-ab9a-f9b5446f13ee}"));
    }

    {
        QBluetoothUuid btUuid(QString("67c8770b-44f1-410a-ab9a-f9b5446f13ee"));
        QVERIFY(!btUuid.isNull());

        QString uuidString(btUuid.toString());
        QVERIFY(!uuidString.isEmpty());
        QCOMPARE(uuidString, QString("{67c8770b-44f1-410a-ab9a-f9b5446f13ee}"));
    }

    {
        QBluetoothUuid btUuid(QString("XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX"));
        QVERIFY(btUuid.isNull());
    }
}

void tst_QBluetoothUuid::tst_assignment()
{
    QBluetoothUuid uuid(QBluetoothUuid::ServiceClassUuid::PublicBrowseGroup);

    {
        QBluetoothUuid copy = uuid;

        QCOMPARE(uuid.toUInt16(), copy.toUInt16());
    }

    {
        QBluetoothUuid copy1;
        QBluetoothUuid copy2;

        QVERIFY(copy1.isNull());
        QVERIFY(copy2.isNull());

        copy1 = copy2 = uuid;

        QVERIFY(!copy1.isNull());
        QVERIFY(!copy2.isNull());

        QCOMPARE(uuid.toUInt16(), copy1.toUInt16());
        QCOMPARE(uuid.toUInt16(), copy2.toUInt16());
    }
}

#define BASEUUID "-0000-1000-8000-00805F9B34FB"

void tst_QBluetoothUuid::tst_conversion_data()
{
    QTest::addColumn<bool>("constructUuid16");
    QTest::addColumn<quint16>("uuid16");
    QTest::addColumn<bool>("constructUuid32");
    QTest::addColumn<quint32>("uuid32");
    QTest::addColumn<bool>("constructUuid128");
    QTest::addColumn<QUuid::Id128Bytes>("uuid128");
    QTest::addColumn<QString>("uuidS");

    static const auto uuid128_32 = [](quint8 a, quint8 b, quint8 c, quint8 d) {
        QUuid::Id128Bytes x = {
            {
                a, b, c, d,
                0x00, 0x00,
                0x10, 0x00,
                0x80, 0x00,
                0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB
            }
        };
        return x;
    };

    auto newRow32 = [](const char *name, quint8 a, quint8 b, quint8 c, quint8 d, const char *s) {
        auto uuid128 = uuid128_32(a, b, c, d);
        quint32 uuid32 = a << 24 | b << 16 | c << 8 | d;
        quint16 uuid16; \
        bool constructUuid16 = (a == 0) && (b == 0);
        if (constructUuid16)
            uuid16 = c << 8 | d;
        else
            uuid16 = 0;
        QTest::newRow(name) << constructUuid16 << uuid16 << true << uuid32 << true << uuid128
                            << (QLatin1Char('{') + QLatin1String(s) + QLatin1Char('}'));
    };

    auto newRow16 = [](const char *name, quint8 a, quint8 b, const char *s) {
        auto uuid128 = uuid128_32(0, 0, a, b);
        quint32 uuid32 = a << 8 | b;
        quint16 uuid16 = a << 8 | b;
        QTest::newRow(name) << true << uuid16 << true << uuid32 << true << uuid128
                            << (QLatin1Char('{') + QLatin1String(s) + QLatin1Char('}'));
    };

    newRow32("base uuid", 0x00, 0x00, 0x00, 0x00, "00000000" BASEUUID);
    newRow16("0x0001", 0x00, 0x01, "00000001" BASEUUID);
    newRow16("0xffff", 0xff, 0xff, "0000FFFF" BASEUUID);
    newRow32("0x00010000", 0x00, 0x01, 0x00, 0x00, "00010000" BASEUUID);
    newRow32("0x0001ffff", 0x00, 0x01, 0xff, 0xff, "0001FFFF" BASEUUID);
    newRow32("0xffff0000", 0xff, 0xff, 0x00, 0x00, "FFFF0000" BASEUUID);
    newRow32("0xffffffff", 0xff, 0xff, 0xff, 0xff, "FFFFFFFF" BASEUUID);

    {
        QUuid::Id128Bytes uuid128 = {
            {
                0x00, 0x11, 0x22, 0x33,
                0x44, 0x55,
                0x66, 0x77,
                0x88, 0x99,
                0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff
            }
        };

        QTest::newRow("00112233-4455-6677-8899-AABBCCDDEEFF")
            << false << quint16(0) << false << quint32(0) << true << uuid128
            << QStringLiteral("{00112233-4455-6677-8899-AABBCCDDEEFF}");
    }
}

void tst_QBluetoothUuid::tst_conversion()
{
    QFETCH(bool, constructUuid16);
    QFETCH(quint16, uuid16);
    QFETCH(bool, constructUuid32);
    QFETCH(quint32, uuid32);
    QFETCH(bool, constructUuid128);
    QFETCH(QUuid::Id128Bytes, uuid128);
    QFETCH(QString, uuidS);

    int minimumSize = 16;
    if (constructUuid16)
        minimumSize = 2;
    else if (constructUuid32)
        minimumSize = 4;

#if defined(Q_OS_DARWIN)
#define CHECK_PLATFORM_CONVERSION(qtUuid) \
    const QMacAutoReleasePool pool; \
    CBUUID *nativeUuid = qtUuid.toCBUUID(); \
    QVERIFY(nativeUuid); \
    QCOMPARE(qtUuid, QBluetoothUuid::fromCBUUID(nativeUuid));
#else
#define CHECK_PLATFORM_CONVERSION(qtUuid)
#endif // Q_OS_DARWIN

    if (constructUuid16) {
        QBluetoothUuid uuid(uuid16);

        QCOMPARE(uuid, QBluetoothUuid(QStringView{uuidS}));

        bool ok;

        QCOMPARE(uuid.toUInt16(&ok), uuid16);
        QVERIFY(ok);

        QCOMPARE(uuid.toUInt32(&ok), uuid32);
        QVERIFY(ok);

        QVERIFY(memcmp(uuid.toBytes().data, uuid128.data, 16) == 0);

        QCOMPARE(uuid.toString().toUpper(), uuidS.toUpper());

        QCOMPARE(uuid.minimumSize(), minimumSize);

        CHECK_PLATFORM_CONVERSION(uuid)
    }

    if (constructUuid32) {
        QBluetoothUuid uuid(uuid32);

        QCOMPARE(uuid, QBluetoothUuid(QLatin1StringView{uuidS.toLatin1()}));

        bool ok;

        quint16 tmp = uuid.toUInt16(&ok);
        QCOMPARE(ok, constructUuid16);
        if (ok) {
            QCOMPARE(tmp, uuid16);
        }

        QCOMPARE(uuid.toUInt32(&ok), uuid32);
        QVERIFY(ok);

        QVERIFY(memcmp(uuid.toBytes().data, uuid128.data, 16) == 0);

        QCOMPARE(uuid.toString().toUpper(), uuidS.toUpper());

        QCOMPARE(uuid.minimumSize(), minimumSize);

        CHECK_PLATFORM_CONVERSION(uuid)
    }

    if (constructUuid128) {
        QBluetoothUuid uuid(uuid128);

        QCOMPARE(uuid, QBluetoothUuid(QUtf8StringView{uuidS.toUtf8()}));

        bool ok;

        quint16 tmpUuid16 = uuid.toUInt16(&ok);
        QCOMPARE(ok, constructUuid16);
        if (ok)
            QCOMPARE(tmpUuid16, uuid16);

        quint32 tmpUuid32 = uuid.toUInt32(&ok);
        QCOMPARE(ok, constructUuid32);
        if (ok)
            QCOMPARE(tmpUuid32, uuid32);

        QVERIFY(memcmp(uuid.toBytes().data, uuid128.data, 16) == 0);

        QCOMPARE(uuid.toString().toUpper(), uuidS.toUpper());

        QCOMPARE(uuid.minimumSize(), minimumSize);

        CHECK_PLATFORM_CONVERSION(uuid)
    }
#undef CHECK_PLATFORM_CONVERSION
}

void tst_QBluetoothUuid::tst_comparison_data()
{
    tst_conversion_data();
}

void tst_QBluetoothUuid::tst_comparison()
{
    QFETCH(bool, constructUuid16);
    QFETCH(quint16, uuid16);
    QFETCH(bool, constructUuid32);
    QFETCH(quint32, uuid32);
    QFETCH(bool, constructUuid128);
    QFETCH(QUuid::Id128Bytes, uuid128);

    QVERIFY(QBluetoothUuid() == QBluetoothUuid());

    if (constructUuid16) {
        QBluetoothUuid quuid16(uuid16);
        QBluetoothUuid quuid32(uuid32);
        QBluetoothUuid quuid128(uuid128);

        QVERIFY(quuid16.toUInt16() == uuid16);
        QVERIFY(quuid16.toUInt32() == uuid32);

        QVERIFY(quuid16 == quuid32);
        QVERIFY(quuid16 == quuid128);

        QVERIFY(quuid32 == quuid16);
        QVERIFY(quuid128 == quuid16);
    }

    if (constructUuid32) {
        QBluetoothUuid quuid32(uuid32);
        QBluetoothUuid quuid128(uuid128);

        QVERIFY(quuid32.toUInt32() == uuid32);
        QVERIFY(quuid32 == quuid128);

        QVERIFY(quuid128 == quuid32);
    }

    if (constructUuid128) {
        QBluetoothUuid quuid128(uuid128);

        for (int var = 0; var < 16; ++var) {
            QVERIFY(quuid128.toBytes().data[var] == uuid128.data[var]);
        }

        // check that toUInt128() call returns the value in the same format as
        // QUuid::Id128Bytes, no matter what version we use (it can be
        // QUuid::toUint128() on platforms that define __SIZEOF_INT128__ or
        // QBluetoothUuid::toUint128() on other platforms).
        const quint128 i128 = quuid128.toUInt128();
        static_assert(sizeof(i128) == 16); // uint128 or QUuid::Id128Bytes
        uchar dst[16];
        memcpy(dst, &i128, sizeof(i128));
        for (int var = 0; var < 16; ++var)
            QCOMPARE_EQ(dst[var], uuid128.data[var]);

        // check that we always have a c-tor taking quint128
        QBluetoothUuid other{i128};
        const auto bytes = other.toBytes();
        for (int var = 0; var < 16; ++var)
            QCOMPARE_EQ(bytes.data[var], uuid128.data[var]);
    }
}

QTEST_MAIN(tst_QBluetoothUuid)

#include "tst_qbluetoothuuid.moc"
