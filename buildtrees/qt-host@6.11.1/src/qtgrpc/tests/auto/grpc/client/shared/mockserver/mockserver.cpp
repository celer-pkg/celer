// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "mockserver.h"
#include "tags.h"

#include <QtTest/qtest.h>

#include <grpcpp/alarm.h>
#include <grpcpp/server_builder.h>

using namespace std::chrono_literals;

TagProcessor::TagProcessor(MockServer *server)
    : mServer(server)
{
    QVERIFY(mServer);
    QVERIFY(mServer->cq());
    mThread = std::thread(&TagProcessor::processLoop, this);
}

TagProcessor::~TagProcessor()
{
    QVERIFY(waitForTagCompletion(5s));

    mRunning.store(false);
    wakeCQ();

    if (mThread.joinable())
        mThread.join();

    drainTags();
}

void TagProcessor::processLoop()
{
    while (mRunning.load(std::memory_order_acquire)) {
        mServer->processTag();
    }
}

void TagProcessor::registerTag(AbstractTag *tag)
{
    std::scoped_lock lock(mMutex);
    const auto it = mActiveTags.insert(tag);
    QVERIFY(it.second);
}

void TagProcessor::unregisterTag(AbstractTag *tag)
{
    std::scoped_lock lock(mMutex);
    const auto erased = mActiveTags.erase(tag);
    QCOMPARE_EQ(erased, 1u);
    if (mActiveTags.empty())
        mCv.notify_all();
}

size_t TagProcessor::activeTagCount() const noexcept
{
    std::scoped_lock lock(mMutex);
    return mActiveTags.size();
}

bool TagProcessor::waitForTagCompletion(std::chrono::milliseconds deadline) noexcept
{
    std::unique_lock lock(mMutex);
    return mCv.wait_for(lock, deadline, [this] { return mActiveTags.empty(); });
}

void TagProcessor::wakeCQ()
{
    auto *alarm = new grpc::Alarm();
    auto *tag = new DeleteTag<grpc::Alarm>(alarm, this);
    alarm->Set(mServer->cq(), gpr_now(GPR_CLOCK_REALTIME), tag);
}

void TagProcessor::drainTags()
{
    QVERIFY(!mRunning);
    while (mServer->processTag(50))
        ;
}

MockServer::MockServer() = default;
MockServer::~MockServer()
{
    stop();
};

bool MockServer::start(std::vector<ListeningPort> ports, std::vector<grpc::Service *> services)
{
    if (!transitionState(State::Stopped, State::Starting))
        return false;

    grpc::ServerBuilder builder;
    for (auto &p : ports)
        builder.AddListeningPort(p.addressUri, p.creds, &p.selectedPort);
    for (auto *s : services)
        builder.RegisterService(s);
    mCQ = builder.AddCompletionQueue();
    mServer = builder.BuildAndStart();
    if (!mServer || !mCQ) {
        mState = State::Stopped;
        return false;
    }
    mState = State::Started;
    return true;
}

bool MockServer::stop()
{
    State currentState = mState.load();
    if (currentState != State::Started && currentState != State::ShuttingDown)
        return currentState == State::Stopped;

    mState = State::ShuttingDown;

    mServer->Shutdown(std::chrono::system_clock::now() + std::chrono::milliseconds(500));
    mCQ->Shutdown();

    void *drainTag;
    bool drainOk;
    while (mCQ->Next(&drainTag, &drainOk)) {
        if (drainTag)
            static_cast<AbstractTag*>(drainTag)->process(drainOk);
    }

    mServer.reset();
    mCQ.reset();
    mState = State::Stopped;

    return true;
}

bool MockServer::processTag(int timeoutMs)
{
    State currentState = mState.load();
    if (currentState != State::Started)
        return false;

    void *rawTag = nullptr;
    bool ok = false;
    const auto status = timeoutMs < 0
        ? mCQ->AsyncNext(&rawTag, &ok, gpr_inf_future(GPR_CLOCK_REALTIME))
        : mCQ->AsyncNext(&rawTag, &ok,
                         std::chrono::system_clock::now() + std::chrono::milliseconds(timeoutMs));

    if (rawTag && status == grpc::CompletionQueue::NextStatus::GOT_EVENT) {
        static_cast<AbstractTag *>(rawTag)->process(ok);
        return true;
    }

    return false;
}

MockServer &MockServer::step(int timeoutMs)
{
    mFutures.emplace_back(std::async(std::launch::async,
                                     [this, timeoutMs] { return processTag(timeoutMs); }));
    return *this;
}

bool MockServer::waitForAllSteps()
{
    for (auto &f : mFutures) {
        if (!f.get())
            return false;
    }
    mFutures.clear();
    return true;
}

void MockServer::startRpcTag(AbstractRpcTag *tag)
{
    QVERIFY(tag);
    tag->start(mCQ.get());
}

std::unique_ptr<TagProcessor> MockServer::createProcessor()
{
    return std::make_unique<TagProcessor>(this);
}

bool MockServer::transitionState(State from, State to)
{
    State expected = from;
    return mState.compare_exchange_strong(expected, to);
}
