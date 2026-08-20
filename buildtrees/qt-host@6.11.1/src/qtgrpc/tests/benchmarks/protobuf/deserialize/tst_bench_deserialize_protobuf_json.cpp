// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtTest/qtest.h>

#include <QtProtobuf/QProtobufSerializer>
#include <QtProtobuf/qprotobufjsonserializer.h>

#include "tst_bench_deserialize_protobuf_base.h"

using namespace Qt::StringLiterals;

class ProtobufJsonDeserializeBenchmark : public ProtobufDeserializeBenchmarkBase
{
    Q_OBJECT
public:
    ProtobufJsonDeserializeBenchmark();
};

std::array<std::pair<QString, QByteArray>, 3> data = {
    std::make_pair("qtbench.SimpleBoolMessage", QByteArray("{\"testFieldBool\":true}")),
    std::make_pair("qtbench.SimpleBytesMessage",
                   QByteArray("{\"testFieldBytes\":\"SGVsbG8gd29ybGQh\"}")),
    std::make_pair("qtbench.RecursiveMessage",
                   QByteArray("{\"testFieldInt\":1,\"testFieldRecursive\":{\"testFieldInt\":2,"
                              "\"testFieldRecursive\":{\"testFieldInt\":3,\"testFieldRecursive\":{"
                              "\"testFieldInt\":4,\"testFieldRecursive\":{\"testFieldInt\":5,"
                              "\"testFieldRecursive\":{\"testFieldInt\":6}}}}}}")),
};

ProtobufJsonDeserializeBenchmark::ProtobufJsonDeserializeBenchmark()
    : ProtobufDeserializeBenchmarkBase(&data, std::make_unique<QProtobufJsonSerializer>())
{
}

QTEST_MAIN(ProtobufJsonDeserializeBenchmark)

#include "tst_bench_deserialize_protobuf_json.moc"
