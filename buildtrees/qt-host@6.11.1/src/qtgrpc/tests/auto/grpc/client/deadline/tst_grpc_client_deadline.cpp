// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <grpcclienttestbase.h>
#include <message_latency_defs.h>
#include <testservice_client.grpc.qpb.h>

#include <QtGrpc/qgrpcstatus.h>

#include <QtTest/qsignalspy.h>
#include <QtTest/qtest.h>

using namespace Qt::Literals::StringLiterals;
using namespace qtgrpc::tests;

class QtGrpcClientDeadlineTest : public GrpcClientTestBase
{
    Q_OBJECT
public:
    QtGrpcClientDeadlineTest() : GrpcClientTestBase(Channels(Channel::WithChannelDeadline)) { }

private Q_SLOTS:
    // We use 'MessageLatency / 4' as channel latency
    void channelDeadlineCallExceeds();
    void channelDeadlineCallFinishes();
    void channelDeadlineServerStreamExceeds();
    void channelDeadlineServerStreamFinishes();
    void callDeadlineClientStreamExceeds();
    void callDeadlineClientStreamFinishes();
    void callDeadlineBiStreamExceeds();
    void callDeadlineBiStreamFinishes();

    void clientCancelBeforeTimeout();
};

void QtGrpcClientDeadlineTest::channelDeadlineCallExceeds()
{
    QThread::msleep(500);
    SleepMessage request;
    request.setSleepTimeMs(2 * MessageLatency);
    auto reply = client()->testMethodSleep(request);

    QSignalSpy finSpy(reply.get(), &QGrpcOperation::finished);
    QVERIFY(finSpy.isValid());

    QCOMPARE_EQ(finSpy.count(), 0);
    QVERIFY(finSpy.wait(3 * MessageLatency));
    QCOMPARE_EQ(finSpy.count(), 1);

    const auto code = qvariant_cast<QGrpcStatus>(finSpy.at(0).first());
    QCOMPARE_EQ(code.code(), QtGrpc::StatusCode::DeadlineExceeded);
    QVERIFY(!code.message().isEmpty());
}

void QtGrpcClientDeadlineTest::channelDeadlineCallFinishes()
{
    auto reply = client()->testMethodSleep(SleepMessage());
    QSignalSpy finSpy(reply.get(), &QGrpcOperation::finished);
    QVERIFY(finSpy.isValid());

    QCOMPARE_EQ(finSpy.count(), 0);
    QVERIFY(finSpy.wait(MessageLatency));
    QCOMPARE_EQ(finSpy.count(), 1);
}

void QtGrpcClientDeadlineTest::channelDeadlineServerStreamExceeds()
{
    ServerStreamSleepMessage request;
    request.setAmountResponses(5);
    auto sleepMsg = request.sleepMessage();
    sleepMsg.setSleepTimeMs(MessageLatency / 4);
    request.setSleepMessage(sleepMsg);

    auto stream = client()->testMethodServerStreamSleep(request);
    QSignalSpy finSpy(stream.get(), &QGrpcOperation::finished);
    QVERIFY(finSpy.isValid());

    QCOMPARE_EQ(finSpy.count(), 0);
    QVERIFY(finSpy.wait(3 * MessageLatency));
    QCOMPARE_EQ(finSpy.count(), 1);

    const auto code = qvariant_cast<QGrpcStatus>(finSpy.at(0).first());
    QCOMPARE_EQ(code.code(), QtGrpc::StatusCode::DeadlineExceeded);
}

void QtGrpcClientDeadlineTest::channelDeadlineServerStreamFinishes()
{
    ServerStreamSleepMessage request;
    request.setAmountResponses(5);
    auto stream = client()->testMethodServerStreamSleep(request);

    QSignalSpy finSpy(stream.get(), &QGrpcOperation::finished);
    QSignalSpy msgSpy(stream.get(), &QGrpcServerStream::messageReceived);
    QVERIFY(finSpy.isValid());
    QVERIFY(msgSpy.isValid());

    QCOMPARE_EQ(finSpy.count(), 0);
    QVERIFY(finSpy.wait(3 * MessageLatency));
    QCOMPARE_EQ(finSpy.count(), 1);

    QCOMPARE_EQ(msgSpy.count(), 5);
}

void QtGrpcClientDeadlineTest::callDeadlineClientStreamExceeds()
{
    SleepMessage request;
    request.setSleepTimeMs(50);
    QGrpcCallOptions opts;
    opts.setDeadlineTimeout({ std::chrono::milliseconds(50) });

    auto stream = client()->testMethodClientStreamSleep(request, opts);
    QSignalSpy finSpy(stream.get(), &QGrpcOperation::finished);
    QVERIFY(finSpy.isValid());
    QCOMPARE_EQ(finSpy.count(), 0);

    stream->writeMessage(request);
    stream->writeMessage(request);
    stream->writeMessage(request);
    QVERIFY(finSpy.wait(MessageLatency));
    QCOMPARE_EQ(finSpy.count(), 1);

    const auto code = qvariant_cast<QGrpcStatus>(finSpy.at(0).first());
    QCOMPARE_EQ(code.code(), QtGrpc::StatusCode::DeadlineExceeded);
}

void QtGrpcClientDeadlineTest::callDeadlineClientStreamFinishes()
{
    QGrpcCallOptions opts;
    opts.setDeadlineTimeout({ std::chrono::milliseconds(3 * MessageLatency) });
    SleepMessage request;

    auto stream = client()->testMethodClientStreamSleep(request, opts);
    QSignalSpy finSpy(stream.get(), &QGrpcOperation::finished);
    QVERIFY(finSpy.isValid());
    QCOMPARE_EQ(finSpy.count(), 0);

    stream->writesDone();
    QVERIFY(finSpy.wait(MessageLatency));
    QCOMPARE_EQ(finSpy.count(), 1);
}

void QtGrpcClientDeadlineTest::callDeadlineBiStreamExceeds()
{
    SleepMessage request;
    request.setSleepTimeMs(30);
    QGrpcCallOptions opts;
    opts.setDeadlineTimeout({ std::chrono::milliseconds(200) });

    auto stream = client()->testMethodBiStreamSleep(request, opts);
    QSignalSpy finSpy(stream.get(), &QGrpcOperation::finished);
    QSignalSpy msgSpy(stream.get(), &QGrpcBidiStream::messageReceived);
    QVERIFY(finSpy.isValid());
    QVERIFY(msgSpy.isValid());
    QCOMPARE_EQ(finSpy.count(), 0);

    stream->writeMessage(request);
    request.setSleepTimeMs(3000);
    stream->writeMessage(request);
    stream->writeMessage(request); // 4. msg
    QVERIFY(finSpy.wait(200 + MessageLatency));
    QCOMPARE_EQ(finSpy.count(), 1);

    const auto code = qvariant_cast<QGrpcStatus>(finSpy.at(0).first());
    QCOMPARE_EQ(code.code(), QtGrpc::StatusCode::DeadlineExceeded);
    QCOMPARE_EQ(msgSpy.count(), 2);
}

void QtGrpcClientDeadlineTest::callDeadlineBiStreamFinishes()
{
    QGrpcCallOptions opts;
    opts.setDeadlineTimeout({ std::chrono::milliseconds(400) });
    SleepMessage request;

    auto stream = client()->testMethodBiStreamSleep(request, opts);
    QSignalSpy finSpy(stream.get(), &QGrpcOperation::finished);
    QSignalSpy msgSpy(stream.get(), &QGrpcBidiStream::messageReceived);
    QVERIFY(finSpy.isValid());
    QVERIFY(msgSpy.isValid());
    QCOMPARE_EQ(finSpy.count(), 0);

    stream->writeMessage(request);
    stream->writeMessage(request);
    stream->writeMessage(request); // 4.msg
    stream->writesDone();
    QVERIFY(finSpy.wait(MessageLatency));
    QCOMPARE_EQ(finSpy.count(), 1);

    QCOMPARE_EQ(msgSpy.count(), 4);
}

void QtGrpcClientDeadlineTest::clientCancelBeforeTimeout()
{
    QGrpcCallOptions opts;
    opts.setDeadlineTimeout({ std::chrono::milliseconds(2000) });
    SleepMessage request;
    request.setSleepTimeMs(1500);

    auto reply = client()->testMethodSleep(request);
    QSignalSpy finSpy(reply.get(), &QGrpcOperation::finished);
    QVERIFY(finSpy.isValid());
    QCOMPARE_EQ(finSpy.count(), 0);

    QTRY_COMPARE_EQ_WITH_TIMEOUT(finSpy.count(), 0, 250);
    reply->cancel();
    QTRY_COMPARE_EQ_WITH_TIMEOUT(finSpy.count(), 1, 150);
    QCOMPARE_EQ(finSpy.count(), 1);

    const auto code = qvariant_cast<QGrpcStatus>(finSpy.at(0).first());
    QCOMPARE_EQ(code.code(), QtGrpc::StatusCode::Cancelled);
}

QTEST_MAIN(QtGrpcClientDeadlineTest)
#include "tst_grpc_client_deadline.moc"
