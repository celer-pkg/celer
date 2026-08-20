// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#pragma once

#include "certificates.h"

#include <grpcpp/grpcpp.h>
#include <grpcpp/security/server_credentials.h>

#include <future>
#include <memory>
#include <string>
#include <thread>
#include <unordered_set>
#include <vector>

struct ListeningPort
{
    std::string addressUri;
    std::shared_ptr<grpc::ServerCredentials> creds;
    int selectedPort = -1;
};

class MockServer;
class AbstractTag;
class AbstractRpcTag;

class TagProcessor
{
public:
    explicit TagProcessor(MockServer *server);
    ~TagProcessor();

    TagProcessor(const TagProcessor &) = delete;
    TagProcessor &operator=(const TagProcessor &) = delete;

    void registerTag(AbstractTag *tag);
    void unregisterTag(AbstractTag *tag);

    [[nodiscard]] size_t activeTagCount() const noexcept;
    [[nodiscard]] bool waitForTagCompletion(std::chrono::milliseconds deadline) noexcept;

private:
    void processLoop();
    void wakeCQ();
    void drainTags();

    MockServer *mServer;
    std::atomic_bool mRunning = true;
    std::thread mThread;

    mutable std::mutex mMutex;
    std::condition_variable mCv;
    std::unordered_set<AbstractTag *> mActiveTags;
};

class MockServer
{
public:
    enum class State {
        Stopped,
        Starting,
        Started,
        ShuttingDown,
    };

    MockServer();
    ~MockServer();

    grpc::ServerCompletionQueue *cq() { return mCQ.get(); }
    State state() const { return mState.load(std::memory_order_acquire); }

    bool start(std::vector<ListeningPort> ports, std::vector<grpc::Service *> services);
    bool stop();

    bool processTag(int timeoutMs = -1);

    void startRpcTag(AbstractRpcTag *tag);

    MockServer &step(int timeoutMs = -1);
    bool waitForAllSteps();

    std::unique_ptr<TagProcessor> createProcessor();

private:
    bool transitionState(State from, State to);

private:
    std::unique_ptr<grpc::Server> mServer;
    std::unique_ptr<grpc::ServerCompletionQueue> mCQ;
    std::vector<std::future<bool>> mFutures;
    std::atomic<State> mState = State::Stopped;
};

#include "tags.h"
