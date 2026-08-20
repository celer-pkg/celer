// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtGrpc/qgrpcstatus.h>

#include <QtTest/qtest.h>

#include <QtCore/qbuffer.h>
#include <QtCore/qobject.h>
#include <QtCore/qttypetraits.h>

using namespace Qt::StringLiterals;
using namespace QtGrpc;

class QGrpcStatusTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void defaultConstructedIsOk() const;
    void hasSpecialMemberFunctions() const;
    void hasImplicitQVariant() const;
    void hasMemberSwap() const;
    void equalityComparable() const;
    void qHashOnlyStatusCodes() const;
    void streamsToDebug() const;
    void streamsToDataStream() const;
};

static_assert(qToUnderlying(StatusCode::Ok) == 0);
static_assert(qToUnderlying(StatusCode::Cancelled) == 1);
static_assert(qToUnderlying(StatusCode::Unknown) == 2);
static_assert(qToUnderlying(StatusCode::InvalidArgument) == 3);
static_assert(qToUnderlying(StatusCode::DeadlineExceeded) == 4);
static_assert(qToUnderlying(StatusCode::NotFound) == 5);
static_assert(qToUnderlying(StatusCode::AlreadyExists) == 6);
static_assert(qToUnderlying(StatusCode::PermissionDenied) == 7);
static_assert(qToUnderlying(StatusCode::ResourceExhausted) == 8);
static_assert(qToUnderlying(StatusCode::FailedPrecondition) == 9);
static_assert(qToUnderlying(StatusCode::Aborted) == 10);
static_assert(qToUnderlying(StatusCode::OutOfRange) == 11);
static_assert(qToUnderlying(StatusCode::Unimplemented) == 12);
static_assert(qToUnderlying(StatusCode::Internal) == 13);
static_assert(qToUnderlying(StatusCode::Unavailable) == 14);
static_assert(qToUnderlying(StatusCode::DataLoss) == 15);
static_assert(qToUnderlying(StatusCode::Unauthenticated) == 16);

void QGrpcStatusTest::defaultConstructedIsOk() const
{
    QGrpcStatus s1;
    QVERIFY(s1.message().isEmpty());
    QCOMPARE_EQ(s1.code(), StatusCode::Ok);
    QVERIFY(s1.isOk());

    QGrpcStatus s2(StatusCode::Ok);
    QCOMPARE_EQ(s1.code(), s2.code());
    QCOMPARE_EQ(s1.message(), s2.message());
}

void QGrpcStatusTest::hasSpecialMemberFunctions() const
{
    const auto check = [](const QGrpcStatus &s1, const QGrpcStatus &s2) {
        QCOMPARE_EQ(s1.code(), s2.code());
        QCOMPARE_EQ(s1.message(), s2.message());
    };

    QString msg = u"(◕д◕✿)😀"_s;
    QGrpcStatus s1 = { StatusCode::PermissionDenied, msg };
    QCOMPARE_EQ(s1.code(), StatusCode::PermissionDenied);
    QCOMPARE_EQ(s1.message(), msg);
    QVERIFY(!s1.isOk());

    QGrpcStatus s2(s1);
    check(s1, s2);
    QGrpcStatus s3 = s2;
    check(s2, s3);

    QGrpcStatus s4(std::move(s3));
    check(s4, s1);
    QGrpcStatus s5 = std::move(s4);
    check(s5, s1);
}

void QGrpcStatusTest::hasImplicitQVariant() const
{
    QGrpcStatus s1(StatusCode::DataLoss, "testcase");
    QVariant var1 = s1;
    QVERIFY(var1.isValid());
    const auto s2 = var1.value<QGrpcStatus>();
    QCOMPARE_EQ(s1.code(), s2.code());
    QCOMPARE_EQ(s1.message(), s2.message());
}

void QGrpcStatusTest::hasMemberSwap() const
{
    QGrpcStatus s1(StatusCode::DataLoss, "testcase");
    QGrpcStatus s2(s1);
    QGrpcStatus s3 = {};
    s1.swap(s3);
    QCOMPARE_EQ(s1.code(), StatusCode::Ok);
    QVERIFY(s1.message().isEmpty());
    QCOMPARE_EQ(s3.code(), s2.code());
    QCOMPARE_EQ(s3.message(), s2.message());
}

void QGrpcStatusTest::equalityComparable() const
{
    QGrpcStatus s1;
    QGrpcStatus s2(StatusCode::Internal);
    QCOMPARE_NE(s1, s2);
    s1 = s2;
    QCOMPARE_EQ(s1, s2);
    s1 = { StatusCode::Internal, "message is ignored" };
    QCOMPARE_EQ(s1, s2);
}

void QGrpcStatusTest::qHashOnlyStatusCodes() const
{
    QGrpcStatus s1;
    QGrpcStatus s2(StatusCode::Internal);
    const auto hash1 = qHash(s1);
    const auto hash2 = qHash(s2);
    QCOMPARE_NE(hash1, hash2);
    QGrpcStatus s3(StatusCode::Internal, "ignored");
    const auto hash3 = qHash(s3);
    QCOMPARE_EQ(hash2, hash3);
    QCOMPARE_NE(hash1, hash3);
}

void QGrpcStatusTest::streamsToDebug() const
{
    QGrpcStatus s1;
    QGrpcStatus s2 = { StatusCode::OutOfRange, "test" };

    QByteArray storage;
    QBuffer buf(&storage);
    QDebug dbg(&buf);

    QVERIFY(buf.data().isEmpty());
    buf.open(QIODevice::WriteOnly);
    dbg << s1 << s2;
    buf.close();
    QVERIFY(!buf.data().isEmpty());
}

void QGrpcStatusTest::streamsToDataStream() const
{
    QGrpcStatus s1;
    QGrpcStatus s2 = { StatusCode::OutOfRange };
    QGrpcStatus s3 = { StatusCode::Unimplemented, "test" };

    QByteArray storage;
    QBuffer buf(&storage);

    // QDataStream
    QDataStream ds(&buf);
    QVERIFY(buf.data().isEmpty());
    buf.open(QIODevice::ReadWrite);
    ds << s1 << s2 << s3;

    buf.seek(0);
    QGrpcStatus os1, os2, os3;
    ds >> os1 >> os2 >> os3;
    QCOMPARE_EQ(s1, os1);
    QCOMPARE_EQ(s1.message(), os1.message());
    QCOMPARE_EQ(s2, os2);
    QCOMPARE_EQ(s2.message(), os2.message());
    QCOMPARE_EQ(s3, os3);
    QCOMPARE_EQ(s3.message(), os3.message());

    buf.close();
    QVERIFY(!buf.data().isEmpty());
}

QTEST_MAIN(QGrpcStatusTest)

#include "tst_qgrpcstatus.moc"
