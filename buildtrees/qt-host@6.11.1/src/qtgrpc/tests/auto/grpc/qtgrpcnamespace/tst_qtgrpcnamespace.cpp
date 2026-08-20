// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtTest/private/qcomparisontesthelper_p.h>
#include <QtTest/qtest.h>

#include <QtGrpc/qtgrpcnamespace.h>

#include <QtCore/qdebug.h>
#include <QtCore/qbuffer.h>
#include <QtCore/qbytearray.h>

using namespace Qt::Literals::StringLiterals;
using namespace QtGrpc;

class QtGrpcNamespaceTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void rpcDescriptorHashing() const;
    void rpcDescriptorStreamsToDebug() const;
    void rpcDescriptorHasMetatype() const;
    void rpcDescriptorStrongOrdering() const;
};

void QtGrpcNamespaceTest::rpcDescriptorHashing() const
{
    RpcDescriptor d1{ "service1"_L1, "method1"_L1, RpcType::UnaryCall };
    auto d2 = d1;
    QCOMPARE_EQ(qHash(d1), qHash(d2));
}

void QtGrpcNamespaceTest::rpcDescriptorStreamsToDebug() const
{
    constexpr QLatin1StringView expected(
        R"(QtGrpc::RpcDescriptor( service: "s1", method: "m1", type: QtGrpc::RpcType::UnaryCall ))");
    RpcDescriptor d1{ "s1"_L1, "m1"_L1, RpcType::UnaryCall };
    QCOMPARE_EQ(QDebug::toBytes(d1), expected);
}

void QtGrpcNamespaceTest::rpcDescriptorHasMetatype() const
{
    const QMetaType metaType = QMetaType::fromType<QtGrpc::RpcDescriptor>();
    QVERIFY(metaType.isValid());
    QCOMPARE(metaType.id(), qMetaTypeId<QtGrpc::RpcDescriptor>());
}

void QtGrpcNamespaceTest::rpcDescriptorStrongOrdering() const
{
    QTestPrivate::testAllComparisonOperatorsCompile<RpcDescriptor>();

    constexpr RpcDescriptor A{
        "ServiceA"_L1,
        "Method"_L1,
        RpcType::UnaryCall
    };

    constexpr RpcDescriptor B{
        "ServiceB"_L1,
        "Method"_L1,
        RpcType::UnaryCall
    };

    QT_TEST_ALL_COMPARISON_OPS(A, B, Qt::strong_ordering::less);

    constexpr RpcDescriptor ASmallCase {
        "serviceA"_L1,
        "Method"_L1,
        RpcType::UnaryCall
    };
    QT_TEST_ALL_COMPARISON_OPS(A, ASmallCase, Qt::strong_ordering::less);

    constexpr auto AClone = A;
    QT_TEST_ALL_COMPARISON_OPS(A, AClone, Qt::strong_ordering::equal);

    constexpr RpcDescriptor BGreater{
        "ServiceB"_L1,
        "Method"_L1,
        RpcType::ServerStreaming
    };
    QT_TEST_ALL_COMPARISON_OPS(BGreater, B, Qt::strong_ordering::greater);
}

QTEST_MAIN(QtGrpcNamespaceTest)

#include "tst_qtgrpcnamespace.moc"
