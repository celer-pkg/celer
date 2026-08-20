// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef TST_BENCH_DESERIALIZE_PROTOBUF_BASE_H
#define TST_BENCH_DESERIALIZE_PROTOBUF_BASE_H

#include <QtTest/qtest.h>

#include <QtProtobuf/qprotobufserializer.h>

#include "bench.qpb.h"

using ProtobufData = std::array<std::pair<QString, QByteArray>, 3>;
using ProtobufDataPtr = ProtobufData *;

class ProtobufDeserializeBenchmarkBase : public QObject
{
    Q_OBJECT

protected:
    ProtobufDeserializeBenchmarkBase(ProtobufDataPtr data,
                                     std::unique_ptr<QAbstractProtobufSerializer> serializer);

private Q_SLOTS:
    void initTestCase();
    void deserialize_data();
    void deserialize();

private:
    ProtobufDataPtr m_data = nullptr;
    std::unique_ptr<QAbstractProtobufSerializer> m_serializer;
    qtbench::MapTypes m_benchMsg;
};

#endif // TST_BENCH_DESERIALIZE_PROTOBUF_BASE_H
