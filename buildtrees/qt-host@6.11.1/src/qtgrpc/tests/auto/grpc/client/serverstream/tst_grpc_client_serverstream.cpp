// Copyright (C) 2022 The Qt Company Ltd.
// Copyright (C) 2019 Alexey Edelev <semlanik@gmail.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
#include <grpcclienttestbase.h>

#include <QtGrpc/QGrpcCallOptions>

#include <QtCore/QCryptographicHash>
#include <QtCore/QFile>
#include <QtCore/QThread>
#include <QtCore/QTimer>

#include <QtTest/QSignalSpy>
#include <QtTest/QTest>

#include <message_latency_defs.h>
#include <testservice_client.grpc.qpb.h>

using namespace Qt::Literals::StringLiterals;
using namespace qtgrpc::tests;

class QtGrpcClientServerStreamTest : public GrpcClientTestBase
{
    Q_OBJECT
public:
    QtGrpcClientServerStreamTest()
        : GrpcClientTestBase(
                Channels{ GrpcClientTestBase::Channel::Qt })
    {
    }

private Q_SLOTS:
    void valid();
    void cancel();
    void deferredCancel();
    void hugeBlob();
    void multipleStreams();
    void multipleStreamsCancel();
    void cancelWhileErrorTimeout();
};

void QtGrpcClientServerStreamTest::valid()
{
    const int ExpectedMessageCount = 4;

    SimpleStringMessage result;
    SimpleStringMessage request;
    request.setTestFieldString("Stream");

    auto stream = client()->testMethodServerStream(request);
    auto *streamPtr = stream.get();

    QSignalSpy messageReceivedSpy(streamPtr, &QGrpcServerStream::messageReceived);
    QVERIFY(messageReceivedSpy.isValid());

    QSignalSpy streamFinishedSpy(streamPtr, &QGrpcServerStream::finished);
    QVERIFY(streamFinishedSpy.isValid());

    QObject::connect(streamPtr, &QGrpcServerStream::messageReceived, this,
                     [&result, stream = std::move(stream)] {
                         const auto ret = stream->read<SimpleStringMessage>();
                         QVERIFY(ret.has_value());
                         result.setTestFieldString(result.testFieldString()
                                                   + ret->testFieldString());
                     });

    QTRY_COMPARE_EQ_WITH_TIMEOUT(streamFinishedSpy.count(), 1,
                                 MessageLatencyWithThreshold * ExpectedMessageCount);
    auto args = streamFinishedSpy.first();
    QCOMPARE(args.count(), 1);
    QCOMPARE_EQ(args.first().value<QGrpcStatus>().code(), QtGrpc::StatusCode::Ok);

    QCOMPARE(messageReceivedSpy.count(), ExpectedMessageCount);
    QCOMPARE_EQ(result.testFieldString(), "Stream1Stream2Stream3Stream4");
}

void QtGrpcClientServerStreamTest::cancel()
{
    const int ExpectedMessageCount = 2;

    SimpleStringMessage result;
    SimpleStringMessage request;
    request.setTestFieldString("Stream");

    auto stream = client()->testMethodServerStream(request);

    QSignalSpy streamFinishedSpy(stream.get(), &QGrpcServerStream::finished);
    QVERIFY(streamFinishedSpy.isValid());

    int i = 0;
    QObject::connect(stream.get(), &QGrpcServerStream::messageReceived, this, [&] {
        const auto ret = stream->read<SimpleStringMessage>();
        QVERIFY(ret.has_value());
        result.setTestFieldString(result.testFieldString() + ret->testFieldString());
        if (++i == ExpectedMessageCount)
            stream->cancel();
    });

    QTRY_COMPARE_EQ_WITH_TIMEOUT(streamFinishedSpy.count(), 1,
                                 MessageLatencyWithThreshold * ExpectedMessageCount);

    auto args = streamFinishedSpy.first();
    QCOMPARE(args.count(), 1);
    QVERIFY(args.first().value<QGrpcStatus>() == QtGrpc::StatusCode::Cancelled);

    QCOMPARE(i, 2);
    QCOMPARE_EQ(result.testFieldString(), "Stream1Stream2");
}

void QtGrpcClientServerStreamTest::deferredCancel()
{
    const int ExpectedMessageCount = 3;

    SimpleStringMessage result;
    SimpleStringMessage request;
    request.setTestFieldString("Stream");

    auto stream = client()->testMethodServerStream(request);

    QSignalSpy streamFinishedSpy(stream.get(), &QGrpcServerStream::finished);
    QVERIFY(streamFinishedSpy.isValid());

    QSignalSpy messageReceivedSpy(stream.get(), &QGrpcServerStream::messageReceived);
    QVERIFY(messageReceivedSpy.isValid());

    int i = 0;
    QObject::connect(stream.get(), &QGrpcServerStream::messageReceived, this, [&] {
        const auto ret = stream->read<SimpleStringMessage>();
        QVERIFY(ret.has_value());
        result.setTestFieldString(result.testFieldString() + ret->testFieldString());
        if (++i == ExpectedMessageCount)
            QTimer::singleShot(MessageLatencyThreshold, stream.get(), &QGrpcServerStream::cancel);
    });

    QTRY_COMPARE_EQ_WITH_TIMEOUT(streamFinishedSpy.count(), 1,
                                 MessageLatencyWithThreshold * ExpectedMessageCount);
    auto args = streamFinishedSpy.first();
    QCOMPARE(args.count(), 1);
    QVERIFY(args.first().value<QGrpcStatus>() == QtGrpc::StatusCode::Cancelled);
    QCOMPARE(messageReceivedSpy.count(), ExpectedMessageCount);

    QCOMPARE_EQ(result.testFieldString(), "Stream1Stream2Stream3");
}

void QtGrpcClientServerStreamTest::hugeBlob()
{
    BlobMessage result;
    BlobMessage request;
    QFile testFile(":/assets/testfile");
    QVERIFY(testFile.open(QFile::ReadOnly));

    request.setTestBytes(testFile.readAll());
    QByteArray dataHash = QCryptographicHash::hash(request.testBytes(), QCryptographicHash::Sha256);

    auto stream = client()->testMethodBlobServerStream(request);
    auto *streamPtr = stream.get();

    QSignalSpy streamFinishedSpy(streamPtr, &QGrpcServerStream::finished);
    QVERIFY(streamFinishedSpy.isValid());

    QObject::connect(streamPtr, &QGrpcServerStream::messageReceived, this,
                     [&result, stream = streamPtr] {
                         const auto ret = stream->read<BlobMessage>();
                         QVERIFY(ret.has_value());
                         result.setTestBytes(ret->testBytes());
                     });

    QTRY_COMPARE_EQ_WITH_TIMEOUT(streamFinishedSpy.count(), 1, MessageLatencyWithThreshold);
    auto args = streamFinishedSpy.first();
    QCOMPARE(args.count(), 1);
    QCOMPARE_EQ(args.first().value<QGrpcStatus>().code(), QtGrpc::StatusCode::Ok);

    QVERIFY(!result.testBytes().isEmpty());
    QByteArray returnDataHash =
            QCryptographicHash::hash(result.testBytes(), QCryptographicHash::Sha256);
    QCOMPARE_EQ(returnDataHash, dataHash);
}

void QtGrpcClientServerStreamTest::multipleStreams()
{
    const int ExpectedMessageCount = 4;
    SimpleStringMessage result;
    SimpleStringMessage request;
    request.setTestFieldString("Stream");

    auto stream = client()->testMethodServerStream(request);
    auto *streamPtr = stream.get();
    // Ensure we're not reusing streams
    QCOMPARE_NE(streamPtr, client()->testMethodServerStream(request).get());

    QSignalSpy streamFinishedSpy(streamPtr, &QGrpcServerStream::finished);
    QVERIFY(streamFinishedSpy.isValid());

    QSignalSpy steamMessageRecievedSpy(streamPtr, &QGrpcServerStream::messageReceived);
    QVERIFY(steamMessageRecievedSpy.isValid());

    QObject::connect(streamPtr, &QGrpcServerStream::messageReceived, this,
                     [&result, stream = std::move(stream)] {
                         const auto ret = stream->read<SimpleStringMessage>();
                         QVERIFY(ret.has_value());
                         result.setTestFieldString(result.testFieldString()
                                                   + ret->testFieldString());
                     });

    QTRY_COMPARE_EQ_WITH_TIMEOUT(streamFinishedSpy.count(), 1,
                                 MessageLatencyWithThreshold * ExpectedMessageCount);
    auto args = streamFinishedSpy.first();
    QCOMPARE(args.count(), 1);
    QCOMPARE_EQ(args.first().value<QGrpcStatus>().code(), QtGrpc::StatusCode::Ok);

    QCOMPARE_EQ(steamMessageRecievedSpy.count(), ExpectedMessageCount);
    QCOMPARE_EQ(result.testFieldString(), "Stream1Stream2Stream3Stream4");
}

void QtGrpcClientServerStreamTest::multipleStreamsCancel()
{
    SimpleStringMessage result;
    SimpleStringMessage request;
    request.setTestFieldString("Stream");

    auto stream = client()->testMethodServerStream(request);
    auto streamNext = client()->testMethodServerStream(request);

    QCOMPARE_NE(stream, streamNext);

    QSignalSpy streamFinishedSpy(stream.get(), &QGrpcServerStream::finished);
    QVERIFY(streamFinishedSpy.isValid());

    QSignalSpy streamNextFinishedSpy(streamNext.get(), &QGrpcServerStream::finished);
    QVERIFY(streamNextFinishedSpy.isValid());

    streamNext->cancel();

    QCOMPARE(streamFinishedSpy.count(), 0);

    QCOMPARE(streamNextFinishedSpy.count(), 1);
    auto args = streamNextFinishedSpy.first();
    QCOMPARE(args.count(), 1);
    QVERIFY(args.first().value<QGrpcStatus>() == QtGrpc::StatusCode::Cancelled);

    stream = client()->testMethodServerStream(request);
    QCOMPARE_NE(stream, streamNext);

    streamNext = client()->testMethodServerStream(request);

    QCOMPARE_NE(stream, streamNext);

    QSignalSpy otherStreamFinishedSpy(stream.get(), &QGrpcServerStream::finished);
    QVERIFY(otherStreamFinishedSpy.isValid());

    QSignalSpy otherStreamNextFinishedSpy(streamNext.get(), &QGrpcServerStream::finished);
    QVERIFY(otherStreamNextFinishedSpy.isValid());

    stream->cancel();

    QCOMPARE(otherStreamFinishedSpy.count(), 1);
    args = otherStreamFinishedSpy.first();
    QCOMPARE(args.count(), 1);
    QVERIFY(args.first().value<QGrpcStatus>() == QtGrpc::StatusCode::Cancelled);

    const int ExpectedMessageCount = 4;
    QTRY_COMPARE_EQ_WITH_TIMEOUT(otherStreamNextFinishedSpy.count(), 1,
                                 MessageLatencyWithThreshold * ExpectedMessageCount);
    args = otherStreamNextFinishedSpy.first();
    QCOMPARE(args.count(), 1);
    QCOMPARE_EQ(args.first().value<QGrpcStatus>().code(), QtGrpc::StatusCode::Ok);
}

void QtGrpcClientServerStreamTest::cancelWhileErrorTimeout()
{
    SimpleStringMessage result;
    SimpleStringMessage request;
    request.setTestFieldString("Stream");

    auto stream = client()->testMethodServerStream(request);

    QSignalSpy streamFinishedSpy(stream.get(), &QGrpcServerStream::finished);
    QVERIFY(streamFinishedSpy.isValid());

    stream->cancel();
    stream.reset();

    QTRY_COMPARE_EQ_WITH_TIMEOUT(streamFinishedSpy.count(), 1, MessageLatencyWithThreshold);
    auto args = streamFinishedSpy.first();
    QCOMPARE(args.count(), 1);
    QVERIFY(args.first().value<QGrpcStatus>() == QtGrpc::StatusCode::Cancelled);
}

QTEST_MAIN(QtGrpcClientServerStreamTest)

#include "tst_grpc_client_serverstream.moc"
