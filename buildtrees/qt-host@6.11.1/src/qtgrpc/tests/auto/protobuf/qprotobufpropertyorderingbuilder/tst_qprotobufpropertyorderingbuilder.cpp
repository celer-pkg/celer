// Copyright (C) 2022 The Qt Company Ltd.
// Copyright (C) 2022 Alexey Edelev <semlanik@gmail.com>, Viktor Kopp <vifactor@gmail.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtTest/QtTest>

#include <QtProtobuf/private/qprotobufpropertyorderingbuilder_p.h>

class tst_QProtobufPropertyOrderingBuilder : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void buildObject();
};

struct DataDeleter {
    void operator()(QtProtobufPrivate::QProtobufPropertyOrdering::Data *ptr) noexcept
    {
        free(ptr);
    }
};

void tst_QProtobufPropertyOrderingBuilder::buildObject()
{
    using namespace QtProtobufPrivate;
    QProtobufPropertyOrderingBuilder builder("TestMessage");
    builder.addV0Field("field1", 1, 0, FieldFlag::Optional);
    builder.addV0Field("field6", 6, 1, FieldFlag::Repeated | FieldFlag::Enum);
    builder.addV0Field("field30", 30, 60, FieldFlag::NoFlags);
    std::unique_ptr<QProtobufPropertyOrdering::Data, DataDeleter> data(builder.build());

    QProtobufPropertyOrdering ordering{data.get()};
    QCOMPARE(ordering.messageFullName(), QByteArray("TestMessage"));
    QCOMPARE(ordering.fieldCount(), 3);

    QCOMPARE(ordering.indexOfFieldNumber(1), 0);
    QCOMPARE(ordering.indexOfFieldNumber(6), 1);
    QCOMPARE(ordering.indexOfFieldNumber(30), 2);
    QCOMPARE(ordering.indexOfFieldNumber(10), -1);

    QCOMPARE(ordering.propertyIndex(0), 0);
    QCOMPARE(ordering.propertyIndex(1), 1);
    QCOMPARE(ordering.propertyIndex(2), 60);
    QCOMPARE(ordering.propertyIndex(3), -1);

    QCOMPARE(ordering.fieldNumber(0), 1);
    QCOMPARE(ordering.fieldNumber(1), 6);
    QCOMPARE(ordering.fieldNumber(2), 30);
    QCOMPARE(ordering.fieldNumber(3), -1);

    QCOMPARE(ordering.jsonName(0), QByteArray("field1"));
    QCOMPARE(ordering.jsonName(1), QByteArray("field6"));
    QCOMPARE(ordering.jsonName(2), QByteArray("field30"));
    QCOMPARE(ordering.jsonName(3), QByteArray());

    QCOMPARE(ordering.fieldFlags(0), FieldFlag::Optional);
    QCOMPARE(ordering.fieldFlags(1), FieldFlag::Repeated | FieldFlag::Enum);
    QCOMPARE(ordering.fieldFlags(2), FieldFlag::NoFlags);
    QCOMPARE(ordering.fieldFlags(3), FieldFlag::NoFlags);
}

QTEST_MAIN(tst_QProtobufPropertyOrderingBuilder)
#include "tst_qprotobufpropertyorderingbuilder.moc"
