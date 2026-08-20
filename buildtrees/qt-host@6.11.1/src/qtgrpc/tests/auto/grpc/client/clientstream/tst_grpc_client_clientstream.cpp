// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
#include <grpcclienttestbase.h>

#include <QtCore/QTimer>
#include <QtTest/QTest>
#include <QtTest/QSignalSpy>

#include <testservice_client.grpc.qpb.h>
#include <message_latency_defs.h>

using namespace Qt::Literals::StringLiterals;
using namespace qtgrpc::tests;

class QtGrpcClientClientStreamTest : public GrpcClientTestBase
{
    Q_OBJECT
public:
    QtGrpcClientClientStreamTest()
        : GrpcClientTestBase(
                Channels{ GrpcClientTestBase::Channel::Qt })
    {
    }

private Q_SLOTS:
    void valid();
    void sequentialSend();
    void sequentialSendWithDone();
    void sequentialSendWithDoneWhenChannelNotReady();
};

void QtGrpcClientClientStreamTest::valid()
{
    const int ExpectedMessageCount = 4;

    SimpleStringMessage request;
    request.setTestFieldString("Stream1");

    auto stream = client()->testMethodClientStream(request);

    int i = 0;
    QTimer sendTimer;
    QObject::connect(&sendTimer, &QTimer::timeout, this, [&]() {
        request.setTestFieldString("Stream" + QString::number(i + 2));
        stream->writeMessage(request);
        if (++i == ExpectedMessageCount)
            sendTimer.stop();
    });

    sendTimer.start(MessageLatency);

    QSignalSpy streamFinishedSpy(stream.get(), &QGrpcServerStream::finished);
    QVERIFY(streamFinishedSpy.isValid());

    QTRY_COMPARE_EQ_WITH_TIMEOUT(streamFinishedSpy.count(), 1,
                                 MessageLatencyWithThreshold * ExpectedMessageCount);

    auto args = streamFinishedSpy.takeFirst();
    QCOMPARE(args.count(), 1);
    QVERIFY(args.takeFirst().value<QGrpcStatus>() == QtGrpc::StatusCode::Ok);

    const auto result = stream->read<SimpleStringMessage>();
    QCOMPARE_EQ(result->testFieldString(), "Stream11Stream22Stream33Stream44");
}

void QtGrpcClientClientStreamTest::sequentialSend()
{
    const int ExpectedMessageCount = 4;

    SimpleStringMessage request;
    request.setTestFieldString("Stream");

    auto stream = client()->testMethodClientStream(request);

    // Ensure that messages are not lost during the sequential sending right after the stream is
    // instanciated.
    for (int i = 0; i < ExpectedMessageCount; ++i) {
        stream->writeMessage(request);
    }

    QSignalSpy streamFinishedSpy(stream.get(), &QGrpcServerStream::finished);
    QVERIFY(streamFinishedSpy.isValid());

    QTRY_COMPARE_EQ_WITH_TIMEOUT(streamFinishedSpy.count(), 1,
                                 MessageLatencyWithThreshold * ExpectedMessageCount);

    auto args = streamFinishedSpy.takeFirst();
    QCOMPARE(args.count(), 1);
    QVERIFY(args.takeFirst().value<QGrpcStatus>() == QtGrpc::StatusCode::Ok);

    const auto result = stream->read<SimpleStringMessage>();
    QVERIFY(result.has_value());
    QCOMPARE_EQ(result->testFieldString(), "Stream1Stream2Stream3Stream4");
}

void QtGrpcClientClientStreamTest::sequentialSendWithDone()
{
    const int ExpectedMessageCount = 4;

    SimpleStringMessage request;
    request.setTestFieldString("Stream");

    auto stream = client()->testMethodClientStreamWithDone(request);

    // Ensure that messages are not lost during the sequential sending right after the stream is
    // instanciated.
    for (int i = 1; i < (ExpectedMessageCount - 1); ++i) {
        stream->writeMessage(request);
    }
    stream->writesDone();
    request.setTestFieldString("StreamWrong");
    stream->writeMessage(request);

    QSignalSpy streamFinishedSpy(stream.get(), &QGrpcServerStream::finished);
    QVERIFY(streamFinishedSpy.isValid());

    QTRY_COMPARE_EQ_WITH_TIMEOUT(streamFinishedSpy.count(), 1,
                                 MessageLatencyWithThreshold * ExpectedMessageCount);

    auto args = streamFinishedSpy.first();
    QCOMPARE(args.count(), 1);
    QCOMPARE_EQ(args.first().value<QGrpcStatus>().code(), QtGrpc::StatusCode::Ok);

    std::optional<SimpleStringMessage> result = stream->read<SimpleStringMessage>();
    QCOMPARE_EQ(result->testFieldString(), "Stream1Stream2Stream3");
}

void QtGrpcClientClientStreamTest::sequentialSendWithDoneWhenChannelNotReady()
{
    auto channel = std::shared_ptr<
        QAbstractGrpcChannel>(new QGrpcHttp2Channel(QUrl("http://localhost:50051",
                                                         QUrl::StrictMode)));
    auto client = std::make_shared<qtgrpc::tests::TestService::Client>();
    client->attachChannel(std::move(channel));

    const int ExpectedMessageCount = 4;

    SimpleStringMessage request;
    request.setTestFieldString("Stream1");

    auto stream = client->testMethodClientStreamWithDone(request);

    // Ensure that messages are not lost during the sequential sending right after the stream is
    // instanciated.
    for (int i = 1; i < (ExpectedMessageCount - 1); ++i) {
        request.setTestFieldString(QString("Stream") + QString::number(i + 1));
        stream->writeMessage(request);
    }
    stream->writesDone();
    request.setTestFieldString("StreamWrong");
    stream->writeMessage(request);

    QSignalSpy streamFinishedSpy(stream.get(), &QGrpcServerStream::finished);
    QVERIFY(streamFinishedSpy.isValid());

    QTRY_COMPARE_EQ_WITH_TIMEOUT(streamFinishedSpy.count(), 1,
                                 MessageLatencyWithThreshold * ExpectedMessageCount);

    auto args = streamFinishedSpy.first();
    QCOMPARE(args.count(), 1);
    QCOMPARE_EQ(args.first().value<QGrpcStatus>().code(), QtGrpc::StatusCode::Ok);

    std::optional<SimpleStringMessage> result = stream->read<SimpleStringMessage>();
    QCOMPARE_EQ(result->testFieldString(), "Stream11Stream22Stream33");
}

QTEST_MAIN(QtGrpcClientClientStreamTest)

#include "tst_grpc_client_clientstream.moc"
