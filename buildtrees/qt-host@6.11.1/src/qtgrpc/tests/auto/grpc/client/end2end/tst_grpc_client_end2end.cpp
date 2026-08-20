// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <mockserver.h>

#include <proto/server/event.pb.h>
#include <proto/server/eventhub.grpc.pb.h>

#include <proto/client/event.qpb.h>
#include <proto/client/eventhub_client.grpc.qpb.h>

#include <QtGrpc/qgrpccalloptions.h>
#include <QtGrpc/qgrpcchanneloptions.h>
#include <QtGrpc/qgrpchttp2channel.h>

#include <QtTest/qsignalspy.h>
#include <QtTest/qtest.h>

#include <QtCore/qbytearray.h>
#include <QtCore/qdatetime.h>
#include <QtCore/qhash.h>
#include <QtCore/qtimer.h>

#include <atomic>
#include <string>
#include <vector>

// Re-define so that we can use QTest Macros inside non void functions too.
#undef QTEST_FAIL_ACTION
#define QTEST_FAIL_ACTION                         \
    do {                                          \
        std::cerr << "Test failed!" << std::endl; \
        std::abort();                             \
    } while (0)

using namespace Qt::Literals::StringLiterals;
using MultiHash = QMultiHash<QByteArray, QByteArray>;

class QtGrpcClientEnd2EndTest : public QObject
{
    Q_OBJECT

public:
    static std::string serverHttpAddress() { return "localhost:50051"; }
    static std::string serverHttpsAddress() { return "localhost:50052"; }
    static std::string serverUnixAddress() { return "unix:///tmp/qtgrpc_test_end2end.sock"; }
    static std::string serverUnixAbstractAddress() { return "unix-abstract:qtgrpc_test_end2end"; }
    static std::vector<ListeningPort> serverListeningPorts()
    {
        return {
            { serverHttpAddress(),         grpc::InsecureServerCredentials() },
#if QT_CONFIG(ssl)
            { serverHttpsAddress(),        serverSslCredentials()            },
#endif
#ifdef Q_OS_UNIX
            { serverUnixAddress(),         grpc::InsecureServerCredentials() },
#endif

#ifdef Q_OS_LINUX
            { serverUnixAbstractAddress(), grpc::InsecureServerCredentials() },
#endif
        };
    }

private Q_SLOTS:
    void initTestCase_data() const;
    void initTestCase();
    void cleanupTestCase();

    void init();
    void cleanup();

    // Testcases:
    void clientMetadataReceived_data() const;
    void clientMetadataReceived();
    void serverMetadataReceived_data() const;
    void serverMetadataReceived();
    void serverInitialMetadataEmitted();

    void bidiStreamsInOrder();

    void clientHandlesCompression_data() const;
    void clientHandlesCompression();

    void channelChangeCancelsInFlightRPCs_data() const;
    void channelChangeCancelsInFlightRPCs();

private:
    static std::shared_ptr<grpc::ServerCredentials> serverSslCredentials()
    {
        grpc::SslServerCredentialsOptions opts(GRPC_SSL_DONT_REQUEST_CLIENT_CERTIFICATE);
        opts.pem_key_cert_pairs.push_back({ SslKey, SslCert });
        return grpc::SslServerCredentials(opts);
    }

private:
    std::unique_ptr<MockServer> m_server;
    std::unique_ptr<EventHub::AsyncService> m_service;
    std::unique_ptr<qt::EventHub::Client> m_client;
};

void QtGrpcClientEnd2EndTest::initTestCase_data() const
{
    QTest::addColumn<std::shared_ptr<QGrpcHttp2Channel>>("channel");

    QUrl httpAddress("http://"_ba + QByteArrayView(serverHttpAddress()));
    QTest::newRow("http") << std::make_shared<QGrpcHttp2Channel>(httpAddress);

#if QT_CONFIG(ssl)
    QSslConfiguration tlsConfig;
    tlsConfig.setProtocol(QSsl::TlsV1_2);
    tlsConfig.setCaCertificates({ QSslCertificate{ QByteArray(SslCert) } });
    tlsConfig.setAllowedNextProtocols({ "h2"_ba });
    QGrpcChannelOptions chOpts;
    chOpts.setSslConfiguration(tlsConfig);
    QUrl httpsAddress("https://"_ba + QByteArrayView(serverHttpsAddress()));
    QTest::newRow("https") << std::make_shared<QGrpcHttp2Channel>(httpsAddress, chOpts);
#endif

#ifdef Q_OS_UNIX
    QUrl unixAddress(serverUnixAddress().data());
    QTest::newRow("unix") << std::make_shared<QGrpcHttp2Channel>(unixAddress);
#endif

#ifdef Q_OS_LINUX
    QUrl unixAbstractAddress(serverUnixAbstractAddress().data());
    QTest::newRow("unix-abstract") << std::make_shared<QGrpcHttp2Channel>(unixAbstractAddress);
#endif
}

void QtGrpcClientEnd2EndTest::initTestCase()
{
    QTest::failOnWarning();
    m_service = std::make_unique<EventHub::AsyncService>();
    m_server = std::make_unique<MockServer>();
    QVERIFY(m_server->start(serverListeningPorts(), { m_service.get() }));
}

void QtGrpcClientEnd2EndTest::cleanupTestCase()
{
    m_client.reset();
    QVERIFY(m_server->stop());
    m_service.reset();
}

void QtGrpcClientEnd2EndTest::init()
{
    QVERIFY(m_service && m_server);
    QFETCH_GLOBAL(std::shared_ptr<QGrpcHttp2Channel>, channel);
    m_client = std::make_unique<qt::EventHub::Client>();
    QVERIFY(m_client->attachChannel(channel));
}

void QtGrpcClientEnd2EndTest::cleanup()
{
    m_client.reset();
}

void QtGrpcClientEnd2EndTest::clientMetadataReceived_data() const
{
    QTest::addColumn<MultiHash>("callMetadata");
    QTest::addColumn<MultiHash>("channelMetadata");
    MultiHash callMd{
        { "client-call-single", "call-value-1" },
        { "client-call-multi",  "call-a"       },
        { "client-call-multi",  "call-b"       }
    };
    MultiHash channelMd{
        { "client-channel-single", "channel-value-1" },
        { "client-channel-multi",  "channel-a"       },
        { "client-channel-multi",  "channel-b"       }
    };
    QTest::addRow("call") << callMd << MultiHash{};
    QTest::addRow("channel") << MultiHash{} << channelMd;
    QTest::addRow("call+channel") << callMd << channelMd;
}

void QtGrpcClientEnd2EndTest::clientMetadataReceived()
{
    QFETCH(const MultiHash, callMetadata);
    QFETCH(const MultiHash, channelMetadata);

    // Setup Server-side handling
    auto processor = m_server->createProcessor();
    struct ServerData
    {
        grpc::ServerAsyncResponseWriter<None> op{ &ctx };
        grpc::ServerContext ctx;

        Event request;
        None response;
    };
    ServerData *data = new ServerData;

    CallbackTag *callHandler = new CallbackTag(
        [&](bool ok) {
            QVERIFY(ok);

            const std::multimap<grpc::string_ref, grpc::string_ref>
                &receivedMd = data->ctx.client_metadata();
            auto mergedMd = channelMetadata;
            mergedMd.unite(callMetadata);

            for (auto it = mergedMd.cbegin(); it != mergedMd.cend(); ++it) {
                // Check that each key-value pair sent by the client exists on the server
                auto serverRange = receivedMd.equal_range(it.key().toStdString());
                auto clientRange = mergedMd.equal_range(it.key());

                QCOMPARE_EQ(std::distance(serverRange.first, serverRange.second),
                            std::distance(clientRange.first, clientRange.second));
                while (clientRange.first != clientRange.second) {
                    // Look for the exact entry in the server range. The order may
                    // be changed but it must be present.
                    const auto it = std::find_if(serverRange.first, serverRange.second,
                                                 [&](auto it) {
                                                     return it.first
                                                         == clientRange.first.key().toStdString()
                                                         && it.second
                                                         == clientRange.first.value().toStdString();
                                                 });
                    QVERIFY(it != serverRange.second);
                    std::advance(clientRange.first, 1);
                }
            }
            data->op.Finish(data->response, grpc::Status::OK,
                            new DeleteTag<ServerData>(data, processor.get()));
            return CallbackTag::Delete;
        },
        processor.get());
    m_service->RequestPush(&data->ctx, &data->request, &data->op, m_server->cq(), m_server->cq(),
                           callHandler);

    // Setup Client-side call
    m_client->channel()->setChannelOptions(QGrpcChannelOptions().setMetadata(channelMetadata));
    auto call = m_client->Push(qt::Event{}, QGrpcCallOptions().setMetadata(callMetadata));
    QVERIFY(call);

    connect(call.get(), &QGrpcOperation::finished, this, [&](const QGrpcStatus &status) {
        QVERIFY(status.isOk());
        auto response = call->read<qt::None>();
        QVERIFY(response.has_value());
    });

    QSignalSpy finishedSpy(call.get(), &QGrpcOperation::finished);
    QVERIFY(finishedSpy.isValid());
    QVERIFY(finishedSpy.wait());
}

void QtGrpcClientEnd2EndTest::serverMetadataReceived_data() const
{
    QTest::addColumn<bool>("filterServerMetadata");
    QTest::addColumn<MultiHash>("expectedInitialMd");
    QTest::addColumn<MultiHash>("expectedTrailingMd");

    MultiHash initialMd{
        { "initial-1", "ivalue-1" },
        { "initial-2", "ivalue-2" }
    };
    MultiHash trailingMd{
        { "trailing-1",     "tvalue-1" },
        { "trailing-multi", "tvalue-x" },
        { "trailing-multi", "tvalue-y" }
    };

    QTest::addRow("filter(true)") << true << initialMd << trailingMd;
    QTest::addRow("filter(false)") << false << initialMd << trailingMd;
}

void QtGrpcClientEnd2EndTest::serverMetadataReceived()
{
    using MultiHash = QMultiHash<QByteArray, QByteArray>;
    QFETCH(const bool, filterServerMetadata);
    QFETCH(const MultiHash, expectedInitialMd);
    QFETCH(const MultiHash, expectedTrailingMd);

    // Setup Server-side handling
    auto processor = m_server->createProcessor();
    struct ServerData
    {
        grpc::ServerAsyncResponseWriter<None> op{ &ctx };
        grpc::ServerContext ctx;

        Event request;
        None response;
    };
    ServerData *data = new ServerData;

    for (auto it = expectedInitialMd.cbegin(); it != expectedInitialMd.cend(); ++it)
        data->ctx.AddInitialMetadata(it.key().toStdString(), it.value().toStdString());
    for (auto it = expectedTrailingMd.cbegin(); it != expectedTrailingMd.cend(); ++it)
        data->ctx.AddTrailingMetadata(it.key().toStdString(), it.value().toStdString());

    CallbackTag *callHandler = new CallbackTag(
        [&](bool ok) {
            QVERIFY(ok);
            data->op.Finish(data->response, grpc::Status::OK,
                            new DeleteTag<ServerData>(data, processor.get()));
            return CallbackTag::Delete;
        },
        processor.get());
    m_service->RequestPush(&data->ctx, &data->request, &data->op, m_server->cq(), m_server->cq(),
                           callHandler);

    // Setup Client-side call
    auto chOpts = QGrpcChannelOptions().setFilterServerMetadata(filterServerMetadata);
    m_client->channel()->setChannelOptions(chOpts);

    auto call = m_client->Push(qt::Event{});
    QVERIFY(call);

    connect(call.get(), &QGrpcOperation::finished, this, [&](const QGrpcStatus &status) {
        QVERIFY(status.isOk());
        auto response = call->read<qt::None>();
        QVERIFY(response.has_value());

        const auto &initialMd = call->serverInitialMetadata();
        const auto &trailingMd = call->serverTrailingMetadata();

        if (filterServerMetadata) {
            QCOMPARE(initialMd, expectedInitialMd);
            QCOMPARE(trailingMd, expectedTrailingMd);
        } else {
            QCOMPARE_GE(initialMd.size(), expectedInitialMd.size());
            QCOMPARE_GE(trailingMd.size(), expectedTrailingMd.size());
            for (auto it = expectedInitialMd.cbegin(); it != expectedInitialMd.cend(); ++it)
                QVERIFY(initialMd.contains(it.key(), it.value()));
            for (auto it = expectedTrailingMd.cbegin(); it != expectedTrailingMd.cend(); ++it)
                QVERIFY(trailingMd.contains(it.key(), it.value()));
        }
    });

    QSignalSpy finishedSpy(call.get(), &QGrpcOperation::finished);
    QVERIFY(finishedSpy.isValid());
    QVERIFY(finishedSpy.wait());
}

void QtGrpcClientEnd2EndTest::serverInitialMetadataEmitted()
{
    // Setup Server-side handling
    auto processor = m_server->createProcessor();
    struct ServerData
    {
        grpc::ServerAsyncResponseWriter<None> op{ &ctx };
        grpc::ServerContext ctx;

        Event request;
        None response;
    };
    ServerData *data = new ServerData;
    data->ctx.AddInitialMetadata("initial", "value");
    data->ctx.AddTrailingMetadata("trailing", "value");

    CallbackTag *callHandler = new CallbackTag(
        [&](bool ok) {
            QVERIFY(ok);
            data->op.SendInitialMetadata(new CallbackTag(
                [&](bool ok) {
                    QVERIFY(ok);
                    // Wait one second before emitting finished.
                    std::this_thread::sleep_for(std::chrono::seconds(1));
                    data->op.Finish(data->response, grpc::Status::OK,
                                    new DeleteTag<ServerData>(data, processor.get()));
                    return CallbackTag::Delete;
                },
                processor.get()));
            return CallbackTag::Delete;
        },
        processor.get());
    m_service->RequestPush(&data->ctx, &data->request, &data->op, m_server->cq(), m_server->cq(),
                           callHandler);

    // Setup Client-side call
    QDateTime initialMetadataTime;
    QDateTime finishedTime;
    auto call = m_client->Push(qt::Event{}, QGrpcCallOptions{}.setFilterServerMetadata(true));
    QVERIFY(call);

    connect(call.get(), &QGrpcOperation::finished, this, [&](const QGrpcStatus &status) {
        finishedTime = QDateTime::currentDateTime();
        QVERIFY(status.isOk());
        QCOMPARE_EQ(call->serverTrailingMetadata().size(), 1);
    });
    connect(call.get(), &QGrpcOperation::serverInitialMetadataReceived, this,
            [&](const QMultiHash<QByteArray, QByteArray> &metadata) {
                initialMetadataTime = QDateTime::currentDateTime();
                QCOMPARE_EQ(call->serverInitialMetadata().size(), 1);
                QCOMPARE_EQ(call->serverInitialMetadata(), metadata);
            });

    QSignalSpy finishedSpy(call.get(), &QGrpcOperation::finished);
    QVERIFY(finishedSpy.isValid());
    QSignalSpy initialMetadataSpy(call.get(), &QGrpcOperation::serverInitialMetadataReceived);
    QVERIFY(initialMetadataSpy.isValid());

    finishedSpy.wait();

    QCOMPARE_EQ(initialMetadataSpy.count(), 1);
    QCOMPARE_LT(initialMetadataTime, finishedTime);
}

void QtGrpcClientEnd2EndTest::bidiStreamsInOrder()
{
    constexpr auto SleepTime = std::chrono::milliseconds(5);

    // Setup Server-side handling
    auto processor = m_server->createProcessor();
    struct ServerData
    {
        grpc::ServerAsyncReaderWriter<Event, Event> op{ &ctx };
        grpc::ServerContext ctx;

        Event request;
        Event response;
        unsigned long count = 0;
        std::atomic<bool> readerDone = false;
        std::atomic<bool> writerDone = false;

        void updateResponse()
        {
            response.set_type(Event::SERVER);
            response.set_number(response.number() + 1);
            response.set_name("server-" + std::to_string(response.number()));
        }
    };
    ServerData *data = new ServerData;

    CallbackTag *reader = new CallbackTag(
        [&, current = 1u](bool ok) mutable {
            if (!ok) {
                data->readerDone = true;
                if (data->writerDone)
                    data->op.Finish(grpc::Status::OK,
                                    new DeleteTag<ServerData>(data, processor.get()));
                return CallbackTag::Delete;
            }
            QCOMPARE_EQ(data->request.type(), Event::CLIENT);
            QCOMPARE_EQ(data->request.number(), current);
            std::string name = "client-" + std::to_string(current);
            QCOMPARE_EQ(data->request.name(), name);
            ++current;

            data->op.Read(&data->request, reader);
            return CallbackTag::Proceed;
        },
        processor.get());
    CallbackTag *writer = new CallbackTag(
        [&](bool ok) {
            QVERIFY(ok);
            if (data->response.number() >= data->count) {
                data->writerDone = true;
                if (data->readerDone)
                    data->op.Finish(grpc::Status::OK,
                                    new DeleteTag<ServerData>(data, processor.get()));
                return CallbackTag::Delete;
            }
            std::this_thread::sleep_for(SleepTime);
            data->updateResponse();
            data->op.Write(data->response, writer);
            return CallbackTag::Proceed;
        },
        processor.get());
    CallbackTag *callHandler = new CallbackTag(
        [&](bool ok) {
            QVERIFY(ok);
            const auto &md = data->ctx.client_metadata();
            const auto countIt = md.find("call-count");
            QVERIFY(countIt != md.cend());
            data->count = std::stoul(std::string(countIt->second.data(), countIt->second.length()));
            QCOMPARE_GT(data->count, 0u);

            data->op.Read(&data->request, reader);
            data->updateResponse();
            data->op.Write(data->response, writer);

            return CallbackTag::Delete;
        },
        processor.get());
    m_service->RequestExchange(&data->ctx, &data->op, m_server->cq(), m_server->cq(), callHandler);

    // Client bidi stream
    uint callCount = 25;
    qt::Event request;

    auto updateRequest = [&] {
        request.setType(qt::Event::Type::CLIENT);
        request.setNumber(request.number() + 1);
        request.setName("client-"_L1 + QString::number(request.number()));
    };

    updateRequest();
    auto copts = QGrpcCallOptions().addMetadata("call-count", QByteArray::number(callCount));
    auto stream = m_client->Exchange(request, copts);
    QVERIFY(stream);

    connect(stream.get(), &QGrpcOperation::finished, this,
            [](const QGrpcStatus &status) { QVERIFY(status.isOk()); });
    connect(stream.get(), &QGrpcBidiStream::messageReceived, this, [&, current = 1u]() mutable {
        const auto response = stream->read<qt::Event>();
        QVERIFY(response.has_value());
        QCOMPARE_EQ(response->type(), qt::Event::Type::SERVER);
        QCOMPARE_EQ(response->number(), current);
        QString name = "server-"_L1 + QString::number(current);
        QCOMPARE_EQ(response->name(), name);
        ++current;
    });

    QTimer delayedWriter;
    connect(&delayedWriter, &QTimer::timeout, this, [&, current = 1u]() mutable {
        if (current >= callCount) {
            stream->writesDone();
            delayedWriter.stop();
        }
        updateRequest();
        stream->writeMessage(request);
        ++current;
    });
    delayedWriter.start(SleepTime);

    QSignalSpy finishedSpy(stream.get(), &QGrpcOperation::finished);
    QVERIFY(finishedSpy.isValid());
    QVERIFY(finishedSpy.wait());
}

void QtGrpcClientEnd2EndTest::clientHandlesCompression_data() const
{
    QTest::addColumn<grpc_compression_algorithm>("compressionAlgo");
    QTest::addRow("compress(None)") << GRPC_COMPRESS_NONE;
    QTest::addRow("compress(Deflate)") << GRPC_COMPRESS_DEFLATE;
    QTest::addRow("compress(Gzip)") << GRPC_COMPRESS_GZIP;
}

void QtGrpcClientEnd2EndTest::clientHandlesCompression()
{
    QFETCH(const grpc_compression_algorithm, compressionAlgo);
    EventList serverResponses;

    class SubscribeListHandler : public AbstractRpcTag
    {
    public:
        SubscribeListHandler(EventList &responses_, EventHub::AsyncService &service_,
                             const grpc_compression_algorithm compressionAlgo_,
                             TagProcessor *processor)
            : AbstractRpcTag(processor), op(&context()), service(service_), responses(responses_),
              compressionAlgo(compressionAlgo_)
        {
            context().set_compression_algorithm(compressionAlgo);
            context().set_compression_level(GRPC_COMPRESS_LEVEL_HIGH);
            // create some 'compressable' data. Try to make it more complex
            // as compression is not guaranteed to actually be applied.
            for (size_t i = 0; i < 100; ++i) {
                const auto v = i % 10;
                Event ev;
                ev.set_name("server;server;" + std::to_string(v));
                ev.set_number(v);
                responses.mutable_events()->Add(std::move(ev));
            }
        }
        void start(grpc::ServerCompletionQueue *cq) override
        {
            service.RequestSubscribeList(&context(), &request, &op, cq, cq, this);
        }
        void process(bool ok) override
        {
            QVERIFY(ok);
            if (index >= responseCount) {
                op.Finish(grpc::Status::OK, new DeleteTag<SubscribeListHandler>(this, processor));
                return;
            }

            grpc::WriteOptions wopts;
            // Enable and disable the compression per-message
            if (index % 2 == 0)
                wopts.set_no_compression();
            op.Write(responses, wopts, this);
            ++index;
        }

        grpc::ServerAsyncWriter<EventList> op;
        EventHub::AsyncService &service;

        None request;
        EventList &responses;

        size_t index = 0;
        const grpc_compression_algorithm compressionAlgo;
        const size_t responseCount = 20;
    };

    auto processor = m_server->createProcessor();
    SubscribeListHandler *handler = new SubscribeListHandler(serverResponses, *m_service,
                                                             compressionAlgo, processor.get());
    m_server->startRpcTag(handler);

    auto call = m_client->SubscribeList(qt::None{});
    QVERIFY(call);

    connect(call.get(), &QGrpcOperation::finished, this,
            [&](const QGrpcStatus &status) { QCOMPARE(status.code(), QtGrpc::StatusCode::Ok); });
    connect(call.get(), &QGrpcServerStream::messageReceived, this, [&] {
        auto response = call->read<qt::EventList>();
        QVERIFY(response);
        QCOMPARE_EQ(response->events().size(), serverResponses.events().size());
        for (int i = 0; i < response->events().size(); ++i) {
            const auto &next = response->events().at(i);
            const auto &baseline = serverResponses.events().at(i);
            QCOMPARE_EQ(next.name(), QString::fromStdString(baseline.name()));
            QCOMPARE_EQ(next.number(), baseline.number());
        }
    });

    QSignalSpy finishedSpy(call.get(), &QGrpcOperation::finished);
    QVERIFY(finishedSpy.isValid());
    QVERIFY(finishedSpy.wait());
}

void QtGrpcClientEnd2EndTest::channelChangeCancelsInFlightRPCs_data() const
{
    QTest::addColumn<int>("triggerCancellationDelay");
    QTest::addRow("Active") << 150;
    QTest::addRow("Idle") << 0;
}

void QtGrpcClientEnd2EndTest::channelChangeCancelsInFlightRPCs()
{
    QFETCH(const int, triggerCancellationDelay);
    auto processor = m_server->createProcessor();
    struct ServerData
    {
        grpc::ServerAsyncResponseWriter<None> op{ &ctx };
        grpc::ServerContext ctx;

        Event request;
        None response;
        bool notifyWhenDone = false;
    };
    auto data = std::make_unique<ServerData>();

    if (triggerCancellationDelay) {
        data->ctx.AsyncNotifyWhenDone(new CallbackTag(
            [&](bool ok) {
                QVERIFY(ok);
                // QVERIFY(data->ctx.IsCancelled()); // TODO: should this be true?
                data->notifyWhenDone = true;
                return CallbackTag::Delete;
            },
            processor.get()));
        CallbackTag *callHandler = new CallbackTag(
            [&](bool ok) {
                QVERIFY(ok);
                return CallbackTag::Delete;
            },
            processor.get());
        m_service->RequestPush(&data->ctx, &data->request, &data->op, m_server->cq(),
                               m_server->cq(), callHandler);
    }

    auto call = m_client->Push(qt::Event{});
    QVERIFY(call);

    connect(call.get(), &QGrpcOperation::finished, this, [&](const QGrpcStatus &status) {
        QCOMPARE_EQ(status.code(), QtGrpc::StatusCode::Cancelled);
    });

    QSignalSpy finishedSpy(call.get(), &QGrpcOperation::finished);
    QSignalSpy channelChangedSpy(m_client.get(), &QGrpcClientBase::channelChanged);
    QVERIFY(finishedSpy.isValid());
    QVERIFY(channelChangedSpy.isValid());

    QTimer::singleShot(triggerCancellationDelay, [&] {
        auto uri = static_cast<QGrpcHttp2Channel *>(m_client->channel().get())->hostUri();
        m_client->attachChannel(std::make_shared<QGrpcHttp2Channel>(uri));
    });

    finishedSpy.wait();
    QCOMPARE_EQ(channelChangedSpy.count(), 1);
    if (triggerCancellationDelay)
        QTRY_COMPARE_EQ_WITH_TIMEOUT(data->notifyWhenDone, true, 5s);
}

QTEST_MAIN(QtGrpcClientEnd2EndTest)

#include "tst_grpc_client_end2end.moc"
