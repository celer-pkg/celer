// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtTest/qtest.h>

#include <QtProtobuf/qprotobufserializer.h>

#include "tst_bench_deserialize_protobuf_base.h"

using namespace Qt::StringLiterals;

class ProtobufDeserializeBenchmark : public ProtobufDeserializeBenchmarkBase
{
    Q_OBJECT
public:
    ProtobufDeserializeBenchmark();
};

std::array<std::pair<QString, QByteArray>, 3> data = {
    std::make_pair("qtbench.SimpleBoolMessage", QByteArray::fromHex("0801")),
    std::make_pair("qtbench.SimpleBytesMessage",
                   QByteArray::fromHex("0a0c48656c6c6f20776f726c6421")),
    std::make_pair("qtbench.RecursiveMessage",
                   QByteArray::fromHex("080112120802120e0803120a08041206080512020806")),
};

ProtobufDeserializeBenchmark::ProtobufDeserializeBenchmark()
    : ProtobufDeserializeBenchmarkBase(&data, std::make_unique<QProtobufSerializer>())
{
}

QTEST_MAIN(ProtobufDeserializeBenchmark)

#include "tst_bench_deserialize_protobuf.moc"
