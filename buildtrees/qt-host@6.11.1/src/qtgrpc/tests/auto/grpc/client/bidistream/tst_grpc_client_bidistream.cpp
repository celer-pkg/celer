// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
#include <grpcclienttestbase.h>

#include <QtCore/QTimer>
#include <QtTest/QSignalSpy>
#include <QtTest/QTest>

#include <message_latency_defs.h>
#include <testservice_client.grpc.qpb.h>

using namespace Qt::Literals::StringLiterals;
using namespace qtgrpc::tests;

class QtGrpcClientBidiStreamTest : public GrpcClientTestBase
{
    Q_OBJECT
public:
    QtGrpcClientBidiStreamTest() : GrpcClientTestBase(Channels{ GrpcClientTestBase::Channel::Qt })
    {
    }

private Q_SLOTS:
    void valid();
    void sequentialSendWithDone();
    void multipleImmediateSendsWithDone();
};

void QtGrpcClientBidiStreamTest::valid()
{
    const int ExpectedMessageCount = 4;

    SimpleStringMessage request;
    request.setTestFieldString("Stream");

    auto stream = client()->testMethodBiStream(request);
    auto *streamPtr = stream.get();
    QString fullResponse;
    int i = 0;
    QObject::connect(streamPtr, &QGrpcBidiStream::messageReceived, this,
                     [stream = streamPtr, &request, &fullResponse, &i]() {
                         if (const auto rsp = stream->read<SimpleStringMessage>()) {
                             fullResponse += rsp->testFieldString() + QString::number(++i);
                             stream->writeMessage(request);
                         }
                     });

    QSignalSpy streamFinishedSpy(streamPtr, &QGrpcServerStream::finished);
    QVERIFY(streamFinishedSpy.isValid());

    QTRY_COMPARE_EQ_WITH_TIMEOUT(streamFinishedSpy.count(), 1,
                                 MessageLatencyWithThreshold * ExpectedMessageCount);

    auto args = streamFinishedSpy.first();
    QCOMPARE(args.count(), 1);
    QCOMPARE_EQ(args.first().value<QGrpcStatus>().code(), QtGrpc::StatusCode::Ok);

    QCOMPARE_EQ(fullResponse, "Stream11Stream22Stream33Stream44");
}

void QtGrpcClientBidiStreamTest::sequentialSendWithDone()
{
    const int ExpectedMessageCount = 4;

    SimpleStringMessage request;
    request.setTestFieldString("Stream");

    auto stream = client()->testMethodBiStreamWithDone(request);
    auto *streamPtr = stream.get();

    QString fullResponse;
    int i = 0;
    QObject::connect(streamPtr, &QGrpcBidiStream::messageReceived, this,
                     [stream = streamPtr, &request, &fullResponse, &i, ExpectedMessageCount]() {
                         Q_UNUSED(ExpectedMessageCount)
                         if (const auto rsp = stream->read<SimpleStringMessage>()) {
                             fullResponse += rsp->testFieldString() + QString::number(++i);
                             if (i == (ExpectedMessageCount - 1)) {
                                 stream->writesDone();
                                 request.setTestFieldString("StreamWrong");
                             }
                             stream->writeMessage(request);
                         }
                     });

    QSignalSpy streamFinishedSpy(streamPtr, &QGrpcServerStream::finished);
    QVERIFY(streamFinishedSpy.isValid());

    QTRY_COMPARE_EQ_WITH_TIMEOUT(streamFinishedSpy.count(), 1,
                                 MessageLatencyWithThreshold * ExpectedMessageCount);

    auto args = streamFinishedSpy.first();
    QCOMPARE(args.count(), 1);
    QCOMPARE_EQ(args.first().value<QGrpcStatus>().code(), QtGrpc::StatusCode::Ok);

    QCOMPARE_EQ(fullResponse, "Stream11Stream22Stream33");
}

void QtGrpcClientBidiStreamTest::multipleImmediateSendsWithDone()
{
    const int ExpectedMessageCount = 4;

    SimpleStringMessage request;
    request.setTestFieldString("Stream1");

    auto stream = client()->testMethodBiStreamWithDone(request);
    auto *streamPtr = stream.get();
    QString fullResponse;

    for (int i = 0; i < (ExpectedMessageCount - 1); ++i) {
        request.setTestFieldString("Stream" + QString::number(i + 2));
        stream->writeMessage(request);
    }

    int i = 1;
    QObject::connect(streamPtr, &QGrpcBidiStream::messageReceived, this,
                     [stream = streamPtr, &fullResponse, &i, ExpectedMessageCount]() {
                         Q_UNUSED(ExpectedMessageCount)
                         if (const auto rsp = stream->read<SimpleStringMessage>()) {
                             fullResponse += rsp->testFieldString();
                             ++i;
                             if (i == (ExpectedMessageCount - 1))
                                 stream->writesDone();
                         }
                     });

    QSignalSpy streamFinishedSpy(streamPtr, &QGrpcServerStream::finished);
    QVERIFY(streamFinishedSpy.isValid());

    QTRY_COMPARE_EQ_WITH_TIMEOUT(streamFinishedSpy.count(), 1,
                                 MessageLatencyWithThreshold * ExpectedMessageCount);

    auto args = streamFinishedSpy.first();
    QCOMPARE(args.count(), 1);
    QCOMPARE_EQ(args.first().value<QGrpcStatus>().code(), QtGrpc::StatusCode::Ok);

    QCOMPARE_EQ(fullResponse, "Stream11Stream22Stream33Stream44");
}

QTEST_MAIN(QtGrpcClientBidiStreamTest)

#include "tst_grpc_client_bidistream.moc"
