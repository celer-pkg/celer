// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <proto/bench_client.grpc.qpb.h>
#include <qrpcbench_common.h>

#include <QtGrpc/qgrpcchanneloptions.h>
#include <QtGrpc/qgrpchttp2channel.h>

#include <QtNetwork/qsslcertificate.h>
#include <QtNetwork/qsslconfiguration.h>

#include <QtCore/qelapsedtimer.h>
#include <QtCore/qcoreapplication.h>
#include <QtCore/qeventloop.h>
#include <QtCore/qobject.h>
#include <QtCore/qtimer.h>

using namespace std::chrono_literals;
using namespace Qt::Literals::StringLiterals;

class QtGrpcClientBenchmark : public QObject
{
    Q_OBJECT
public:
    explicit QtGrpcClientBenchmark(const std::string &transport, quint64 calls,
                                   qsizetype payload = 0)
        : mCalls(calls)
    {
        if (payload > 0)
            sData = Bench::generatePayloads<QByteArray>(payload);

        QUrl uri;
        QGrpcChannelOptions opts;
        const auto address = QString::fromStdString(Bench::getTransportAddress(transport));
        if (transport == "https") {
            uri = QUrl("https://"_L1 + address);
            QSslCertificate crt(QByteArray(Bench::SslRootKey.data(), Bench::SslRootKey.size()));
            QSslConfiguration sslConfig;
            sslConfig.setProtocol(QSsl::TlsV1_2OrLater);
            sslConfig.addCaCertificate(crt);
            sslConfig.setAllowedNextProtocols({ "h2" });
            opts.setSslConfiguration(sslConfig);
        } else if (transport == "http") {
            uri = QUrl("http://"_L1 + address);
        } else {
            uri = QUrl(address);
        }

        mClient.attachChannel(std::make_shared<QGrpcHttp2Channel>(std::move(uri), opts));
    }
    ~QtGrpcClientBenchmark() override = default;

    QtGrpcClientBenchmark(const QtGrpcClientBenchmark &) = delete;
    QtGrpcClientBenchmark &operator=(const QtGrpcClientBenchmark &) = delete;

    QtGrpcClientBenchmark(QtGrpcClientBenchmark &&) = delete;
    QtGrpcClientBenchmark &operator=(QtGrpcClientBenchmark &&) = delete;

    void unaryCall();
    void serverStreaming();
    void clientStreaming();
    void bidiStreaming();

private:
    void unaryCallHelper(qt::bench::UnaryCallRequest &request, Bench::BenchmarkData &benchData);

private:
    qt::bench::BenchmarkService::Client mClient;
    QEventLoop mLoop;
    QElapsedTimer mTimer;
    int64_t mCalls;

    inline static std::vector<QByteArray> sData;
};

void QtGrpcClientBenchmark::unaryCall()
{
    qt::bench::UnaryCallRequest request;
    Bench::BenchmarkData benchData(mCalls);
    benchData.callCount = -50; // warmup
    unaryCallHelper(request, benchData);
    mLoop.exec();
}

// recursively enqueue a message after the previous message finished
void QtGrpcClientBenchmark::unaryCallHelper(qt::bench::UnaryCallRequest &request,
                                            Bench::BenchmarkData &benchData)
{
    request.setTimestamp(Bench::getTimestamp());
    if (!sData.empty() && benchData.callCount >= 0) {
        auto nextPayload = Bench::nextPayload(sData);
        benchData.sendBytes += nextPayload.size();
        request.setPayload(std::move(nextPayload));
    }
    auto reply = mClient.UnaryCall(request);
    auto *replyPtr = reply.get();

    QObject::connect(
        replyPtr, &QGrpcCallReply::finished, &mClient,
        [reply = std::move(reply), this, &request, &benchData](const QGrpcStatus &status) {
            if (benchData.callCount == 0)
                mTimer.start();
            if (status.isOk()) {
                ++benchData.callCount;
                if (benchData.callCount >= 0) {
                    const auto response = reply->read<qt::bench::UnaryCallResponse>();
                    if (response->hasPayload())
                        benchData.receivedBytes += static_cast<quint64>(response->payload().size());
                    benchData.requestLatenciesNanos.push_back(response->requestLatencyNanos());
                    benchData.responseLatenciesNanos
                        .push_back(Bench::calculateLatencyNanosNow(response->timestamp()));
                }
                if (benchData.callCount < mCalls) {
                    unaryCallHelper(request, benchData);
                } else {
                    benchData.elapsedNanos = mTimer.nsecsElapsed();
                    Client::printBenchmarkResult("UnaryCall", benchData);
                    mLoop.quit();
                }
            } else {
                qDebug() << "FAILED: " << status;
                mLoop.quit();
            }
        },
        Qt::SingleShotConnection);
}

void QtGrpcClientBenchmark::serverStreaming()
{
    Bench::BenchmarkData benchData(mCalls);

    qt::bench::ServerStreamingRequest request;
    if (!sData.empty()) {
        auto nextPayload = Bench::nextPayload(sData);
        benchData.sendBytes += nextPayload.size();
        request.setPayload(std::move(nextPayload));
    }
    request.setPing(mCalls);
    auto stream = mClient.ServerStreaming(request);
    auto *streamPtr = stream.get();

    QObject::connect(streamPtr, &QGrpcServerStream::messageReceived, this,
                     [this, stream = streamPtr, &benchData]() {
                         if (benchData.callCount == 0)
                             mTimer.start();
                         const auto response = stream->read<qt::bench::ServerStreamingResponse>();
                         if (response->hasPayload())
                             benchData.receivedBytes += response->payload().size();
                         ++benchData.callCount;
                     });

    QObject::connect(streamPtr, &QGrpcServerStream::finished, this,
                     [this, &benchData](const QGrpcStatus &status) {
                         if (status.isOk()) {
                             benchData.elapsedNanos = mTimer.nsecsElapsed();
                             Client::printBenchmarkResult("ServerStreaming", benchData);
                         } else {
                             qDebug() << "FAILED: " << status;
                         }
                         mLoop.quit();
                     });
    mLoop.exec();
}

void QtGrpcClientBenchmark::clientStreaming()
{
    Bench::BenchmarkData benchData(mCalls);

    qt::bench::ClientStreamingRequest request;
    if (!sData.empty()) {
        auto nextPayload = Bench::nextPayload(sData);
        benchData.sendBytes += nextPayload.size();
        request.setPayload(std::move(nextPayload));
    }
    request.setPing(benchData.callCount++);

    const auto stream = mClient.ClientStreaming(request);

    QTimer::singleShot(0, [this, &stream, &request, &benchData]() {
        // Run on event loop
        mTimer.start();
        for (; benchData.callCount < mCalls; ++benchData.callCount) {
            if (!sData.empty()) {
                auto nextPayload = Bench::nextPayload(sData);
                benchData.sendBytes += nextPayload.size();
                request.setPayload(std::move(nextPayload));
            }
            request.setPing(benchData.callCount);
            stream->writeMessage(request);
        }
        stream->writesDone();
    });

    QObject::connect(stream.get(), &QGrpcServerStream::finished, this,
                     [this, &stream, &benchData](const QGrpcStatus &status) {
                         if (status.isOk()) {
                             const auto resp = stream->read<qt::bench::ClientStreamingResponse>();
                             if (resp->hasPayload())
                                 benchData.receivedBytes = resp->payload().size();
                             benchData.elapsedNanos = mTimer.nsecsElapsed();
                             Client::printBenchmarkResult("ClientStreaming", benchData);
                         } else {
                             qDebug() << "FAILED: " << status;
                         }
                         mLoop.quit();
                     });
    mLoop.exec();
}

void QtGrpcClientBenchmark::bidiStreaming()
{
    Bench::BenchmarkData benchData(mCalls);

    QGrpcCallOptions copts;
    copts.addMetadata("write-queries"_ba, QString::number(mCalls).toUtf8());

    qt::bench::BiDiStreamingRequest request;
    qt::bench::BiDiStreamingResponse response;

    if (!sData.empty()) {
        auto nextPayload = Bench::nextPayload(sData);
        benchData.sendBytes += nextPayload.size();
        copts.addMetadata("write-size", QByteArray::number(nextPayload.size()));
        request.setPayload(std::move(nextPayload));
    }

    auto stream = mClient.BiDiStreaming(request, copts);
    auto *streamPtr = stream.get();

    QTimer::singleShot(0, [this, &stream, &request, &benchData]() {
        // Run on event loop
        mTimer.start();
        for (; benchData.callCount < mCalls; ++benchData.callCount) {
            if (!sData.empty()) {
                auto nextPayload = Bench::nextPayload(sData);
                benchData.sendBytes += nextPayload.size();
                request.setPayload(std::move(nextPayload));
            }
            stream->writeMessage(request);
        }
        stream->writesDone();
    });

    QObject::connect(streamPtr, &QGrpcBidiStream::messageReceived, this,
                     [this, stream = streamPtr, &response, &benchData]() {
                         if (stream->read(&response)) {
                             if (response.hasPayload())
                                 benchData.receivedBytes += response.payload().size();
                         } else {
                             qDebug() << "FAILED: read()";
                             mLoop.quit();
                         }
                     });

    QObject::connect(streamPtr, &QGrpcServerStream::finished, this,
                     [this, &benchData](const QGrpcStatus &status) {
                         if (status.isOk()) {
                             benchData.elapsedNanos = mTimer.nsecsElapsed();
                             benchData.callCount *= 2;
                             Client::printBenchmarkResult("BidiStreaming", benchData);
                         } else {
                             qDebug() << "FAILED: " << status;
                         }
                         mLoop.quit();
                     });
    mLoop.exec();
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    Client::benchmarkMain<QtGrpcClientBenchmark>("QtGrpcClient", argc, argv);
}

#include "bench_qtgrpcclient.moc"
