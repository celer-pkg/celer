// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "interceptor_helper.h"
#include <mockserver.h>

#include <proto/server/interceptor1.grpc.pb.h>
#include <proto/server/interceptor1.pb.h>
#include <proto/server/interceptor2.grpc.pb.h>
#include <proto/server/interceptor2.pb.h>

#include <grpcpp/completion_queue.h>
#include <grpcpp/server_context.h>

#include <proto/client/interceptor1_client.grpc.qpb.h>
#include <proto/client/interceptor2_client.grpc.qpb.h>

#include <QtGrpc/qgrpcchanneloptions.h>
#include <QtGrpc/qgrpchttp2channel.h>
#include <QtGrpc/qgrpcinterceptorchain.h>

#include <QtProtobuf/qprotobufmessage.h>
#include <QtProtobuf/qprotobufserializer.h>

#include <QtTest/qsignalspy.h>
#include <QtTest/qtest.h>

#include <QtCore/qset.h>
#include <QtCore/qtimer.h>

#include <deque>
#include <string>
#include <vector>

#undef QTEST_FAIL_ACTION
#define QTEST_FAIL_ACTION                         \
    do {                                          \
        std::cerr << "Test failed!" << std::endl; \
        std::abort();                             \
    } while (0)

using namespace Qt::Literals::StringLiterals;

class QtGrpcClientInterceptorsTest : public QObject
{
    Q_OBJECT

public:
    static std::string serverAddress() { return "localhost:50051"; }

    static std::vector<ListeningPort> serverPorts()
    {
        return {
            { serverAddress(), grpc::InsecureServerCredentials() }
        };
    }

    static std::shared_ptr<QGrpcHttp2Channel> createChannel(QGrpcInterceptorChain interceptors)
    {
        QUrl url("http://"_L1 + QString::fromStdString(serverAddress()));
        return std::make_shared<QGrpcHttp2Channel>(url, QGrpcChannelOptions{},
                                                   std::move(interceptors));
    }

    enum class InterceptorUsage { Owning, NonOwning, Mixed };

private Q_SLOTS:
    void initTestCase_data() const
    {
        QTest::addColumn<InterceptorUsage>("interceptorUsage");
        QTest::newRow("InterceptorUsage::Owning") << InterceptorUsage::Owning;
        QTest::newRow("InterceptorUsage::NonOwning") << InterceptorUsage::NonOwning;
        QTest::newRow("InterceptorUsage::Mixed") << InterceptorUsage::Mixed;
    }
    void initTestCase()
    {
        QTest::failOnWarning();
        m_service1 = std::make_unique<tst::i1::Interceptor::AsyncService>();
        m_service2 = std::make_unique<tst::i2::Interceptor::AsyncService>();
        m_server = std::make_unique<MockServer>();
        QVERIFY(m_server->start(serverPorts(), { m_service1.get(), m_service2.get() }));
    }

    void cleanupTestCase()
    {
        QVERIFY(m_server->stop());
        m_service1.reset();
        m_service2.reset();
    }

    void init() { QVERIFY(m_service1 && m_service2); }
    void cleanup() { CallLog.clear(); }

    void unaryCallOrder();
    void bidiStreamCallOrder();
    void clientStreamCallOrder();
    void failedCallOrder();
    void cancelledCallOrder();

    void partialCapabilities();
    void onStartDrop();
    void onStartDropFromSecond();
    void modifyArguments();

    void interceptionContextAccessors();
    void interceptionContextUniqueId();

    void addInterceptorVariations();
    void removeAllInterceptorsMultiple();

private:
    struct UnaryHandler
    {
        grpc::ServerContext ctx;
        grpc::ServerAsyncResponseWriter<tst::i1::CallMessage> op{ &ctx };
        tst::i1::CallMessage request;
        tst::i1::CallMessage response;
    };

    struct BidiStreamHandler
    {
        grpc::ServerContext ctx;
        grpc::ServerAsyncReaderWriter<tst::i2::StreamMessage, tst::i2::StreamMessage> op{ &ctx };
        tst::i2::StreamMessage request;
        std::atomic<bool> writePending = { false };
        std::atomic<bool> finishRequested = { false };
        std::atomic<bool> finishStarted = { false };
        std::deque<tst::i2::StreamMessage> pendingMessages;

        std::function<void()> writeNext;
        std::function<void()> startFinish;
        CallbackTag *readerTag = nullptr;
    };

    struct ClientStreamHandler
    {
        grpc::ServerContext ctx;
        grpc::ServerAsyncReader<tst::i2::StreamMessage, tst::i2::StreamMessage> op{ &ctx };
        tst::i2::StreamMessage request;
        std::atomic<bool> isCancelled = false;
    };

    void setupUnaryEcho(std::unique_ptr<TagProcessor> &processor);
    void setupUnaryError(std::unique_ptr<TagProcessor> &processor, grpc::StatusCode code);
    void setupBidiStreamEcho(std::unique_ptr<TagProcessor> &processor);
    void setupClientStreamSink(std::unique_ptr<TagProcessor> &processor,
                               std::atomic<bool> &cancelled);

    std::unique_ptr<MockServer> m_server;
    std::unique_ptr<tst::i1::Interceptor::AsyncService> m_service1;
    std::unique_ptr<tst::i2::Interceptor::AsyncService> m_service2;
    qt::tst::i1::Interceptor::Client m_client1;
    qt::tst::i1::Interceptor::Client m_client1_1;
    qt::tst::i2::Interceptor::Client m_client2;
};

void QtGrpcClientInterceptorsTest::setupUnaryEcho(std::unique_ptr<TagProcessor> &processor)
{
    auto *data = new UnaryHandler;

    auto *handler = new CallbackTag(
        [data, &processor](bool ok) {
            QVERIFY(ok);
            data->response = data->request;
            data->op.Finish(data->response, grpc::Status::OK,
                            new DeleteTag<UnaryHandler>(data, processor.get()));
            return CallbackTag::Delete;
        },
        processor.get());

    m_service1->RequestUnary(&data->ctx, &data->request, &data->op, m_server->cq(), m_server->cq(),
                             handler);
}

void QtGrpcClientInterceptorsTest::setupUnaryError(std::unique_ptr<TagProcessor> &processor,
                                                   grpc::StatusCode code)
{
    auto *data = new UnaryHandler;

    auto *handler = new CallbackTag(
        [data, code, &processor](bool ok) {
            QVERIFY(ok);
            data->op.FinishWithError(grpc::Status(code, "error"),
                                     new DeleteTag<UnaryHandler>(data, processor.get()));
            return CallbackTag::Delete;
        },
        processor.get());

    m_service1->RequestUnary(&data->ctx, &data->request, &data->op, m_server->cq(), m_server->cq(),
                             handler);
}

void QtGrpcClientInterceptorsTest::setupBidiStreamEcho(std::unique_ptr<TagProcessor> &processor)
{
    auto *data = new BidiStreamHandler;

    auto *processorPtr = processor.get();

    data->startFinish = [data, processorPtr]() {
        if (data->finishStarted.exchange(true, std::memory_order_acq_rel))
            return;
        data->op.Finish(grpc::Status::OK, new DeleteTag<BidiStreamHandler>(data, processorPtr));
    };

    data->writeNext = [data, processorPtr]() {
        if (data->pendingMessages.empty()) {
            data->writePending.store(false, std::memory_order_release);
            if (data->finishRequested.load(std::memory_order_acquire))
                data->startFinish();
            return;
        }

        auto msg = std::move(data->pendingMessages.front());
        data->pendingMessages.pop_front();

        data->op.Write(msg,
                       new CallbackTag(
                           [data](bool ok) {
                               QVERIFY(ok);
                               (data->writeNext)();
                               return CallbackTag::Delete;
                           },
                           processorPtr));
    };

    data->readerTag = new CallbackTag(
        [data](bool ok) {
            if (!ok) {
                // Client closed its write side (or stream ended).
                // Defer Finish until any in-flight Write completes.
                data->finishRequested.store(true, std::memory_order_release);
                if (!data->writePending.load(std::memory_order_acquire))
                    data->startFinish();
                return CallbackTag::Delete;
            }

            data->pendingMessages.push_back(data->request);
            if (!data->writePending.exchange(true, std::memory_order_acq_rel))
                data->writeNext(); // start write loop if idle

            data->op.Read(&data->request, data->readerTag);
            return CallbackTag::Proceed;
        },
        processorPtr);

    auto *handler = new CallbackTag(
        [data](bool ok) {
            QVERIFY(ok);
            data->op.Read(&data->request, data->readerTag);
            return CallbackTag::Delete;
        },
        processorPtr);

    m_service2->RequestBidiStream(&data->ctx, &data->op, m_server->cq(), m_server->cq(), handler);
}

void QtGrpcClientInterceptorsTest::setupClientStreamSink(std::unique_ptr<TagProcessor> &processor,
                                                         std::atomic<bool> &cancelled)
{
    auto *data = new ClientStreamHandler;

    auto reader = std::make_shared<CallbackTag *>(nullptr);
    *reader = new CallbackTag(
        [data, &processor, reader](bool ok) {
            if (!ok) {
                if (!data->isCancelled) {
                    data->op.Finish(data->request, grpc::Status::OK,
                                    new DeleteTag<ClientStreamHandler>(data, processor.get()));
                }
                return CallbackTag::Delete;
            }
            data->op.Read(&data->request, *reader);
            return CallbackTag::Proceed;
        },
        processor.get());

    data->ctx.AsyncNotifyWhenDone(new CallbackTag(
        [&cancelled, data](bool ) {
            data->isCancelled = data->ctx.IsCancelled();
            cancelled = data->ctx.IsCancelled();
            return CallbackTag::Delete;
        },
        processor.get()));

    auto *handler = new CallbackTag(
        [data, reader, processor = processor.get()](bool ok) {
            QVERIFY(ok);
            data->op.Read(&data->request, *reader);
            data->ctx.AddInitialMetadata("test-key", "test-value");
            data->op.SendInitialMetadata(new VoidTag(processor));
            return CallbackTag::Delete;
        },
        processor.get());

    m_service2->RequestClientStream(&data->ctx, &data->op, m_server->cq(), m_server->cq(), handler);
}

void QtGrpcClientInterceptorsTest::unaryCallOrder()
{
    QFETCH_GLOBAL(const InterceptorUsage, interceptorUsage);

    auto processor = m_server->createProcessor();
    setupUnaryEcho(processor);

    auto i1 = std::make_unique<LoggingInterceptor>("A");
    auto i2 = std::make_unique<LoggingInterceptor>("B");
    QGrpcInterceptorChain interceptors;

    switch (interceptorUsage) {
    case InterceptorUsage::Owning:
        QVERIFY(interceptors.add(std::move(i1)));
        QVERIFY(interceptors.add(std::move(i2)));
        break;
    case InterceptorUsage::NonOwning:
        QVERIFY(interceptors.add(i1.get()));
        QVERIFY(interceptors.add(i2.get()));
        break;
    case InterceptorUsage::Mixed:
        QVERIFY(interceptors.add(std::move(i1)));
        QVERIFY(interceptors.add(i2.get()));
        break;
    }

    auto channel = createChannel(std::move(interceptors));
    m_client1.attachChannel(channel);

    auto reply = m_client1.Unary(qt::tst::i1::CallMessage{});
    QVERIFY(reply);

    QSignalSpy finishedSpy(reply.get(), &QGrpcCallReply::finished);
    QVERIFY(finishedSpy.wait());
    QVERIFY(reply->read<qt::tst::i1::CallMessage>().has_value());

    const QList<InterceptorCall> expected = {
        { "A", Capability::Start            },
        { "B", Capability::Start            },
        { "B", Capability::InitialMetadata  },
        { "A", Capability::InitialMetadata  },
        { "B", Capability::MessageReceived  },
        { "A", Capability::MessageReceived  },
        { "B", Capability::TrailingMetadata },
        { "A", Capability::TrailingMetadata },
        { "B", Capability::Finished         },
        { "A", Capability::Finished         },
    };
    QCOMPARE(CallLog, expected);
}

void QtGrpcClientInterceptorsTest::bidiStreamCallOrder()
{
    QFETCH_GLOBAL(const InterceptorUsage, interceptorUsage);

    auto processor = m_server->createProcessor();
    setupBidiStreamEcho(processor);

    auto interceptorA = std::make_unique<LoggingInterceptor>("A");
    auto interceptorB = std::make_unique<LoggingInterceptor>("B");
    QGrpcInterceptorChain interceptors;

    switch (interceptorUsage) {
    case InterceptorUsage::Owning:
        QVERIFY(interceptors.set(std::move(interceptorA), std::move(interceptorB)));
        break;
    case InterceptorUsage::NonOwning:
        QVERIFY(interceptors.set(interceptorA.get(), interceptorB.get()));
        break;
    case InterceptorUsage::Mixed:
        QVERIFY(interceptors.set(std::move(interceptorA)));
        QVERIFY(interceptors.add(interceptorB.get()));
        break;
    }

    auto channel = createChannel(std::move(interceptors));
    m_client2.attachChannel(channel);

    qt::tst::i2::StreamMessage msg;
    msg.setTag(1);

    auto stream = m_client2.BidiStream(msg);
    QVERIFY(stream);

    connect(stream.get(), &QGrpcBidiStream::messageReceived, stream.get(), [&]() {
        auto received = stream->read<qt::tst::i2::StreamMessage>();
        QVERIFY(received.has_value());
        if (received->tag() > 0) {
            received->setTag(received->tag() - 1);
            stream->writeMessage(*received);
        } else {
            stream->writesDone();
        }
    });

    QSignalSpy finishedSpy(stream.get(), &QGrpcOperation::finished);
    QVERIFY(finishedSpy.wait());

    const QList<InterceptorCall> expected = {
        { "A", Capability::Start            },
        { "B", Capability::Start            },
        { "B", Capability::InitialMetadata  },
        { "A", Capability::InitialMetadata  },
        { "B", Capability::MessageReceived  }, // receive tag=1
        { "A", Capability::MessageReceived  },
        { "A", Capability::WriteMessage     }, // send tag=0
        { "B", Capability::WriteMessage     },
        { "B", Capability::MessageReceived  }, // receive tag=0
        { "A", Capability::MessageReceived  },
        { "A", Capability::WritesDone       },
        { "B", Capability::WritesDone       },
        { "B", Capability::TrailingMetadata },
        { "A", Capability::TrailingMetadata },
        { "B", Capability::Finished         },
        { "A", Capability::Finished         },
    };
    QCOMPARE(CallLog, expected);
}

void QtGrpcClientInterceptorsTest::clientStreamCallOrder()
{
    QFETCH_GLOBAL(const InterceptorUsage, interceptorUsage);

    std::atomic<bool> cancelled{ false };
    auto processor = m_server->createProcessor();
    setupClientStreamSink(processor, cancelled);

    auto i1 = std::make_unique<LoggingInterceptor>("A");
    auto i2 = std::make_unique<LoggingInterceptor>("B");
    QGrpcInterceptorChain interceptors;

    switch (interceptorUsage) {
    case InterceptorUsage::Owning:
        QVERIFY(interceptors.set(std::move(i1), std::move(i2)));
        break;
    case InterceptorUsage::NonOwning:
        QVERIFY(interceptors.set(i1.get(), i2.get()));
        break;
    case InterceptorUsage::Mixed:
        QVERIFY(interceptors.set(i1.get()));
        QVERIFY(interceptors.add(std::move(i2)));
        break;
    }

    auto channel = createChannel(std::move(interceptors));
    m_client2.attachChannel(channel);

    auto stream = m_client2.ClientStream(qt::tst::i2::StreamMessage{});
    QVERIFY(stream);

    qt::tst::i2::StreamMessage msg;
    msg.setTag(1);
    stream->writeMessage(msg);
    msg.setTag(2);
    stream->writeMessage(msg);
    stream->writesDone();

    QSignalSpy finishedSpy(stream.get(), &QGrpcOperation::finished);
    QVERIFY(finishedSpy.wait());

    const QList<InterceptorCall> expected = {
        { "A", Capability::Start            },
        { "B", Capability::Start            },
        { "A", Capability::WriteMessage     },
        { "B", Capability::WriteMessage     },
        { "A", Capability::WriteMessage     },
        { "B", Capability::WriteMessage     },
        { "A", Capability::WritesDone       },
        { "B", Capability::WritesDone       },
        { "B", Capability::InitialMetadata  },
        { "A", Capability::InitialMetadata  },
        { "B", Capability::MessageReceived  },
        { "A", Capability::MessageReceived  },
        { "B", Capability::TrailingMetadata },
        { "A", Capability::TrailingMetadata },
        { "B", Capability::Finished         },
        { "A", Capability::Finished         },
    };
    QCOMPARE(CallLog, expected);
}

void QtGrpcClientInterceptorsTest::failedCallOrder()
{
    QFETCH_GLOBAL(const InterceptorUsage, interceptorUsage);

    auto processor = m_server->createProcessor();
    setupUnaryError(processor, grpc::CANCELLED);

    auto interceptorA = std::make_unique<LoggingInterceptor>("A");
    auto interceptorB = std::make_unique<LoggingInterceptor>("B");
    QGrpcInterceptorChain interceptors;

    switch (interceptorUsage) {
    case InterceptorUsage::Owning:
        QVERIFY(interceptors.add(std::move(interceptorA)));
        QVERIFY(interceptors.add(std::move(interceptorB)));
        break;
    case InterceptorUsage::NonOwning:
        QVERIFY(interceptors.add(interceptorA.get()));
        QVERIFY(interceptors.add(interceptorB.get()));
        break;
    case InterceptorUsage::Mixed:
        QVERIFY(interceptors.add(std::move(interceptorA)));
        QVERIFY(interceptors.add(interceptorB.get()));
        break;
    }

    auto channel = createChannel(std::move(interceptors));
    m_client1.attachChannel(channel);

    auto reply = m_client1.Unary(qt::tst::i1::CallMessage{});
    QVERIFY(reply);

    QSignalSpy finishedSpy(reply.get(), &QGrpcCallReply::finished);
    QVERIFY(finishedSpy.wait());

    auto args = finishedSpy.takeFirst();
    auto status = args.at(0).value<QGrpcStatus>();
    QCOMPARE(status.code(), QtGrpc::StatusCode::Cancelled);

    // Server error = trailers-only response
    const QList<InterceptorCall> expected = {
        { "A", Capability::Start            },
        { "B", Capability::Start            },
        { "B", Capability::TrailingMetadata },
        { "A", Capability::TrailingMetadata },
        { "B", Capability::Finished         },
        { "A", Capability::Finished         },
    };
    QCOMPARE(CallLog, expected);
}

void QtGrpcClientInterceptorsTest::cancelledCallOrder()
{
    QFETCH_GLOBAL(const InterceptorUsage, interceptorUsage);

    std::atomic<bool> serverCancelled{ false };
    auto processor = m_server->createProcessor();
    setupClientStreamSink(processor, serverCancelled);

    auto i1 = std::make_unique<LoggingInterceptor>("A");
    auto i2 = std::make_unique<LoggingInterceptor>("B");
    QGrpcInterceptorChain interceptors;

    switch (interceptorUsage) {
    case InterceptorUsage::Owning:
        QVERIFY(interceptors.set(std::move(i1), std::move(i2)));
        break;
    case InterceptorUsage::NonOwning:
        QVERIFY(interceptors.set(i1.get(), i2.get()));
        break;
    case InterceptorUsage::Mixed:
        QVERIFY(interceptors.add(i1.get()));
        QVERIFY(interceptors.add(std::move(i2)));
        break;
    }

    auto channel = createChannel(std::move(interceptors));
    m_client2.attachChannel(channel);

    auto stream = m_client2.ClientStream(qt::tst::i2::StreamMessage{});
    QVERIFY(stream);

    connect(stream.get(), &QGrpcOperation::serverInitialMetadataReceived, this,
            [&stream](const auto &) {
                // Headers must be received before cancelling
                QTimer::singleShot(0, [&]() {
                    qt::tst::i2::StreamMessage msg;
                    msg.setTag(1);
                    stream->writeMessage(msg);
                    stream->cancel();
                });
            });

    QSignalSpy finishedSpy(stream.get(), &QGrpcOperation::finished);
    QTRY_VERIFY_WITH_TIMEOUT(serverCancelled, 5000);
    QCOMPARE(finishedSpy.count(), 1);

    auto args = finishedSpy.takeFirst();
    auto status = args.at(0).value<QGrpcStatus>();
    QCOMPARE(status.code(), QtGrpc::StatusCode::Cancelled);

    const QList<InterceptorCall> expected = {
        { "A", Capability::Start           },
        { "B", Capability::Start           },
        { "B", Capability::InitialMetadata },
        { "A", Capability::InitialMetadata },
        { "A", Capability::WriteMessage    },
        { "B", Capability::WriteMessage    },
        { "A", Capability::Cancel          },
        { "B", Capability::Cancel          },
        { "B", Capability::Finished        },
        { "A", Capability::Finished        },
    };
    QCOMPARE(CallLog, expected);
}

void QtGrpcClientInterceptorsTest::partialCapabilities()
{
    QFETCH_GLOBAL(const InterceptorUsage, interceptorUsage);

    auto processor = m_server->createProcessor();
    setupUnaryEcho(processor);

    auto partial = std::make_unique<PartialInterceptor>("Partial");
    auto full = std::make_unique<LoggingInterceptor>("Full");
    QGrpcInterceptorChain interceptors;

    switch (interceptorUsage) {
    case InterceptorUsage::Owning:
        QVERIFY(interceptors.set(std::move(partial), std::move(full)));
        break;
    case InterceptorUsage::NonOwning:
        QVERIFY(interceptors.set(partial.get(), full.get()));
        break;
    case InterceptorUsage::Mixed:
        QVERIFY(interceptors.set(partial.get()));
        QVERIFY(interceptors.add(std::move(full)));
        break;
    }

    auto channel = createChannel(std::move(interceptors));
    m_client1.attachChannel(channel);

    auto reply = m_client1.Unary(qt::tst::i1::CallMessage{});
    QVERIFY(reply);

    QSignalSpy finishedSpy(reply.get(), &QGrpcCallReply::finished);
    QVERIFY(finishedSpy.wait());

    const QList<InterceptorCall> expected = {
        { "Partial", Capability::Start            },
        { "Full",    Capability::Start            },
        { "Full",    Capability::InitialMetadata  },
        { "Full",    Capability::MessageReceived  },
        { "Full",    Capability::TrailingMetadata },
        { "Full",    Capability::Finished         },
        { "Partial", Capability::Finished         },
    };
    QCOMPARE(CallLog, expected);
}

void QtGrpcClientInterceptorsTest::onStartDrop()
{
    QFETCH_GLOBAL(const InterceptorUsage, interceptorUsage);

    auto processor = m_server->createProcessor();
    setupUnaryEcho(processor);

    auto dropper = std::make_unique<DroppingInterceptor>("Drop", [](QtGrpc::RpcDescriptor desc) {
        if (desc.service == "tst.i1.Interceptor"_L1 && desc.method == "Unary"_L1
            && desc.type == QtGrpc::RpcType::UnaryCall) {
            return QGrpcStartInterceptor::Continuation::Proceed;
        }
        return QGrpcStartInterceptor::Continuation::Drop;
    });
    auto logger = std::make_unique<LoggingInterceptor>("After");
    QGrpcInterceptorChain interceptors;

    switch (interceptorUsage) {
    case InterceptorUsage::Owning:
        QVERIFY(interceptors.set(std::move(dropper), std::move(logger)));
        break;
    case InterceptorUsage::NonOwning:
        QVERIFY(interceptors.set(dropper.get(), logger.get()));
        break;
    case InterceptorUsage::Mixed:
        QVERIFY(interceptors.add(dropper.get()));
        QVERIFY(interceptors.add(std::move(logger)));
        break;
    }

    auto channel = createChannel(std::move(interceptors));
    m_client1.attachChannel(channel);
    m_client2.attachChannel(channel);

    {
        auto stream = m_client2.ClientStream(qt::tst::i2::StreamMessage{});
        QSignalSpy streamSpy(stream.get(), &QGrpcCallReply::finished);
        QVERIFY(streamSpy.isValid());
        QTRY_COMPARE(streamSpy.count(), 1);
        auto status = streamSpy.takeFirst().at(0).value<QGrpcStatus>();
        QCOMPARE_EQ(status.code(), QtGrpc::StatusCode::Aborted);
    }

    {
        auto stream = m_client2.BidiStream(qt::tst::i2::StreamMessage{});
        QSignalSpy streamSpy(stream.get(), &QGrpcCallReply::finished);
        QVERIFY(streamSpy.isValid());
        QTRY_COMPARE(streamSpy.count(), 1);
        auto status = streamSpy.takeFirst().at(0).value<QGrpcStatus>();
        QCOMPARE_EQ(status.code(), QtGrpc::StatusCode::Aborted);
    }

    {
        auto reply = m_client1.Unary(qt::tst::i1::CallMessage{});
        QSignalSpy replySpy(reply.get(), &QGrpcCallReply::finished);
        QVERIFY(replySpy.isValid());
        QVERIFY(replySpy.wait());
        auto status = replySpy.takeFirst().at(0).value<QGrpcStatus>();
        QCOMPARE_EQ(status.code(), QtGrpc::StatusCode::Ok);
    }

    const QList<InterceptorCall> expected = {
        { "Drop",  Capability::Start            }, // stream1 - fail
        { "After", Capability::Finished         },
        { "Drop",  Capability::Start            }, // stream2 - fail
        { "After", Capability::Finished         },

        { "Drop",  Capability::Start            }, // unary - success
        { "After", Capability::Start            },
        { "After", Capability::InitialMetadata  },
        { "After", Capability::MessageReceived  },
        { "After", Capability::TrailingMetadata },
        { "After", Capability::Finished         },
    };
    QCOMPARE(CallLog, expected);
}

void QtGrpcClientInterceptorsTest::onStartDropFromSecond()
{
    QFETCH_GLOBAL(const InterceptorUsage, interceptorUsage);

    auto processor = m_server->createProcessor();
    setupUnaryEcho(processor);

    auto first = std::make_unique<LoggingInterceptor>("First");
    QGrpcStartInterceptor::Continuation continuation = QGrpcStartInterceptor::Continuation::Drop;
    auto dropper = std::make_unique<DroppingInterceptor>("Drop", [&continuation](QtGrpc::RpcDescriptor) {
                                                             return continuation;
                                                         });
    auto third = std::make_unique<LoggingInterceptor>("Proceed");

    QGrpcInterceptorChain interceptors;

    switch (interceptorUsage) {
    case InterceptorUsage::Owning:
        QVERIFY(interceptors.set(std::move(first), std::move(dropper), std::move(third)));
        break;
    case InterceptorUsage::NonOwning:
        QVERIFY(interceptors.set(first.get(), dropper.get(), third.get()));
        break;
    case InterceptorUsage::Mixed:
        QVERIFY(interceptors.add(first.get()));
        QVERIFY(interceptors.add(std::move(dropper)));
        QVERIFY(interceptors.add(std::move(third)));
        break;
    }

    auto channel = createChannel(std::move(interceptors));
    m_client1.attachChannel(channel);

    const auto makeCall = [this](QtGrpc::StatusCode expectedCode) {
        auto reply = m_client1.Unary(qt::tst::i1::CallMessage{});
        QVERIFY(reply);
        QSignalSpy finishedSpy(reply.get(), &QGrpcCallReply::finished);
        QTRY_COMPARE(finishedSpy.count(), 1);
        auto status = finishedSpy.takeFirst().at(0).value<QGrpcStatus>();
        QCOMPARE(status.code(), expectedCode);
    };
    makeCall(QtGrpc::StatusCode::Aborted);
    continuation = QGrpcStartInterceptor::Continuation::Proceed;
    makeCall(QtGrpc::StatusCode::Ok);

    QByteArrayList onStartNames;
    for (const auto &call : std::as_const(CallLog)) {
        if (call.capability == Capability::Start)
            onStartNames.append(call.name);
    }
    QByteArrayList onStartExpected = {
        "First", "Drop",
        "First", "Drop", "Proceed"
    };
    QCOMPARE(onStartNames, onStartExpected);
}

void QtGrpcClientInterceptorsTest::modifyArguments()
{
    QFETCH_GLOBAL(const InterceptorUsage, interceptorUsage);

    const QByteArray clientMdKey = "client-key";
    const QByteArray serverInitialMdKey = "server-initial-key";
    const QByteArray serverTrailingMdKey = "server-trailing-key";

    auto processor = m_server->createProcessor();

    struct ModifyingBidiHandler
    {
        grpc::ServerContext ctx;
        grpc::ServerAsyncReaderWriter<tst::i2::StreamMessage, tst::i2::StreamMessage> op{ &ctx };
        tst::i2::StreamMessage request;
    };

    auto *data = new ModifyingBidiHandler;
    data->ctx.AddInitialMetadata(serverInitialMdKey.toStdString(), "Base");
    data->ctx.AddTrailingMetadata(serverTrailingMdKey.toStdString(), "Base");

    auto reader = std::make_shared<CallbackTag *>(nullptr);
    *reader = new CallbackTag(
        [data, &processor, reader](bool ok) {
            if (!ok) {
                data->op.Finish(grpc::Status::OK,
                                new DeleteTag<ModifyingBidiHandler>(data, processor.get()));
                return CallbackTag::Delete;
            }
            // Echo back
            data->op.Write(data->request,
                           new CallbackTag([](bool) { return CallbackTag::Delete; },
                                           processor.get()));
            data->op.Read(&data->request, *reader);
            return CallbackTag::Proceed;
        },
        processor.get());

    auto *handler = new CallbackTag(
        [data, reader](bool ok) {
            QVERIFY(ok);
            data->op.Read(&data->request, *reader);
            return CallbackTag::Delete;
        },
        processor.get());

    m_service2->RequestBidiStream(&data->ctx, &data->op, m_server->cq(), m_server->cq(), handler);

    class ModifyingInterceptor : public QGrpcStartInterceptor,
                                 public QGrpcInitialMetadataInterceptor,
                                 public QGrpcMessageReceivedInterceptor,
                                 public QGrpcWriteMessageInterceptor,
                                 public QGrpcTrailingMetadataInterceptor,
                                 public QGrpcFinishedInterceptor
    {
    public:
        explicit ModifyingInterceptor(QByteArray name) : m_name(std::move(name)) { }

        Continuation onStart(QGrpcInterceptionContext &, QProtobufMessage &message,
                             QGrpcCallOptions &opts) override
        {
            CallLog.push_back({ m_name, Capability::Start });
            if (auto *msg = qprotobufmessage_cast<qt::tst::i2::StreamMessage *>(&message))
                msg->setData(msg->data() + ';' + m_name);
            // Modify metadata
            auto md = opts.metadata(QtGrpc::MultiValue);
            for (auto it = md.begin(); it != md.end(); ++it)
                it.value() += ';' + m_name;
            opts.setMetadata(md);
            return Continuation::Proceed;
        }

        void onInitialMetadata(QGrpcInterceptionContext &,
                               QMultiHash<QByteArray, QByteArray> &metadata) override
        {
            CallLog.push_back({ m_name, Capability::InitialMetadata });
            for (auto it = metadata.begin(); it != metadata.end(); ++it)
                it.value() += ';' + m_name;
        }

        void onMessageReceived(QGrpcInterceptionContext &ctx, QByteArray &bytes) override
        {
            CallLog.push_back({ m_name, Capability::MessageReceived });
            qt::tst::i2::StreamMessage msg;
            ctx.channel().serializer()->deserialize(&msg, bytes);
            msg.setData(msg.data() + ';' + m_name);
            bytes = ctx.channel().serializer()->serialize(&msg);
        }

        void onWriteMessage(QGrpcInterceptionContext &, QProtobufMessage &message) override
        {
            CallLog.push_back({ m_name, Capability::WriteMessage });
            if (auto *msg = qprotobufmessage_cast<qt::tst::i2::StreamMessage *>(&message))
                msg->setData(msg->data() + ';' + m_name);
        }

        void onTrailingMetadata(QGrpcInterceptionContext &,
                                QMultiHash<QByteArray, QByteArray> &metadata) override
        {
            CallLog.push_back({ m_name, Capability::TrailingMetadata });
            for (auto it = metadata.begin(); it != metadata.end(); ++it)
                it.value() += ';' + m_name;
        }

        void onFinished(QGrpcInterceptionContext &, QGrpcStatus &status) override
        {
            CallLog.push_back({ m_name, Capability::Finished });
            status = QGrpcStatus(status.code(), status.message() + ';' + m_name);
        }

    private:
        QByteArray m_name;
    };

    auto modifyingInterceptor1 = std::make_unique<ModifyingInterceptor>("i1");
    auto modifyingInterceptor2 = std::make_unique<ModifyingInterceptor>("i2");
    QGrpcInterceptorChain interceptors;

    switch (interceptorUsage) {
    case InterceptorUsage::Owning:
        QVERIFY(interceptors.set(std::move(modifyingInterceptor1), std::move(modifyingInterceptor2)));
        break;
    case InterceptorUsage::NonOwning:
        QVERIFY(interceptors.set(modifyingInterceptor1.get(), modifyingInterceptor2.get()));
        break;
    case InterceptorUsage::Mixed:
        QVERIFY(interceptors.add(std::move(modifyingInterceptor1)));
        QVERIFY(interceptors.add(modifyingInterceptor2.get()));
        break;
    }

    auto channel = createChannel(std::move(interceptors));
    m_client2.attachChannel(channel);

    QGrpcCallOptions opts;
    opts.addMetadata(clientMdKey, "Base");

    qt::tst::i2::StreamMessage request;
    request.setData("Base");

    auto stream = m_client2.BidiStream(request, opts);
    QVERIFY(stream);

    bool receivedMessage = false;
    connect(stream.get(), &QGrpcBidiStream::messageReceived, stream.get(), [&]() {
        auto msg = stream->read<qt::tst::i2::StreamMessage>();
        QVERIFY(msg.has_value());
        QCOMPARE(msg->data(), "Base;i1;i2;i2;i1");
        receivedMessage = true;
        stream->writesDone();
    });

    QSignalSpy finishedSpy(stream.get(), &QGrpcOperation::finished);
    QVERIFY(finishedSpy.wait());
    QVERIFY(receivedMessage);

    auto initialMd = stream->serverInitialMetadata();
    QCOMPARE(initialMd.value(serverInitialMdKey), "Base;i2;i1");

    auto trailingMd = stream->serverTrailingMetadata();
    QCOMPARE(trailingMd.value(serverTrailingMdKey), "Base;i2;i1");
}

void QtGrpcClientInterceptorsTest::interceptionContextAccessors()
{
    QFETCH_GLOBAL(const InterceptorUsage, interceptorUsage);
    if (interceptorUsage == InterceptorUsage::Mixed)
        return;

    auto processor = m_server->createProcessor();
    setupUnaryEcho(processor);

    QGrpcCallOptions opts;
    opts.addMetadata("test-key", "test-value");
    opts.setDeadlineTimeout(std::chrono::seconds(30));

    auto i1 = std::make_unique<ContextVerifyingInterceptor>("Ctx"_ba);
    auto interceptor = i1.get();
    QGrpcInterceptorChain interceptors;

    switch (interceptorUsage) {
    case InterceptorUsage::Owning:
        QVERIFY(interceptors.add(std::move(i1)));
        break;
    case InterceptorUsage::NonOwning:
        QVERIFY(interceptors.add(i1.get()));
        break;
    case InterceptorUsage::Mixed:
        Q_UNREACHABLE();
        return;
    }

    auto channel = createChannel(std::move(interceptors));

    m_client1.attachChannel(channel);
    auto reply = m_client1.Unary(qt::tst::i1::CallMessage{}, opts);
    QVERIFY(reply);

    QSignalSpy finishedSpy(reply.get(), &QGrpcCallReply::finished);
    QVERIFY(finishedSpy.wait());

    QCOMPARE(interceptor->capturedDescriptor1->service, "tst.i1.Interceptor"_L1);
    QCOMPARE(interceptor->capturedDescriptor1->method, "Unary"_L1);
    QCOMPARE(interceptor->capturedDescriptor1->type, QtGrpc::RpcType::UnaryCall);

    QCOMPARE_EQ(*interceptor->capturedDescriptor1, *interceptor->capturedDescriptor2);
    QCOMPARE_EQ(*interceptor->capturedDescriptor1, *interceptor->capturedDescriptor3);

    QCOMPARE_EQ(interceptor->capturedCallOptions.deadlineTimeout(), opts.deadlineTimeout());
    QCOMPARE_EQ(interceptor->capturedCallOptions.metadata(QtGrpc::MultiValue),
                opts.metadata(QtGrpc::MultiValue));
}

void QtGrpcClientInterceptorsTest::interceptionContextUniqueId()
{
    QFETCH_GLOBAL(const InterceptorUsage, interceptorUsage);
    if (interceptorUsage == InterceptorUsage::Mixed)
        return;

    constexpr qsizetype NumCalls = 20 * 2; // 2 channels
    auto processor = m_server->createProcessor();
    for (int i = 0; i < NumCalls; ++i)
        setupUnaryEcho(processor);

    struct SharedData
    {
        QMap<QtGrpc::InterceptorCapability, QSet<quint64>> operationIds;
    };

    class IdCapturingInterceptor : public QGrpcStartInterceptor,
                                   public QGrpcInitialMetadataInterceptor,
                                   public QGrpcMessageReceivedInterceptor,
                                   public QGrpcTrailingMetadataInterceptor,
                                   public QGrpcFinishedInterceptor
    {
    public:
        IdCapturingInterceptor(std::shared_ptr<SharedData> sharedData_)
            : sharedData(std::move(sharedData_))
        {
        }

        Continuation onStart(QGrpcInterceptionContext &context, QProtobufMessage &,
                             QGrpcCallOptions &) override
        {
            insertOperationId(context, QtGrpc::InterceptorCapability::Start);
            return Continuation::Proceed;
        }

        void onInitialMetadata(QGrpcInterceptionContext &context,
                               QMultiHash<QByteArray, QByteArray> &) override
        {
            insertOperationId(context, QtGrpc::InterceptorCapability::InitialMetadata);
        }

        void onMessageReceived(QGrpcInterceptionContext &context, QByteArray &) override
        {
            insertOperationId(context, QtGrpc::InterceptorCapability::MessageReceived);
        }

        void onTrailingMetadata(QGrpcInterceptionContext &context,
                                QMultiHash<QByteArray, QByteArray> &) override
        {
            insertOperationId(context, QtGrpc::InterceptorCapability::TrailingMetadata);
        }

        void onFinished(QGrpcInterceptionContext &context, QGrpcStatus &) override
        {
            insertOperationId(context, QtGrpc::InterceptorCapability::Finished);
        }

    private:
        void insertOperationId(QGrpcInterceptionContext &context, QtGrpc::InterceptorCapability cap)
        {
            QVERIFY(!sharedData->operationIds[cap].contains(context.operationId()));
            sharedData->operationIds[cap].insert(context.operationId());
        }

        std::shared_ptr<SharedData> sharedData;
    };

    auto sharedData = std::make_shared<SharedData>();

    std::unique_ptr<IdCapturingInterceptor> nonOwningInterceptor;
    if (interceptorUsage == InterceptorUsage::NonOwning)
        nonOwningInterceptor = std::make_unique<IdCapturingInterceptor>(sharedData);

    auto createChain = [&]() {
        QGrpcInterceptorChain interceptors;
        if (interceptorUsage == InterceptorUsage::Owning)
            QVERIFY(interceptors.add(std::make_unique<IdCapturingInterceptor>(sharedData)));
        else
            QVERIFY(interceptors.add(nonOwningInterceptor.get()));
        return interceptors;
    };

    auto channel1 = createChannel(createChain());
    auto channel2 = createChannel(createChain());
    m_client1.attachChannel(channel1);
    m_client1_1.attachChannel(channel2);

    for (int i = 0; i < NumCalls / 2; ++i) {
        {
            auto reply = m_client1.Unary(qt::tst::i1::CallMessage{});
            QSignalSpy finishedSpy(reply.get(), &QGrpcCallReply::finished);
            QVERIFY(finishedSpy.wait());
        }
        {
            auto reply = m_client1_1.Unary(qt::tst::i1::CallMessage{});
            QSignalSpy finishedSpy(reply.get(), &QGrpcCallReply::finished);
            QVERIFY(finishedSpy.wait());
        }
    }

    const auto unaryCapabilities = {
        QtGrpc::InterceptorCapability::Start,
        QtGrpc::InterceptorCapability::InitialMetadata,
        QtGrpc::InterceptorCapability::MessageReceived,
        QtGrpc::InterceptorCapability::TrailingMetadata,
        QtGrpc::InterceptorCapability::Finished,
    };

    // Verify that each hook point contains the same operation IDs
    for (const auto c : unaryCapabilities) {
        QCOMPARE_EQ(sharedData->operationIds[c].size(), NumCalls);
        QCOMPARE_EQ(sharedData->operationIds[QtGrpc::InterceptorCapability::Start],
                    sharedData->operationIds[c]);
    }
}

void QtGrpcClientInterceptorsTest::addInterceptorVariations()
{
    QFETCH_GLOBAL(const InterceptorUsage, interceptorUsage);

    auto processor = m_server->createProcessor();
    setupUnaryEcho(processor);

    auto i1 = std::make_unique<LoggingInterceptor>("Single");
    auto i2 = std::make_unique<LoggingInterceptor>("V1");
    auto i3 = std::make_unique<LoggingInterceptor>("V2");
    auto i4 = std::make_unique<LoggingInterceptor>("V3");

    QGrpcInterceptorChain interceptors;

    switch (interceptorUsage) {
    case InterceptorUsage::Owning:
        QVERIFY(interceptors.set(std::move(i1), std::move(i2), std::move(i3), std::move(i4)));
        break;
    case InterceptorUsage::NonOwning:
        QVERIFY(interceptors.set(i1.get(), i2.get(), i3.get(), i4.get()));
        break;
    case InterceptorUsage::Mixed:
        QVERIFY(interceptors.add(std::move(i1)));
        QVERIFY(interceptors.add(i2.get()));
        QVERIFY(interceptors.add(std::move(i3)));
        QVERIFY(interceptors.add(i4.get()));
        break;
    }

    auto channel = createChannel(std::move(interceptors));

    m_client1.attachChannel(channel);

    auto reply = m_client1.Unary(qt::tst::i1::CallMessage{});
    QVERIFY(reply);

    QSignalSpy finishedSpy(reply.get(), &QGrpcCallReply::finished);
    QVERIFY(finishedSpy.wait());

    QByteArrayList startOrder;
    for (const auto &call : std::as_const(CallLog)) {
        if (call.capability == Capability::Start)
            startOrder.append(call.name);
    }
    QCOMPARE(startOrder, QByteArrayList({ "Single", "V1", "V2", "V3" }));
}

void QtGrpcClientInterceptorsTest::removeAllInterceptorsMultiple()
{
    QFETCH_GLOBAL(const InterceptorUsage, interceptorUsage);

    auto processor = m_server->createProcessor();
    setupUnaryEcho(processor);

    auto i1 = std::make_unique<LoggingInterceptor>("A");
    auto i2 = std::make_unique<LoggingInterceptor>("B");
    QGrpcInterceptorChain interceptors;

    switch (interceptorUsage) {
    case InterceptorUsage::Owning:
        QVERIFY(interceptors.add(std::move(i1)));
        QVERIFY(interceptors.add(std::move(i2)));
        break;
    case InterceptorUsage::NonOwning:
        QVERIFY(interceptors.add(i1.get()));
        QVERIFY(interceptors.add(i2.get()));
        break;
    case InterceptorUsage::Mixed:
        QVERIFY(interceptors.add(std::move(i1)));
        QVERIFY(interceptors.add(i2.get()));
        break;
    }

    interceptors.clear();

    auto i3 = std::make_unique<LoggingInterceptor>("New");
    if (interceptorUsage == InterceptorUsage::Owning)
        QVERIFY(interceptors.add(std::move(i3)));
    else
        QVERIFY(interceptors.add(i3.get()));

    auto channel = createChannel(std::move(interceptors));
    m_client1.attachChannel(channel);

    auto reply = m_client1.Unary(qt::tst::i1::CallMessage{});
    QVERIFY(reply);

    QSignalSpy finishedSpy(reply.get(), &QGrpcCallReply::finished);
    QVERIFY(finishedSpy.wait());

    QByteArrayList names;
    for (const auto &call : std::as_const(CallLog)) {
        if (call.capability == Capability::Start)
            names.append(call.name);
    }
    QCOMPARE(names, QByteArrayList{ "New" });
}

QTEST_MAIN(QtGrpcClientInterceptorsTest)

#include "tst_grpc_client_interceptors.moc"
