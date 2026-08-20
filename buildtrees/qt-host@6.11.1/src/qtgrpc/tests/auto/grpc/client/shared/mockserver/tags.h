// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#pragma once

#include "mockserver.h"

#include <grpcpp/completion_queue.h>
#include <grpcpp/server_context.h>

#include <atomic>
#include <functional>

class AbstractTag
{
public:
    explicit AbstractTag(TagProcessor *processor_) : processor(processor_)
    {
        processor->registerTag(this);
    }
    virtual ~AbstractTag() { processor->unregisterTag(this); }

    AbstractTag(const AbstractTag &) = delete;
    AbstractTag &operator=(const AbstractTag &) = delete;

    AbstractTag(AbstractTag &&) = delete;
    AbstractTag &operator=(AbstractTag &&) = delete;

    virtual void process(bool ok) = 0;

protected:
    TagProcessor *processor;
};

class CallbackTag : public AbstractTag
{
public:
    enum Operation { Proceed, Delete };
    using Function = std::function<Operation(bool)>;

    explicit CallbackTag(Function fn, TagProcessor *processor_)
        : AbstractTag(processor_), mFn(std::move(fn))
    {
    }
    void process(bool ok) override
    {
        if (mFn(ok) == Delete)
            delete this;
    }

private:
    Function mFn;
};

class VoidTag final : public CallbackTag
{
public:
    VoidTag(TagProcessor *processor_) : CallbackTag([](bool) { return Delete; }, processor_) { }
};

// TODO: gRPC 1.50.1 on windows has faulty lifetime management and still uses
// the context post completion in the interceptors. This should be fixed in
// newer versions. Remove this when upgrading to favor stack based lifetime
// management in testcases.
template <typename Data>
class DeleteTag final : public CallbackTag
{
public:
    explicit DeleteTag(Data *data, TagProcessor *processor_)
        : CallbackTag([](bool) { return Delete; }, processor_), data(data)
    {
        assert(this->data);
    }
    ~DeleteTag()
    {
        delete data;
        data = nullptr;
    }

private:
    Data *data;
};

class AbstractRpcTag : public AbstractTag
{
public:
    AbstractRpcTag(TagProcessor *processor_) : AbstractTag(processor_)
    {
        mContext.AsyncNotifyWhenDone(new CallbackTag(
            [this](bool ok) {
                if (ok && mContext.IsCancelled())
                    mIsCancelled = true;
                return CallbackTag::Delete;
            },
            processor));
    }

    virtual void start(grpc::ServerCompletionQueue *cq) = 0;

    const grpc::ServerContext &context() const { return mContext; }
    grpc::ServerContext &context() { return mContext; }
    [[nodiscard]] bool isCancelled() const noexcept { return mIsCancelled.load(); }

private:
    std::atomic<bool> mIsCancelled = false;
    grpc::ServerContext mContext;
};
