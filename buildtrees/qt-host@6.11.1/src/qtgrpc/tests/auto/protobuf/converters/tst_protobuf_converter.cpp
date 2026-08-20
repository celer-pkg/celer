// Copyright (C) 2022 The Qt Company Ltd.
// Copyright (C) 2019 Alexey Edelev <semlanik@gmail.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QTest>

#include <QVariant>
#include <qtprotobuftypes.h>
#include <qprotobufregistration.h>

class QtProtobufConverterTest : public QObject
{
    Q_OBJECT
public:
    QtProtobufConverterTest() {
        // Need to call this explicitly since there is not implicit call generated (we don't use any
        // messages)
        qRegisterProtobufTypes();
    }
private Q_SLOTS:
    void testFromTypeConverters();
    void testToTypeConverters();
};

void QtProtobufConverterTest::testFromTypeConverters()
{
    QVariant testVariant;

    testVariant.setValue<uint32_t>(42);
    QCOMPARE(testVariant.value<QtProtobuf::fixed32>().t, quint32(42));

    testVariant.setValue<uint64_t>(43);
    QCOMPARE(testVariant.value<QtProtobuf::fixed64>().t, quint64(43));

    testVariant.setValue<int32_t>(44);
    QCOMPARE(testVariant.value<QtProtobuf::sfixed32>().t, 44);

    testVariant.setValue<int64_t>(45);
    QCOMPARE(testVariant.value<QtProtobuf::sfixed64>().t, 45);

    testVariant.setValue<int32_t>(46);
    QCOMPARE(testVariant.value<QtProtobuf::int32>().t, 46);

    testVariant.setValue<int64_t>(47);
    QCOMPARE(testVariant.value<QtProtobuf::int64>().t, 47);
}

void QtProtobufConverterTest::testToTypeConverters()
{
    bool ok = false;
    QVariant testVariant;
    testVariant.setValue<QtProtobuf::fixed32>({42});
    QCOMPARE(testVariant.toUInt(&ok), quint32(42));
    QVERIFY(ok);

    ok = false;
    testVariant.setValue<QtProtobuf::fixed64>({43});
    QCOMPARE(testVariant.toULongLong(&ok), quint64(43));
    QVERIFY(ok);

    ok = false;
    testVariant.setValue<QtProtobuf::sfixed32>({44});
    QCOMPARE(testVariant.toInt(&ok), 44);
    QVERIFY(ok);

    ok = false;
    testVariant.setValue<QtProtobuf::sfixed64>({45});
    QCOMPARE(testVariant.toLongLong(&ok), 45);
    QVERIFY(ok);

    ok = false;
    testVariant.setValue<QtProtobuf::int32>({46});
    QCOMPARE(testVariant.toInt(&ok), 46);
    QVERIFY(ok);

    ok = false;
    testVariant.setValue<QtProtobuf::int64>({47});
    QCOMPARE(testVariant.toLongLong(&ok), 47);
    QVERIFY(ok);
}

QTEST_MAIN(QtProtobufConverterTest)
#include "tst_protobuf_converter.moc"
