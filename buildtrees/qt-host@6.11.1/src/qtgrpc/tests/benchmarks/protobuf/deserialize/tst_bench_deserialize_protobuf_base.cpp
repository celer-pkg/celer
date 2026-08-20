// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "tst_bench_deserialize_protobuf_base.h"

#include "bench.qpb.h"

using namespace Qt::StringLiterals;

ProtobufDeserializeBenchmarkBase::
    ProtobufDeserializeBenchmarkBase(ProtobufDataPtr data,
                                     std::unique_ptr<QAbstractProtobufSerializer> serializer)
    : m_data(data), m_serializer(std::move(serializer))
{
}

void ProtobufDeserializeBenchmarkBase::initTestCase()
{
    qtbench::ScalarTypes s;
    s.setField1(std::numeric_limits<QtProtobuf::int32>::min());
    s.setField2(std::numeric_limits<QtProtobuf::uint32>::max());
    s.setField3(std::numeric_limits<QtProtobuf::sint32>::max());
    s.setField4(std::numeric_limits<QtProtobuf::fixed32>::max());
    s.setField5(std::numeric_limits<QtProtobuf::sfixed32>::min());
    s.setField6(std::numeric_limits<QtProtobuf::int64>::min());
    s.setField7(std::numeric_limits<QtProtobuf::uint64>::max());
    s.setField8(std::numeric_limits<QtProtobuf::sint64>::min());
    s.setField9(std::numeric_limits<QtProtobuf::fixed64>::max());
    s.setField10(std::numeric_limits<QtProtobuf::sfixed64>::min());
    s.setField11(42.0);
    s.setField12(42.0);
    s.setField13("Hello Qt!"_L1);
    s.setField14("Hello Qt!"_ba);
    s.setField15(qtbench::ScalarTypes::EnumType::Value3);

    qtbench::ScalarTypesRepeated r;
    r.setField1({
        s,
        s,
        s,
        s,
        s,
        s,
        s,
        s,
        s,
        s,
    });
    r.setField2({ qtbench::ScalarTypes::EnumType::Value1, qtbench::ScalarTypes::EnumType::Value2,
                  qtbench::ScalarTypes::EnumType::Value3, qtbench::ScalarTypes::EnumType::Value3,
                  qtbench::ScalarTypes::EnumType::Value2, qtbench::ScalarTypes::EnumType::Value1,
                  qtbench::ScalarTypes::EnumType::Value2, qtbench::ScalarTypes::EnumType::Value3,
                  qtbench::ScalarTypes::EnumType::Value1, qtbench::ScalarTypes::EnumType::Value2,
                  qtbench::ScalarTypes::EnumType::Value1, qtbench::ScalarTypes::EnumType::Value3 });

    m_benchMsg.setField1({
        { 0,                                             r },
        { std::numeric_limits<QtProtobuf::int32>::max(), r },
        { std::numeric_limits<QtProtobuf::int32>::min(), r }
    });
    m_benchMsg.setField2({
        { ""_L1,           r },
        { "Hellow Qt!"_L1, r }
    });
}

void ProtobufDeserializeBenchmarkBase::deserialize_data()
{
    QTest::addColumn<QString>("name");
    QTest::addColumn<QByteArray>("data");

    for (auto &&[name, value] : *m_data)
        QTest::newRow(qPrintable(name)) << name << value;

    qtbench::BenchmarkMessage fullMsg;
    for (auto i : { 1, 10, 100, 1000 }) {
        fullMsg.setField1(QList<qtbench::MapTypes>{ i, m_benchMsg });
        QTest::addRow("qtbench.BenchmarkMessage x %d", i)
            << "qtbench.BenchmarkMessage" << fullMsg.serialize(m_serializer.get());
    }
}

void ProtobufDeserializeBenchmarkBase::deserialize()
{
    QFETCH(QString, name);
    QFETCH(QByteArray, data);

    QBENCHMARK {
        QProtobufMessagePointer res = QProtobufMessage::constructByName(name);
        QVERIFY(res);
        QVERIFY(res->deserialize(m_serializer.get(), data));
    }
}

#include "moc_tst_bench_deserialize_protobuf_base.cpp"
