// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef RPCJOBS_H
#define RPCJOBS_H

#include "qrpcbench_common.h"

#include <grpc/support/time.h>
#include <proto/bench.grpc.pb.h>
#include <proto/bench.pb.h>

#include <grpcpp/server_context.h>

using qt::bench::BenchmarkService;

class RpcJob
{
public:
    RpcJob() = default;
    virtual ~RpcJob() = default;

    RpcJob(const RpcJob &) = delete;
    RpcJob &operator=(const RpcJob &) = delete;

    RpcJob(RpcJob &&) = delete;
    RpcJob &operator=(RpcJob &&) = delete;

    virtual void onProcess(bool ok) = 0;
    virtual void onDone() = 0;

    [[nodiscard]] bool isDone() const noexcept { return mOnDoneCalled; }

private:
    std::atomic<bool> mOnDoneCalled = false;
    friend class NotifyWhenDone;
};

class NotifyWhenDone final : public RpcJob
{
public:
    explicit NotifyWhenDone(RpcJob *parent) : mParent(parent) { }

    void onProcess(bool /* ok */) override
    {
        assert(mParent);
        mParent->mOnDoneCalled = true;
        onDone();
    }

    void onDone() override { delete this; }

private:
    RpcJob *mParent;
};

class UnaryCall final : public RpcJob
{
    using Request = qt::bench::UnaryCallRequest;
    using Response = qt::bench::UnaryCallResponse;

public:
    explicit UnaryCall(grpc::ServerCompletionQueue *cq, BenchmarkService::AsyncService *service)
        : mCQ(cq), mService(service), mWriter(&mContext)
    {
        mService->RequestUnaryCall(&mContext, &mRequest, &mWriter, cq, cq, this);
        mContext.AsyncNotifyWhenDone(new NotifyWhenDone(this));
    }

    void onProcess(bool ok) override
    {
        if (!ok || isDone())
            return onDone();

        if (mState == State::Created) {
            new UnaryCall(mCQ, mService);
            *mResponse.mutable_timestamp() = Bench::getTimestamp();
            mResponse
                .set_request_latency_nanos(Bench::calculateLatencyNanos(mRequest.timestamp(),
                                                                        mResponse.timestamp()));
            if (mRequest.has_payload())
                mResponse.mutable_payload()->swap(*mRequest.mutable_payload());
            mWriter.Finish(mResponse, grpc::Status::OK, this);
            mState = State::Processed;
        } else {
            return onDone();
        }
    }

    void onDone() override { delete this; }

private:
    grpc::ServerContext mContext;
    grpc::ServerCompletionQueue *mCQ;
    BenchmarkService::AsyncService *mService;

    Request mRequest;
    Response mResponse;
    grpc::ServerAsyncResponseWriter<Response> mWriter;

    enum class State { Created, Processed };
    State mState = State::Created;
};

class ServerStreaming final : public RpcJob
{
    using Request = qt::bench::ServerStreamingRequest;
    using Response = qt::bench::ServerStreamingResponse;

public:
    explicit ServerStreaming(grpc::ServerCompletionQueue *cq,
                             BenchmarkService::AsyncService *service)
        : mCQ(cq), mService(service), mServerStream(&mContext)
    {
        mService->RequestServerStreaming(&mContext, &mRequest, &mServerStream, mCQ, mCQ, this);
        mContext.AsyncNotifyWhenDone(new NotifyWhenDone(this));
    }

    void onProcess(bool ok) override
    {
        if (!ok || isDone())
            return onDone();

        switch (mState) {
        case State::Created: {
            new ServerStreaming(mCQ, mService);

            if (mRequest.has_payload()) {
                mResponseData = Bench::generatePayloads<std::string>(mRequest.payload().size(), 2);
                mResponse.set_payload(Bench::nextPayload(mResponseData));
            }
            mResponse.set_pong(mPongCount);

            mServerStream.Write(mResponse, this);
            mState = State::Writing;
        } break;
        case State::Writing: {
            if (++mPongCount >= mRequest.ping()) {
                mServerStream.Finish(grpc::Status::OK, this);
                mState = State::Finished;
            } else {
                mResponse.set_payload(Bench::nextPayload(mResponseData));
                mResponse.set_pong(mPongCount);
                mServerStream.Write(mResponse, this);
            }
        } break;
        case State::Finished: {
            // Finish will invoke onDone directly.
        } break;
        }
    }

    void onDone() override
    {
        std::cout << std::format("serverstream done, {} pongs\n", mPongCount);
        delete this;
    }

private:
    grpc::ServerContext mContext;
    grpc::ServerCompletionQueue *mCQ;
    BenchmarkService::AsyncService *mService;
    grpc::ServerAsyncWriter<Response> mServerStream;

    enum class State { Created, Writing, Finished };
    State mState = State::Created;

    Request mRequest;
    Response mResponse;
    uint64_t mPongCount = 0;
    std::vector<std::string> mResponseData;
};

class ClientStreaming final : public RpcJob
{
    using Request = qt::bench::ClientStreamingRequest;
    using Response = qt::bench::ClientStreamingResponse;

public:
    explicit ClientStreaming(grpc::ServerCompletionQueue *cq,
                             BenchmarkService::AsyncService *service)
        : mCQ(cq), mService(service), mClientStream(&mContext)
    {
        mService->RequestClientStreaming(&mContext, &mClientStream, mCQ, mCQ, this);
        mContext.AsyncNotifyWhenDone(new NotifyWhenDone(this));
    }

    void onProcess(bool ok) override
    {
        if (isDone())
            return onDone();

        switch (mState) {
        case State::Created: {
            new ClientStreaming(mCQ, mService);
            if (!ok)
                return onDone();
            mClientStream.Read(&mRequest, this);
            mState = State::Reading;
        } break;
        case State::Reading: {
            if (!ok) {
                if (mRequest.has_payload())
                    mResponse.mutable_payload()->swap(*mRequest.mutable_payload());
                mResponse.set_pong(mPingCount);
                mClientStream.Finish(mResponse, grpc::Status::OK, this);
                mState = State::Finished;
            } else {
                ++mPingCount;
                mClientStream.Read(&mRequest, this);
            }
        } break;
        case State::Finished: {
            // Finish will invoke onDone directly.
        } break;
        }
    }

    void onDone() override
    {
        std::cout << std::format("clientstream done, {} pings\n", mPingCount);
        delete this;
    }

private:
    grpc::ServerContext mContext;
    grpc::ServerCompletionQueue *mCQ;
    BenchmarkService::AsyncService *mService;
    grpc::ServerAsyncReader<Response, Request> mClientStream;

    enum class State { Created, Reading, Finished };
    State mState = State::Created;

    Request mRequest;
    Response mResponse;
    uint64_t mPingCount = 0;
};

class BiDiStreaming final : public RpcJob
{
    using Request = qt::bench::BiDiStreamingRequest;
    using Response = qt::bench::BiDiStreamingResponse;
public:
    explicit BiDiStreaming(grpc::ServerCompletionQueue *cq, BenchmarkService::AsyncService *service)
        : mCQ(cq), mService(service), mStream(&mContext)
    {
        mService->RequestBiDiStreaming(&mContext, &mStream, mCQ, mCQ, this);
        mContext.AsyncNotifyWhenDone(new NotifyWhenDone(this));
    }

    void onProcess(bool ok) override
    {
        if (isDone())
            return onDone();

        switch (mState) {
        case State::Created: {
            new BiDiStreaming(mCQ, mService);
            if (!ok)
                return onDone();
            auto md = mContext.client_metadata();
            if (const auto it = md.find("write-queries"); it == md.cend()) {
                std::cout << "requested 'write-queries' not found in metadata\n";
                return onDone();
            } else {
                mWriteQueries = std::stoul({ it->second.data() });
            }
            if (const auto it = md.find("write-size"); it != md.cend()) {
                const auto payloadSize = std::stoul({ it->second.data() });
                mResponseData = Bench::generatePayloads<std::string>(payloadSize, 2);
            }
            mState = State::Processing;
            mReadOperation = new ReadOperation(this);
            mWriteOperation = new WriteOperation(this);
        } break;
        case State::Processing: {
        } break;
        case State::Finished: {
            // Finish will invoke onDone directly.
        } break;
        }
    }

    void onDone() override
    {
        std::cout << "bidistream done\n";
        delete mReadOperation;
        delete mWriteOperation;
        delete this;
    }

private:
    grpc::ServerContext mContext;
    grpc::ServerCompletionQueue *mCQ;
    BenchmarkService::AsyncService *mService;

    grpc::ServerAsyncReaderWriter<Response, Request> mStream;

    enum class State { Created, Processing, Finished };
    State mState = State::Created;

    class ReadOperation final : public RpcJob
    {
    public:
        explicit ReadOperation(BiDiStreaming *parent) : mParent(parent)
        {
            mParent->mStream.Read(&mRequest, this);
        }

        ~ReadOperation() override = default;

        void onProcess(bool ok) override
        {
            if (mParent->mState != State::Processing)
                return;

            if (!ok) {
                mParent->mReadDone = true;
                if (mParent->mWriteDone) { // both ends finished
                    mParent->mState = State::Finished;
                    mParent->mStream.Finish(grpc::Status::OK, mParent);
                }
                return;
            }

            mParent->mStream.Read(&mRequest, this);
        }

        void onDone() override { delete this; }

    private:
        BiDiStreaming *mParent;
        Request mRequest;
    };

    class WriteOperation final : public RpcJob
    {
    public:
        explicit WriteOperation(BiDiStreaming *parent) : mParent(parent)
        {
            if (mParent->mResponseData.size() > 0)
                mResponse.set_payload(Bench::nextPayload(mParent->mResponseData));
            mParent->mStream.Write(mResponse, this);
            ++mWritten;
        }

        ~WriteOperation() override = default;

        void onProcess(bool ok) override
        {
            if (mParent->mState == State::Finished)
                return;

            if (mWritten >= mParent->mWriteQueries) {
                mParent->mWriteDone = true;
                if (mParent->mReadDone || !ok) { // both ends finished
                    mParent->mState = State::Finished;
                    mParent->mStream.Finish(grpc::Status::OK, mParent);
                }
                return;
            }

            mResponse.set_payload(Bench::nextPayload(mParent->mResponseData));
            mParent->mStream.Write(mResponse, this);
            ++mWritten;
        }

        void onDone() override { delete this; }

    private:
        BiDiStreaming *mParent;
        Response mResponse;
        size_t mWritten = 0;
    };

    friend class ReadOperation;
    friend class WriteOperation;
    ReadOperation *mReadOperation{};
    WriteOperation *mWriteOperation{};
    size_t mWriteQueries = 0;
    std::atomic<bool> mReadDone = false;
    std::atomic<bool> mWriteDone = false;

    std::vector<std::string> mResponseData;
};

#endif // RPCJOBS_H
