// Copyright (C) 2022 The Qt Company Ltd.
// Copyright (C) 2019 Alexey Edelev <semlanik@gmail.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <grpcclienttestbase.h>

#include <QtCore/QThread>
#include <QtCore/QTimer>
#include <QtCore/qbytearray.h>
#include <QtCore/qhash.h>

#include <QtGrpc/QGrpcCallOptions>

#include <QtTest/QSignalSpy>
#include <QtTest/QTest>

#include <memory>

#include <message_latency_defs.h>
#include <server_proc_runner.h>
#include <testservice_client.grpc.qpb.h>

using namespace Qt::Literals::StringLiterals;
using namespace qtgrpc::tests;

class QtGrpcClientUnaryCallTest : public GrpcClientTestBase
{
    Q_OBJECT
public:
    QtGrpcClientUnaryCallTest()
        : GrpcClientTestBase(GrpcClientTestBase::Channels{
            GrpcClientTestBase::Channel::Qt,
            GrpcClientTestBase::Channel::Json,
            #if !defined(Q_OS_DARWIN) && !defined(Q_OS_WIN32)
                            GrpcClientTestBase::Channel::Ssl,
            #endif
        })
    {
    }

private Q_SLOTS:
    void callIsValid();
    void immediateCancel();
    void deferredCancel();
    void asyncClientStatusMessage();
    void asyncStatusMessage();
#if QT_DEPRECATED_SINCE(6, 13)
    void deprecatedMetadata();
#endif
    void metadata_data();
    void metadata();
};

void QtGrpcClientUnaryCallTest::callIsValid()
{
    SimpleStringMessage request;
    request.setTestFieldString("Hello Qt!");

    std::shared_ptr<SimpleStringMessage> result = std::make_shared<SimpleStringMessage>();
    bool waitForReply = false;
    std::shared_ptr<QGrpcCallReply> reply = client()->testMethod(request);
    connect(
        reply.get(), &QGrpcOperation::finished, this,
        [&reply, resultWeak = std::weak_ptr(result), &waitForReply](const QGrpcStatus &status) {
            QCOMPARE_EQ(status.code(), QtGrpc::StatusCode::Ok);
            auto resultOpt = reply->read<SimpleStringMessage>();
            QVERIFY(resultOpt.has_value());
            if (auto resultVal = resultWeak.lock(); resultVal)
                *resultVal = resultOpt.value();
            waitForReply = true;
        },
        Qt::SingleShotConnection);

    QTRY_COMPARE_EQ_WITH_TIMEOUT(waitForReply, true, MessageLatency);
    QCOMPARE_EQ(result->testFieldString(), "Hello Qt!");
}

void QtGrpcClientUnaryCallTest::immediateCancel()
{
    SimpleStringMessage request;
    request.setTestFieldString("sleep");

    std::shared_ptr<QGrpcCallReply> reply = client()->testMethod(request);

    QSignalSpy replyFinishedSpy(reply.get(), &QGrpcCallReply::finished);
    QVERIFY(replyFinishedSpy.isValid());

    reply->cancel();

    QTRY_COMPARE_EQ_WITH_TIMEOUT(replyFinishedSpy.count(), 1, FailTimeout);

    QCOMPARE_EQ(qvariant_cast<QGrpcStatus>(replyFinishedSpy.at(0).first()).code(),
                QtGrpc::StatusCode::Cancelled);
}

void QtGrpcClientUnaryCallTest::deferredCancel()
{
    SimpleStringMessage request;
    request.setTestFieldString("sleep");

    std::shared_ptr<QGrpcCallReply> reply = client()->testMethod(request);

    QSignalSpy replyFinishedSpy(reply.get(), &QGrpcCallReply::finished);
    QVERIFY(replyFinishedSpy.isValid());

    QTimer::singleShot(MessageLatencyThreshold, reply.get(), &QGrpcCallReply::cancel);

    QTRY_COMPARE_EQ_WITH_TIMEOUT(replyFinishedSpy.count(), 1, FailTimeout);
    auto args = replyFinishedSpy.first();
    QCOMPARE(args.count(), 1);
    QCOMPARE_EQ(args.first().value<QGrpcStatus>().code(), QtGrpc::StatusCode::Cancelled);
}

void QtGrpcClientUnaryCallTest::asyncClientStatusMessage()
{
    SimpleStringMessage request;
    request.setTestFieldString("Some status message");

    auto reply = client()->testMethodStatusMessage(request);

    QSignalSpy replyFinishedSpy(reply.get(), &QGrpcCallReply::finished);
    QVERIFY(replyFinishedSpy.isValid());

    QTRY_COMPARE_EQ_WITH_TIMEOUT(replyFinishedSpy.count(), 1, FailTimeout);

    QCOMPARE(qvariant_cast<QGrpcStatus>(replyFinishedSpy.at(0).first()).message(),
             request.testFieldString());
}

void QtGrpcClientUnaryCallTest::asyncStatusMessage()
{
    SimpleStringMessage request;
    request.setTestFieldString("Some status message");

    std::shared_ptr<QGrpcCallReply> reply = client()->testMethodStatusMessage(request);

    QSignalSpy replyFinishedSpy(reply.get(), &QGrpcCallReply::finished);
    QVERIFY(replyFinishedSpy.isValid());

    QTRY_COMPARE_EQ_WITH_TIMEOUT(replyFinishedSpy.count(), 1, FailTimeout);
    auto args = replyFinishedSpy.first();
    QCOMPARE(args.count(), 1);
    QCOMPARE(args.first().value<QGrpcStatus>().message(), request.testFieldString());
}

#if QT_DEPRECATED_SINCE(6, 13)

void QtGrpcClientUnaryCallTest::deprecatedMetadata()
{
    QGrpcCallOptions opt;
    QHash<QByteArray, QByteArray> clientMd{
        { "request_initial",  "3"  },
        { "request_trailing", "2"  },
        { "request_sum",      "20" },
        { "request_sum",      "10" }
    };
    QT_IGNORE_DEPRECATIONS(opt.setMetadata(clientMd);)
    auto reply = client()->testMetadata({}, opt);
    QSignalSpy replyFinishedSpy(reply.get(), &QGrpcCallReply::finished);
    QVERIFY(replyFinishedSpy.isValid());

    QTRY_COMPARE_EQ_WITH_TIMEOUT(replyFinishedSpy.count(), 1, FailTimeout);
    const auto &args = replyFinishedSpy.first();
    QCOMPARE_EQ(args.count(), 1);
    QVERIFY(args.first().value<QGrpcStatus>().isOk());

    QT_IGNORE_DEPRECATIONS(const auto &md = reply->metadata();)
    auto initialIt = md.equal_range("response_initial");
    QCOMPARE_EQ(std::distance(initialIt.first, initialIt.second), 1);
    QCOMPARE_EQ(initialIt.first.value(), "2");
}

#endif // QT_DEPRECATED_SINCE(6, 13)

void QtGrpcClientUnaryCallTest::metadata_data()
{
    QTest::addColumn<bool>("filterServerMetadata");
    QTest::addRow("filterServerMetadata(true)") << true;
    QTest::addRow("filterServerMetadata(false)") << false;
}

void QtGrpcClientUnaryCallTest::metadata()
{
    // TODO: QTBUG-138407
    QFETCH(const bool, filterServerMetadata);
    QGrpcCallOptions opt;
    QMultiHash<QByteArray, QByteArray> clientMd{
        { "request_initial",  "3"  },
        { "request_trailing", "2"  },
        { "request_sum",      "20" },
        { "request_sum",      "10" }
    };
    opt.setFilterServerMetadata(filterServerMetadata);
    opt.setMetadata(clientMd);
    auto reply = client()->testMetadata({}, opt);
    QSignalSpy replyFinishedSpy(reply.get(), &QGrpcCallReply::finished);
    QVERIFY(replyFinishedSpy.isValid());

    QTRY_COMPARE_EQ_WITH_TIMEOUT(replyFinishedSpy.count(), 1, FailTimeout);
    const auto &args = replyFinishedSpy.first();
    QCOMPARE_EQ(args.count(), 1);
    QVERIFY(args.first().value<QGrpcStatus>().isOk());

    const auto &initialMd = reply->serverInitialMetadata();
    if (filterServerMetadata)
        QCOMPARE_EQ(initialMd.size(), 3);
    else
        QCOMPARE_GT(initialMd.size(), 3);
    auto initialIt = initialMd.equal_range("response_initial");
    QCOMPARE_EQ(std::distance(initialIt.first, initialIt.second), 3);
    QCOMPARE_EQ(initialIt.first.value(), "2");
    std::advance(initialIt.first, 1);
    QCOMPARE_EQ(initialIt.first.value(), "1");
    std::advance(initialIt.first, 1);
    QCOMPARE_EQ(initialIt.first.value(), "0");

    const auto &trailingMd = reply->serverTrailingMetadata();
    if (filterServerMetadata)
        QCOMPARE_EQ(trailingMd.size(), 3);
    else
        QCOMPARE_GT(trailingMd.size(), 3);
    auto trailingIt = trailingMd.equal_range("response_trailing");
    QCOMPARE_EQ(std::distance(trailingIt.first, trailingIt.second), 2);
    QCOMPARE_EQ(trailingIt.first.value(), "1");
    std::advance(trailingIt.first, 1);
    QCOMPARE_EQ(trailingIt.first.value(), "0");

    auto sumIt = trailingMd.equal_range("response_sum");
    QCOMPARE_EQ(std::distance(sumIt.first, sumIt.second), 1);
    QCOMPARE_EQ(sumIt.first.value(), "30"_ba);
}

QTEST_MAIN(QtGrpcClientUnaryCallTest)

#include "tst_grpc_client_unarycall.moc"
