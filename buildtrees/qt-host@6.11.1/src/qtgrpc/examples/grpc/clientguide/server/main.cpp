// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "clientguide.grpc.pb.h"

#include <grpcpp/security/server_credentials.h>
#include <grpcpp/server.h>
#include <grpcpp/server_builder.h>

#include <charconv>
#include <chrono>
#include <iostream>
#include <mutex>
#include <sstream>
#include <string_view>

using client::guide::Request;
using client::guide::Response;

static constexpr std::string_view ServerUri = "localhost:50056";

namespace {

int64_t now()
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()
                                                                     .time_since_epoch())
        .count();
}

std::ostream &operator<<(std::ostream &stream, const Request &request)
{
    return stream << "Request( time: " << request.time() << ", num: " << request.num() << " )";
}

// Validate Bearer token format and freshness
grpc::Status validateAuth(grpc::ServerContext *context)
{
    const auto fail = [&](grpc::StatusCode code, const std::string &msg) {
        context->AddTrailingMetadata("www-authenticate",
                                     R"(Bearer realm="clientguide", error="invalid_token")");
        return grpc::Status{ code, msg };
    };

    const auto &md = context->client_metadata();
    const auto it = md.find("authorization");
    if (it == md.end())
        return grpc::Status::OK; // no auth required in this demo

    std::string_view auth{ it->second.data(), it->second.size() };
    if (auth.size() < 7 || auth.substr(0, 7) != "Bearer ")
        return { grpc::StatusCode::UNAUTHENTICATED, "Invalid authorization format" };

    auto token = auth.substr(7);

    // Expected: header.payload.signature
    const auto dot1 = token.find('.');
    const auto dot2 = token.find('.', dot1 == token.npos ? dot1 : dot1 + 1);
    if (dot1 == token.npos || dot2 == token.npos)
        return fail(grpc::StatusCode::UNAUTHENTICATED, "Invalid token structure");

    // payload = timestamp
    std::string_view payload = token.substr(dot1 + 1, dot2 - dot1 - 1);

    int64_t ts{ };
    const auto [_, ec] = std::from_chars(payload.data(), payload.data() + payload.size(), ts);
    if (ec != std::errc{ })
        return fail(grpc::StatusCode::UNAUTHENTICATED, "Invalid token payload");

    const auto age = now() - ts;
    if (age > 500) {
        return fail(grpc::StatusCode::UNAUTHENTICATED,
                    "Token expired with age: " + std::to_string(age) + " ms");
    }

    return grpc::Status::OK;
}

std::string getPrintableTraceId(const std::multimap<grpc::string_ref, grpc::string_ref> &md)
{
    const auto traceIt = md.find("x-trace-id");
    if (traceIt == md.cend())
        return { };
    return std::string(", Metadata: { ")
        .append(traceIt->first.data(), traceIt->first.size())
        .append(": ")
        .append(traceIt->second.data(), traceIt->second.size())
        .append(" }");
}

class LogLine
{
public:
    ~LogLine()
    {
        std::lock_guard lock(mutex());
        std::cout << m_buf.str() << std::endl;
    }

    template <typename T>
    LogLine &operator<<(T &&value)
    {
        m_buf << std::forward<T>(value);
        return *this;
    }

private:
    static std::mutex &mutex()
    {
        static std::mutex m;
        return m;
    }
    std::ostringstream m_buf;
};

LogLine log() { return { }; }

} // namespace

class ClientGuideService : public client::guide::ClientGuideService::Service
{
    grpc::Status UnaryCall(grpc::ServerContext *context, const Request *request,
                           Response *response) override
    {
        //! [auth]
        if (auto authStatus = validateAuth(context); !authStatus.ok())
            return authStatus;
        //! [auth]

        log() << "Server (UnaryCall): " << *request
              << getPrintableTraceId(context->client_metadata());

        //! [time]
        const auto time = now();
        if (request->time() > time)
            return { grpc::StatusCode::INVALID_ARGUMENT, "Request time is in the future!" };
        //! [time]

        //! [response]
        response->set_num(request->num());
        response->set_time(time);
        return grpc::Status::OK;
        //! [response]
    }

    grpc::Status ServerStreaming(grpc::ServerContext *context, const Request *request,
                                 grpc::ServerWriter<Response> *writer) override
    {
        if (auto authStatus = validateAuth(context); !authStatus.ok())
            return authStatus;

        log() << "Server (ServerStreaming): " << *request
              << getPrintableTraceId(context->client_metadata());

        if (request->time() > now())
            return { grpc::StatusCode::INVALID_ARGUMENT, "Request time is in the future!" };

        Response response;
        for (int32_t i = 0; i < request->num(); ++i) {
            response.set_num(i);
            response.set_time(now());
            writer->Write(response);
        }

        return grpc::Status::OK;
    }

    grpc::Status ClientStreaming(grpc::ServerContext *context, grpc::ServerReader<Request> *reader,
                                 Response *response) override
    {
        if (auto authStatus = validateAuth(context); !authStatus.ok())
            return authStatus;

        log() << "Server (ClientStreaming) accepted call"
              << getPrintableTraceId(context->client_metadata());

        Request request;
        int32_t count = 0;
        while (reader->Read(&request)) {
            log() << "Server (ClientStreaming): " << request;
            if (request.time() > now())
                return { grpc::StatusCode::INVALID_ARGUMENT, "Request time is in the future!" };
            ++count;
        }
        response->set_num(count);
        response->set_time(now());
        return grpc::Status::OK;
    }

    grpc::Status
    BidirectionalStreaming(grpc::ServerContext *context,
                           grpc::ServerReaderWriter<Response, Request> *stream) override
    {
        if (auto authStatus = validateAuth(context); !authStatus.ok())
            return authStatus;

        log() << "Server (BidirectionalStreaming) accepted call"
              << getPrintableTraceId(context->client_metadata());

        Request request;
        Response response;

        while (stream->Read(&request)) {
            log() << "Server (BidirectionalStreaming): " << request;
            const auto time = now();
            if (request.time() > time)
                return { grpc::StatusCode::INVALID_ARGUMENT, "Request time is in the future!" };
            response.set_num(request.num());
            response.set_time(time);
            if (!stream->Write(response))
                return grpc::Status::CANCELLED;
        }

        return grpc::Status::OK;
    }
};

int main(int /* argc */, char * /* argv */[])
{
    std::unique_ptr<grpc::Server> server;
    ClientGuideService service;
    {
        grpc::ServerBuilder builder;
        builder.AddListeningPort(ServerUri.data(), grpc::InsecureServerCredentials());
        builder.RegisterService(&service);
        server = builder.BuildAndStart();
    }
    std::cout << "Server listening on: " << ServerUri.data() << std::endl;
    server->Wait();
}
