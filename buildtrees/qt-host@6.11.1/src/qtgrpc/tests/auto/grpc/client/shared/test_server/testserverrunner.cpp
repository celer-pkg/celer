// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "testserverrunner.h"

#include <QDebug>
#include <QFile>
#include <QString>
#include <QThread>

#include "testservice.grpc.pb.h"

#include <grpc++/grpc++.h>

#include <memory>
#include <charconv>

namespace {

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ServerWriter;
using grpc::Status;
using qtgrpc::tests::Empty;
using qtgrpc::tests::BlobMessage;
using qtgrpc::tests::SimpleIntMessage;
using qtgrpc::tests::SimpleStringMessage;
using qtgrpc::tests::TestService;

// Logic and data behind the server's behavior.
class TestServiceServiceImpl final : public qtgrpc::tests::TestService::Service
{
public:
    TestServiceServiceImpl(qint64 latency) : m_latency(latency) { }

private:
    grpc::Status testMethod(grpc::ServerContext *, const SimpleStringMessage *request,
                            SimpleStringMessage *response) override;

    grpc::Status testMethodServerStream(grpc::ServerContext *, const SimpleStringMessage *request,
                                        grpc::ServerWriter<SimpleStringMessage> *writer) override;

    grpc::Status testMethodBlobServerStream(grpc::ServerContext *, const BlobMessage *request,
                                            grpc::ServerWriter<BlobMessage> *writer) override;
    grpc::Status testMethodStatusMessage(grpc::ServerContext *, const SimpleStringMessage *request,
                                         SimpleStringMessage *) override;
    grpc::Status testMethodNonCompatibleArgRet(grpc::ServerContext *,
                                               const SimpleIntMessage *request,
                                               SimpleStringMessage *response) override;
    grpc::Status testMetadata(grpc::ServerContext *,
                                                const Empty *,
                                                qtgrpc::tests::Empty *) override;

    grpc::Status
    testMethodClientStream(::grpc::ServerContext *,
                           ::grpc::ServerReader<::qtgrpc::tests::SimpleStringMessage> *reader,
                           ::qtgrpc::tests::SimpleStringMessage *response) override;

    grpc::Status testMethodBiStream(
            ::grpc::ServerContext *context,
            ::grpc::ServerReaderWriter<::qtgrpc::tests::SimpleStringMessage,
                                       ::qtgrpc::tests::SimpleStringMessage> *stream) override;
    grpc::Status
    testMethodClientStreamWithDone(::grpc::ServerContext *,
                           ::grpc::ServerReader<::qtgrpc::tests::SimpleStringMessage> *reader,
                           ::qtgrpc::tests::SimpleStringMessage *response) override;

    grpc::Status testMethodBiStreamWithDone(
        ::grpc::ServerContext *context,
        ::grpc::ServerReaderWriter<::qtgrpc::tests::SimpleStringMessage,
                                   ::qtgrpc::tests::SimpleStringMessage> *stream) override;

    grpc::Status testMethodSleep(grpc::ServerContext *context,
                                 const qtgrpc::tests::SleepMessage *request,
                                 qtgrpc::tests::Empty * /* response */) override
    {
        if (request->has_sleeptimems())
            QThread::msleep(request->sleeptimems());

        if (context->IsCancelled())
            return { grpc::StatusCode::CANCELLED, "testMethodSleep" };
        return grpc::Status::OK;
    }

    grpc::Status
    testMethodServerStreamSleep(grpc::ServerContext *context,
                                const qtgrpc::tests::ServerStreamSleepMessage *request,
                                grpc::ServerWriter<qtgrpc::tests::Empty> *writer) override
    {
        size_t count = 0;
        while (count < request->amountresponses()) {
            if (request->sleepmessage().has_sleeptimems())
                QThread::msleep(request->sleepmessage().sleeptimems());
            writer->Write(qtgrpc::tests::Empty());
            ++count;
        }

        if (context->IsCancelled())
            return { grpc::StatusCode::CANCELLED, "testMethodServerStreamSleep" };
        return grpc::Status::OK;
    }

    grpc::Status
    testMethodClientStreamSleep(grpc::ServerContext *context,
                                grpc::ServerReader<qtgrpc::tests::SleepMessage> *reader,
                                qtgrpc::tests::Empty * /* response */) override
    {
        qtgrpc::tests::SleepMessage request;
        while (reader->Read(&request)) {
            if (request.has_sleeptimems())
                QThread::msleep(request.sleeptimems());
        }

        if (context->IsCancelled())
            return { grpc::StatusCode::CANCELLED, "testMethodClientStreamSleep" };
        return grpc::Status::OK;
    }

    grpc::Status
    testMethodBiStreamSleep(grpc::ServerContext *context,
                            grpc::ServerReaderWriter<qtgrpc::tests::Empty,
                                                     qtgrpc::tests::SleepMessage> *stream) override
    {
        qtgrpc::tests::SleepMessage request;

        while (stream->Read(&request)) {
            if (request.has_sleeptimems())
                QThread::msleep(request.sleeptimems());
            if (!stream->Write(Empty()))
                return { grpc::StatusCode::ABORTED, "testMethodBiStreamSleep failed to write" };
        }

        if (context->IsCancelled())
            return { grpc::StatusCode::CANCELLED, "testMethodBiStreamSleep" };
        return grpc::Status::OK;
    }

    ::grpc::Status replyWithMetadata(::grpc::ServerContext *ctx, const ::qtgrpc::tests::Empty *,
                                     ::qtgrpc::tests::MetadataMessage *response) override
    {
        qInfo() << "replyWithMetadata called";
        for (const auto &header : ctx->client_metadata()) {
            auto *headerValue = response->mutable_values()->Add();
            headerValue->set_key(std::string(header.first.data(), header.first.size()));
            headerValue->set_value(std::string(header.second.data(), header.second.size()));
        }

        return Status();
    }

    qint64 m_latency;
};
}

Status TestServiceServiceImpl::testMethod(grpc::ServerContext *, const SimpleStringMessage *request,
                                          SimpleStringMessage *response)
{
    qInfo() << "testMethod called with: " << request->testfieldstring();
    response->set_testfieldstring(request->testfieldstring());
    if (request->testfieldstring() == "sleep") {
        QThread::msleep(m_latency);
    }
    return Status();
}

Status TestServiceServiceImpl::testMethodServerStream(grpc::ServerContext *,
                                                      const SimpleStringMessage *request,
                                                      ServerWriter<SimpleStringMessage> *writer)

{
    qInfo() << "testMethodServerStream called with: " << request->testfieldstring();
    SimpleStringMessage msg;

    msg.set_testfieldstring(request->testfieldstring() + "1");
    QThread::msleep(m_latency);
    qInfo() << "send back " << (request->testfieldstring() + "1");
    writer->Write(msg);

    msg.set_testfieldstring(request->testfieldstring() + "2");
    QThread::msleep(m_latency);
    qInfo() << "send back " << (request->testfieldstring() + "2");
    writer->Write(msg);

    msg.set_testfieldstring(request->testfieldstring() + "3");
    QThread::msleep(m_latency);
    qInfo() << "send back " << (request->testfieldstring() + "3");
    writer->Write(msg);

    msg.set_testfieldstring(request->testfieldstring() + "4");
    QThread::msleep(m_latency);
    qInfo() << "send back " << (request->testfieldstring() + "4");
    writer->WriteLast(msg, grpc::WriteOptions());

    return Status();
}

Status TestServiceServiceImpl::testMethodBlobServerStream(grpc::ServerContext *,
                                                          const BlobMessage *request,
                                                          ServerWriter<BlobMessage> *writer)

{
    qInfo() << "testMethodBlobServerStream called with testbytes size: "
            << request->testbytes().size();
    BlobMessage msg;
    msg.set_allocated_testbytes(new std::string(request->testbytes()));
    writer->Write(msg);
    return Status();
}

Status TestServiceServiceImpl::testMethodStatusMessage(grpc::ServerContext *,
                                                       const SimpleStringMessage *request,
                                                       SimpleStringMessage *)
{
    return Status(grpc::StatusCode::UNIMPLEMENTED, request->testfieldstring());
}

Status TestServiceServiceImpl::testMethodNonCompatibleArgRet(grpc::ServerContext *,
                                                             const SimpleIntMessage *request,
                                                             SimpleStringMessage *response)
{
    qInfo() << "testMethodNonCompatibleArgRet called with: " << request->testfield();
    response->set_testfieldstring(std::to_string(request->testfield()));
    return Status();
}

Status TestServiceServiceImpl::testMetadata(grpc::ServerContext *ctx, const Empty *,
                                            qtgrpc::tests::Empty *)
{
    const auto &md = ctx->client_metadata();
    auto initial = md.find("request_initial");
    uint initialCount = 0;
    if (initial != md.end()) {
        std::string v (initial->second.begin(), initial->second.end());
        auto result = std::from_chars(v.data(), v.data() + v.size(), initialCount);
        if (result.ec != std::errc())
            return { grpc::StatusCode::ABORTED, "conversion failed initial" };
    }

    auto trailing = md.find("request_trailing");
    uint trailingCount = 0;
    if (trailing != md.end()) {
        std::string v (trailing->second.begin(), trailing->second.end());
        auto result = std::from_chars(v.data(), v.data() + v.size(), trailingCount);
        if (result.ec != std::errc())
            return { grpc::StatusCode::ABORTED, "conversion failed trailing" };
    }

    auto sum = md.equal_range("request_sum");
    uint sumCount = 0;
    while (sum.first != sum.second) {
        uint temp = 0;
        std::string v (sum.first->second.begin(), sum.first->second.end());
        auto result = std::from_chars(v.data(), v.data() + v.size(), temp);
        if (result.ec != std::errc())
            return { grpc::StatusCode::ABORTED, "conversion failed sum" };
        sumCount += temp;
        ++sum.first;
    }

    for (auto i = 0u; i < initialCount; ++i)
        ctx->AddInitialMetadata("response_initial", std::to_string(i));
    for (auto i = 0u; i < trailingCount; ++i)
        ctx->AddTrailingMetadata("response_trailing", std::to_string(i));
    if (sumCount > 0)
        ctx->AddTrailingMetadata("response_sum", std::to_string(sumCount));

    return Status::OK;
}

grpc::Status TestServiceServiceImpl::testMethodClientStream(
        ::grpc::ServerContext *, ::grpc::ServerReader<::qtgrpc::tests::SimpleStringMessage> *reader,
        ::qtgrpc::tests::SimpleStringMessage *response)
{
    ::qtgrpc::tests::SimpleStringMessage req;
    std::string rspString;
    for (int i = 0; i < 4; ++i) {
        if (!reader->Read(&req)) {
            qInfo() << "Unable to read message from client stream";
            return grpc::Status(grpc::StatusCode::DATA_LOSS, rspString);
        }
        rspString += req.testfieldstring();
        rspString += std::to_string(i + 1);
    }

    response->set_testfieldstring(rspString);
    return {};
}

grpc::Status TestServiceServiceImpl::testMethodBiStream(
        ::grpc::ServerContext *,
        ::grpc::ServerReaderWriter<::qtgrpc::tests::SimpleStringMessage,
                                   ::qtgrpc::tests::SimpleStringMessage> *stream)
{
    ::qtgrpc::tests::SimpleStringMessage req;
    for (int i = 0; i < 4; ++i) {
        if (!stream->Read(&req)) {
            qInfo() << "Unable to read message from bidirectional stream";
            return grpc::Status(grpc::StatusCode::DATA_LOSS, "Read failed");
        }
        std::string rspString = req.testfieldstring() + std::to_string(i + 1);
        ::qtgrpc::tests::SimpleStringMessage rsp;
        rsp.set_testfieldstring(rspString);
        if (!stream->Write(rsp, {})) {
            qInfo() << "Unable to write message to bidirectional stream";
            return grpc::Status(grpc::StatusCode::DATA_LOSS, "Write failed");
        }
        QThread::msleep(m_latency);
    }

    return {};
}

grpc::Status TestServiceServiceImpl::testMethodClientStreamWithDone(
    ::grpc::ServerContext *, ::grpc::ServerReader<::qtgrpc::tests::SimpleStringMessage> *reader,
    ::qtgrpc::tests::SimpleStringMessage *response)
{
    ::qtgrpc::tests::SimpleStringMessage req;
    std::string rspString;
    int i = 0;
    while (reader->Read(&req)) {
        rspString += req.testfieldstring();
        rspString += std::to_string(++i);
    }

    response->set_testfieldstring(rspString);
    return {};
}

grpc::Status TestServiceServiceImpl::testMethodBiStreamWithDone(
    ::grpc::ServerContext *,
    ::grpc::ServerReaderWriter<::qtgrpc::tests::SimpleStringMessage,
                               ::qtgrpc::tests::SimpleStringMessage> *stream)
{
    ::qtgrpc::tests::SimpleStringMessage req;
    int i = 0;
    while (stream->Read(&req)) {
        std::string rspString = req.testfieldstring() + std::to_string(++i);
        ::qtgrpc::tests::SimpleStringMessage rsp;
        rsp.set_testfieldstring(rspString);
        if (!stream->Write(rsp, {})) {
            qInfo() << "Unable to write message to bidirectional stream";
            return grpc::Status(grpc::StatusCode::DATA_LOSS, "Write failed");
        }
        QThread::msleep(m_latency);
    }

    return {};
}

void TestServer::run(qint64 latency)
{
    TestServiceServiceImpl service(latency);

    grpc::ServerBuilder builder;
    QFile cfile(":/assets/cert.pem");
    if (!cfile.open(QFile::ReadOnly)) {
        qDebug() << "Unable to open SSL certificate. Test lacks resources.";
        return;
    }
    QString cert = cfile.readAll();

    QFile kfile(":/assets/key.pem");
    if (!kfile.open(QFile::ReadOnly)){
        qDebug() << "Unable to open SSL key. Test lacks resources.";
        return;
    }
    QString key = kfile.readAll();

    grpc::SslServerCredentialsOptions opts(GRPC_SSL_DONT_REQUEST_CLIENT_CERTIFICATE);
    opts.pem_key_cert_pairs.push_back({ key.toStdString(), cert.toStdString() });

    QString httpURI("127.0.0.1:50051");
    builder.AddListeningPort(httpURI.toStdString(), grpc::InsecureServerCredentials());
    QString httpsURI("127.0.0.1:50052");
    builder.AddListeningPort(httpsURI.toStdString(), grpc::SslServerCredentials(opts));
#ifndef Q_OS_WINDOWS
    QString unixUri("unix:///tmp/qtgrpc_test.sock");
    builder.AddListeningPort(unixUri.toStdString(), grpc::InsecureServerCredentials());
#endif
    builder.RegisterService(&service);

    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
    if (!server) {
        qDebug() << "Creating grpc::Server failed.";
        return;
    }
#ifdef Q_OS_WINDOWS
    qDebug() << "Server listening on " << httpURI << httpsURI;
#else
    qDebug() << "Server listening on " << httpURI << httpsURI << "and" << unixUri;
#endif
    server->Wait();
}
