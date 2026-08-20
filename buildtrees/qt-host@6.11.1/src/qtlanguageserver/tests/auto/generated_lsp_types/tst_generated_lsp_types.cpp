// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtTest/QtTest>
#include "tst_generated_lsp_types.h"

using namespace QLspSpecification;

template <typename T>
static QJsonValue serialize(const T &value)
{
    QTypedJson::JsonBuilder builder;
    T copy{ value };
    QTypedJson::doWalk(builder, copy);
    return builder.popLastValue();
}

template <typename T>
static QJsonValue deserializeAndReserialize(const QJsonValue &value)
{
    QTypedJson::Reader reader(value);
    T result;
    QTypedJson::doWalk(reader, result);
    // note: T has no operator==, so compare the serialized version instead
    return serialize(result);
}

template <typename T>
static void tst_types_impl(const T &expectedValue, QByteArrayView expectedJson)
{
    QJsonParseError error;
    const QJsonValue expectedJsonValue = QJsonValue::fromJson(expectedJson);
    QVERIFY2(error.error == QJsonParseError::NoError, qPrintable(error.errorString()));
    QCOMPARE_EQ(serialize(expectedValue), expectedJsonValue);

    QCOMPARE_EQ(deserializeAndReserialize<T>(expectedJsonValue), expectedJsonValue);
}

void tst_generated_lsp_types::tst_types()
{
    tst_types_impl(ProgressParams{ "myToken", WorkDoneProgressBegin{ "begin", "hello world!" } },
                   R"({"token":"myToken","value":{"kind":"begin","title":"hello world!"}})");
}

QTEST_MAIN(tst_generated_lsp_types)
