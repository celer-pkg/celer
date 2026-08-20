// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <conformance.qpb.h>
#include <test_messages_proto3.qpb.h>

#include <cstdio>
#include <memory>

#include <QtCore/QCoreApplication>
#include <QtCore/QSocketNotifier>
#include <QtCore/QtTypeTraits>

#include <QtProtobuf/QProtobufJsonSerializer>
#include <QtProtobuf/QProtobufSerializer>

class ConformaceServer : public QObject
{
public:
    ConformaceServer();
    void readStdin(int socket, QSocketNotifier::Type type);

    QByteArray runTest(const QByteArray &reqData);

private:
    QProtobufSerializer m_protoSerializer;
    QProtobufJsonSerializer m_jsonSerializer;

    std::unique_ptr<QSocketNotifier> m_stdinNotifier;
};

ConformaceServer::ConformaceServer()
    : QObject(),
      m_stdinNotifier(std::make_unique<QSocketNotifier>(fileno(stdin), QSocketNotifier::Read, this))
{
    connect(m_stdinNotifier.get(), &QSocketNotifier::activated, this, &ConformaceServer::readStdin);
}

void ConformaceServer::readStdin(int, QSocketNotifier::Type)
{
    uint32_t dataSize = 0;
    if (fread(&dataSize, sizeof(dataSize), 1, stdin) != 1) {
        fprintf(stderr, "Unable to read the message size from stdin. Assume the channel is closed.\n");
        QCoreApplication::quit();
        return;
    }

    if (dataSize == 0) {
        fprintf(stderr, "Received message of size 0. Potential issue\n");
        return;
    }

    QByteArray reqData(dataSize, Qt::Uninitialized);
    if (fread(reqData.data(), sizeof(char), dataSize, stdin) != dataSize) {
        fprintf(stderr, "Unable read message of size %d\n", dataSize);
    }

    auto respData = runTest(reqData);
    dataSize = respData.size();

    if (fwrite(&dataSize, sizeof(dataSize), 1, stdout) != 1) {
        fprintf(stderr, "Unable to write the message size to stdout\n");
        QCoreApplication::exit(1);
        return;
    }

    if (fwrite(respData.constData(), sizeof(char), dataSize, stdout) != dataSize) {
        fprintf(stderr, "Unable to write the message to stdout\n");
        QCoreApplication::exit(1);
        return;
    }
    fflush(stdout);
}

QByteArray ConformaceServer::runTest(const QByteArray &reqData)
{

    conformance::ConformanceRequest request;
    request.deserialize(&m_protoSerializer, reqData);

    conformance::ConformanceResponse response;

    if (request.requestedOutputFormat() != conformance::WireFormatGadget::WireFormat::PROTOBUF
        && request.requestedOutputFormat() != conformance::WireFormatGadget::WireFormat::JSON) {
        response.setSkipped(QString("Unsupported response format %1")
                                .arg(qToUnderlying(request.requestedOutputFormat())));
        return response.serialize(&m_protoSerializer);
    }

    if (conformance::ConformanceRequest::PayloadFields::ProtobufPayload != request.payloadField()
        && conformance::ConformanceRequest::PayloadFields::JsonPayload != request.payloadField()) {
        response.setSkipped(QString("Unsuported payload type %1").arg(int(request.payloadField())));
        return response.serialize(&m_protoSerializer);
    }

    bool isProtoInput = conformance::ConformanceRequest::PayloadFields::ProtobufPayload
        == request.payloadField();
    bool isProtoOutput = request.requestedOutputFormat()
        == conformance::WireFormatGadget::WireFormat::PROTOBUF;

    QAbstractProtobufSerializer *activeSerializer = isProtoOutput
        ? static_cast<QAbstractProtobufSerializer *>(&m_protoSerializer)
        : static_cast<QAbstractProtobufSerializer *>(&m_jsonSerializer);
    QAbstractProtobufSerializer *activeDeserializer = isProtoInput
        ? static_cast<QAbstractProtobufSerializer *>(&m_protoSerializer)
        : static_cast<QAbstractProtobufSerializer *>(&m_jsonSerializer);

    auto msg = QProtobufMessage::constructByName(request.messageType());
    if (!msg) {
        response.setSkipped(QString("Unknown message %1").arg(request.messageType()));
        return response.serialize(&m_protoSerializer);
    }

    if (request.messageType() == "conformance.FailureSet") {
        conformance::FailureSet failures;
        response.setProtobufPayload(failures.serialize(&m_protoSerializer));
        return response.serialize(&m_protoSerializer);
    }
    QByteArray payload = isProtoInput ? request.protobufPayload() : request.jsonPayload().toUtf8();

    if (!activeDeserializer->deserialize(msg.get(), payload)
        || activeDeserializer->lastError() != QAbstractProtobufSerializer::Error::None) {
        response.setParseError(activeDeserializer->lastErrorString());
        return response.serialize(&m_protoSerializer);
    }

    QByteArray result = activeSerializer->serialize(msg.get());
    if (isProtoOutput)
        response.setProtobufPayload(result);
    else {
        response.setJsonPayload(QString::fromUtf8(result));
    }
    return response.serialize(&m_protoSerializer);
}

int main(int argc, char *argv[])
{
    // Ensure we don't output anything to stdout
    qInstallMessageHandler([](QtMsgType, const QMessageLogContext &context, const QString &msg) {
        QByteArray localMsg = msg.toLocal8Bit();
        const char *file = context.file ? context.file : "";
        const char *function = context.function ? context.function : "";
        fprintf(stderr, "%s (%s:%u, %s)\n", localMsg.constData(), file, context.line, function);
        return;
    });

    QCoreApplication app(argc, argv);
    ConformaceServer server;
    return app.exec();
}
