// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <proto/bench.grpc.pb.h>
#include <proto/bench.pb.h>
#include <qrpcbench_common.h>

#include <grpcpp/grpcpp.h>
#include <grpcpp/security/credentials.h>

#include <absl/log/initialize.h>

#include <QtCore/qelapsedtimer.h>

class AsyncGrpcClientBenchmark
{
public:
    explicit AsyncGrpcClientBenchmark(const std::string &transport, uint64_t calls,
                                      size_t payload = 0)
        : mExpectedCalls(int64_t(calls))
    {
        assert(mExpectedCalls > 0);
        if (payload > 0)
            sData = Bench::generatePayloads<std::string>(payload);

        std::shared_ptr<grpc::ChannelCredentials> creds;
        grpc::ChannelArguments args;
        if (transport == "https") {
            grpc::SslCredentialsOptions sslOpts;
            sslOpts.pem_root_certs = { Bench::SslRootKey.data(), Bench::SslRootKey.size() };
            creds = grpc::SslCredentials(sslOpts);
        } else {
            creds = grpc::InsecureChannelCredentials();
        }
        auto channel = grpc::CreateCustomChannel(Bench::getTransportAddress(transport), creds, args);
        mStub = qt::bench::BenchmarkService::NewStub(std::move(channel));
    }

    void unaryCall();
    void serverStreaming();
    void clientStreaming();
    void bidiStreaming();

private:
    std::unique_ptr<qt::bench::BenchmarkService::Stub> mStub;
    QElapsedTimer mTimer;
    int64_t mExpectedCalls;

    inline static std::vector<std::string> sData;
};

void AsyncGrpcClientBenchmark::unaryCall()
{
    struct UnaryCallData
    {
        grpc::ClientContext context;
        grpc::Status status;
        std::unique_ptr<grpc::ClientAsyncResponseReader<qt::bench::UnaryCallResponse>> reader;

        qt::bench::UnaryCallRequest request;
        qt::bench::UnaryCallResponse response;
    };

    Bench::BenchmarkData benchData(static_cast<uint64_t>(mExpectedCalls));
    benchData.callCount = -50; // for warmup

    grpc::CompletionQueue cq;
    void *rawTag = nullptr;
    bool ok = false;

    const auto startCall = [this, &cq, &benchData]() {
        auto *call = new UnaryCallData();
        *call->request.mutable_timestamp() = Bench::getTimestamp();
        if (!sData.empty() && benchData.callCount >= 0) {
            auto nextPayload = Bench::nextPayload(sData);
            benchData.sendBytes += nextPayload.size();
            call->request.set_payload(std::move(nextPayload));
        }
        call->reader = mStub->AsyncUnaryCall(&call->context, call->request, &cq);
        call->reader->Finish(&call->response, &call->status, call);
    };

    startCall();
    mTimer.start();

    while (cq.Next(&rawTag, &ok)) {
        auto *rpcResult = static_cast<UnaryCallData *>(rawTag);
        if (rpcResult->status.ok()) {
            if (rpcResult->response.has_payload())
                benchData.receivedBytes += rpcResult->response.payload().size();
            if (++benchData.callCount < mExpectedCalls) {
                if (benchData.callCount >= 0) {
                    benchData.requestLatenciesNanos
                        .push_back(rpcResult->response.request_latency_nanos());
                    benchData.responseLatenciesNanos
                        .push_back(Bench::calculateLatencyNanosNow(rpcResult->response.timestamp()));
                }
                delete rpcResult;
                startCall();
                continue;
            } else {
                delete rpcResult;
                break;
            }
        } else {
            std::cout << "FAILED: " << rpcResult->status.error_message() << '\n';
            delete rpcResult;
        }
    }

    benchData.elapsedNanos = mTimer.nsecsElapsed();
    Client::printBenchmarkResult("UnaryCall", benchData);

    cq.Shutdown();
    while (cq.Next(&rawTag, &ok))
        ;
}

void AsyncGrpcClientBenchmark::serverStreaming()
{
    struct ServerStreamingData
    {
        grpc::ClientContext context;
        grpc::Status status;
        std::unique_ptr<grpc::ClientAsyncReader<qt::bench::ServerStreamingResponse>> stream;

        qt::bench::ServerStreamingRequest request;
        qt::bench::ServerStreamingResponse response;

        std::function<bool(bool)> readHandler;
        std::function<bool(bool)> finishHandler;
        std::function<bool(bool)> callHandler;
    };

    Bench::BenchmarkData benchData(static_cast<uint64_t>(mExpectedCalls));

    grpc::CompletionQueue cq;
    void *rawTag = nullptr;
    bool ok = false;

    auto *call = new ServerStreamingData();
    call->readHandler = [call, &benchData](bool ok) {
        if (ok) {
            call->stream->Read(&call->response, &call->readHandler);
            if (call->response.has_payload())
                benchData.receivedBytes += call->response.payload().size();
            ++benchData.callCount;
        }
        return true;
    };
    call->finishHandler = [this, call, &benchData](bool ok) {
        if (ok && call->status.ok()) {
            benchData.elapsedNanos = mTimer.nsecsElapsed();
            Client::printBenchmarkResult("ServerStreaming", benchData);
        } else {
            std::cout << "FAILED: " << call->status.error_message();
        }
        return false;
    };
    call->callHandler = [this, call](bool ok) {
        if (ok) {
            call->stream->Read(&call->response, &call->readHandler);
            call->stream->Finish(&call->status, &call->finishHandler);
            mTimer.restart();
            return true;
        } else {
            std::cout << "FAILED: serverStreamingRequest\n";
            return false;
        }
    };

    if (!sData.empty()) {
        auto nextPayload = Bench::nextPayload(sData);
        benchData.sendBytes += nextPayload.size();
        call->request.set_payload(std::move(nextPayload));
    }
    call->request.set_ping(mExpectedCalls);
    call->stream = mStub->AsyncServerStreaming(&call->context, call->request, &cq,
                                               &call->callHandler);

    while (cq.Next(&rawTag, &ok)) {
        auto rpcTag = *static_cast<std::function<bool(bool)> *>(rawTag);
        if (!rpcTag(ok))
            break;
    }

    cq.Shutdown();
    while (cq.Next(&rawTag, &ok))
        ;

    delete call;
}

void AsyncGrpcClientBenchmark::clientStreaming()
{
    struct ClientStreamingData
    {
        grpc::ClientContext context;
        grpc::Status status;
        std::unique_ptr<grpc::ClientAsyncWriter<qt::bench::ClientStreamingRequest>> stream;

        qt::bench::ClientStreamingRequest request;
        qt::bench::ClientStreamingResponse response;

        std::function<bool(bool)> writeHandler;
        std::function<bool(bool)> writesDoneHandler;
        std::function<bool(bool)> finishHandler;
        std::function<bool(bool)> callHandler;
    };

    Bench::BenchmarkData benchData(static_cast<uint64_t>(mExpectedCalls));

    grpc::CompletionQueue cq;
    void *rawTag = nullptr;
    bool ok = false;

    auto *call = new ClientStreamingData();
    call->writeHandler = [this, call, &benchData](bool ok) {
        if (ok && benchData.callCount < mExpectedCalls) {
            if (!sData.empty()) {
                auto nextPayload = Bench::nextPayload(sData);
                benchData.sendBytes += nextPayload.size();
                call->request.set_payload(std::move(nextPayload));
            }
            call->request.set_ping(benchData.callCount);
            call->stream->Write(call->request, &call->writeHandler);
            ++benchData.callCount;
        } else if (ok && benchData.callCount >= mExpectedCalls) {
            call->stream->WritesDone(&call->writesDoneHandler);
        }
        return true;
    };
    call->writesDoneHandler = [](bool) { return true; };
    call->finishHandler = [this, call, &benchData](bool ok) {
        if (ok && call->status.ok()) {
            if (call->response.has_payload())
                benchData.receivedBytes += call->response.payload().size();
            benchData.elapsedNanos = mTimer.nsecsElapsed();
            Client::printBenchmarkResult("ClientStreaming", benchData);
        } else {
            std::cout << "FAILED: " << call->status.error_message();
        }
        return false;
    };
    call->callHandler = [this, call](bool ok) {
        if (ok) {
            mTimer.start();
            call->writeHandler(true); // Start first write
            call->stream->Finish(&call->status, &call->finishHandler);
            return true;
        } else {
            std::cout << "FAILED: clientStreaming\n";
            return false;
        }
    };

    call->stream = mStub->AsyncClientStreaming(&call->context, &call->response, &cq,
                                               &call->callHandler);

    while (cq.Next(&rawTag, &ok)) {
        auto rpcTag = *static_cast<std::function<bool(bool)> *>(rawTag);
        if (!rpcTag(ok))
            break;
    }

    cq.Shutdown();
    while (cq.Next(&rawTag, &ok))
        ;

    delete call;
}

void AsyncGrpcClientBenchmark::bidiStreaming()
{
    struct BidiStreamingData
    {
        grpc::ClientContext context;
        grpc::Status status;
        std::unique_ptr<grpc::ClientAsyncReaderWriter<qt::bench::BiDiStreamingRequest,
                                                      qt::bench::BiDiStreamingResponse>>
            stream;

        qt::bench::BiDiStreamingRequest request;
        qt::bench::BiDiStreamingResponse response;

        std::function<bool(bool)> finishHandler;
        std::function<bool(bool)> callHandler;
        std::function<bool(bool)> writeHandler;
        std::function<bool(bool)> writesDoneHandler;
        std::function<bool(bool)> readHandler;
    };
    Bench::BenchmarkData benchData(static_cast<uint64_t>(mExpectedCalls));

    grpc::CompletionQueue cq;
    void *rawTag = nullptr;
    bool ok = false;

    auto *call = new BidiStreamingData();
    call->writeHandler = [this, call, &benchData](bool ok) {
        if (ok && benchData.callCount < mExpectedCalls) {
            if (!sData.empty()) {
                auto nextPayload = Bench::nextPayload(sData);
                benchData.sendBytes += nextPayload.size();
                call->request.set_payload(std::move(nextPayload));
            }
            call->stream->Write(call->request, &call->writeHandler);
            ++benchData.callCount;
        } else if (ok && benchData.callCount >= mExpectedCalls) {
            call->stream->WritesDone(&call->writesDoneHandler);
        }
        return true;
    };
    call->writesDoneHandler = [](bool) { return true; };
    call->readHandler = [call, &benchData](bool ok) {
        if (ok) {
            if (call->response.has_payload())
                benchData.receivedBytes += call->response.payload().size();
            call->response.Clear();
            call->stream->Read(&call->response, &call->readHandler);
        }
        return true;
    };
    call->finishHandler = [this, call, &benchData](bool ok) {
        if (ok && call->status.ok()) {
            benchData.elapsedNanos = mTimer.nsecsElapsed();
            benchData.callCount *= 2;
            Client::printBenchmarkResult("BidiStreaming", benchData);
        } else {
            std::cout << "FAILED: " << call->status.error_message();
        }
        return false;
    };
    call->callHandler = [call](bool ok) {
        if (ok) {
            call->stream->Finish(&call->status, &call->finishHandler);
            call->stream->Read(&call->response, &call->readHandler);
            call->writeHandler(true);
            return true;
        } else {
            std::cout << "FAILED: clientStreaming\n";
            return false;
        }
    };

    if (!sData.empty())
        call->context.AddMetadata("write-size", std::to_string(sData[0].size()));
    call->context.AddMetadata("write-queries", std::to_string(mExpectedCalls));
    mTimer.restart();
    call->stream = mStub->AsyncBiDiStreaming(&call->context, &cq, &call->callHandler);

    while (cq.Next(&rawTag, &ok)) {
        auto rpcTag = *static_cast<std::function<bool(bool)> *>(rawTag);
        if (!rpcTag(ok))
            break;
    }

    cq.Shutdown();
    while (cq.Next(&rawTag, &ok))
        ;

    delete call;
}

int main(int argc, char *argv[])
{
    absl::InitializeLog();
    Client::benchmarkMain<AsyncGrpcClientBenchmark>("AsyncReferenceClient", argc, argv);
}
