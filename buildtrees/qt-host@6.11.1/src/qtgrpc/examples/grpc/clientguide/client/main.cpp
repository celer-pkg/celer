// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
// clang-format off

//! [gen-includes]
#include "clientguide.qpb.h"
#include "clientguide_client.grpc.qpb.h"
//! [gen-includes]
#include "client_wrapper.h"
#include "interceptors.h"

#include <QtGrpc/QGrpcHttp2Channel>
#include <QtGrpc/qgrpcstream.h>

#include <QtCore/QCommandLineParser>
#include <QtCore/QCoreApplication>
#include <QtCore/QDateTime>
#include <QtCore/QDir>
#include <QtCore/QProcess>
#include <QtCore/QThread>
#include <QtCore/QTimer>
#include <QtCore/QUrl>

#include <limits>
#include <memory>

// We use part of the namespace to clarify the source.
using namespace client;
using namespace Qt::Literals::StringLiterals;

void startServerProcess();
QDebug operator<<(QDebug debug, const guide::Response &response);
enum class ExpectedResult { Success, Failure };

class ClientGuide : public QObject
{
public:
    //! [basic-1]
    explicit ClientGuide(std::shared_ptr<QAbstractGrpcChannel> channel)
    {
        m_client.attachChannel(std::move(channel));
    }
    //! [basic-1]

    //! [basic-2]
    static guide::Request createRequest(int32_t num,
                                        ExpectedResult expected = ExpectedResult::Success)
    {
        guide::Request request;
        request.setNum(num);
        // The server-side logic fails the RPC if the time is in the future.
        const auto time = expected == ExpectedResult::Failure ?
            std::numeric_limits<int64_t>::max() : now();
        request.setTime(time);
        return request;
    }
    //! [basic-2]

    //! [unary-0]
    void unaryCall(const guide::Request &request, const QGrpcCallOptions &opts = { })
    {
        std::unique_ptr<QGrpcCallReply> reply = m_client.UnaryCall(request, opts);
        const auto *replyPtr = reply.get();
        connect(
            replyPtr, &QGrpcCallReply::finished, replyPtr,
            [reply = std::move(reply)](const QGrpcStatus &status) {
                if (status.isOk()) {
                    if (const auto response = reply->read<guide::Response>())
                        qDebug() << "Client (UnaryCall) finished, received:" << *response;
                    else
                        qDebug("Client (UnaryCall) deserialization failed");
                } else {
                    qDebug() << "Client (UnaryCall) failed:" << status;
                }
            },
            Qt::SingleShotConnection);
    }
    //! [unary-0]

    //! [sstream-0]
    void serverStreaming(const guide::Request &initialRequest)
    {
        std::unique_ptr<QGrpcServerStream> stream = m_client.ServerStreaming(initialRequest);
        const auto *streamPtr = stream.get();

        connect(
            streamPtr, &QGrpcServerStream::finished, streamPtr,
            [stream = std::move(stream)](const QGrpcStatus &status) {
                if (status.isOk())
                    qDebug("Client (ServerStreaming) finished");
                else
                    qDebug() << "Client (ServerStreaming) failed:" << status;
            },
            Qt::SingleShotConnection);
        //! [sstream-0]
        //! [sstream-1]
        connect(streamPtr, &QGrpcServerStream::messageReceived, streamPtr, [streamPtr] {
            if (const auto response = streamPtr->read<guide::Response>())
                qDebug() << "Client (ServerStream) received:" << *response;
            else
                qDebug("Client (ServerStream) deserialization failed");
        });
    }
    //! [sstream-1]

    // ! [cstream-0]
    void clientStreaming(const guide::Request &initialRequest)
    {
        m_clientStream = m_client.ClientStreaming(initialRequest);
        for (int32_t i = 1; i < 3; ++i)
            m_clientStream->writeMessage(createRequest(initialRequest.num() + i));
        m_clientStream->writesDone();

        connect(m_clientStream.get(), &QGrpcClientStream::finished, m_clientStream.get(),
            [this](const QGrpcStatus &status) {
                if (status.isOk()) {
                    if (const auto response = m_clientStream->read<guide::Response>()) {
                        qDebug() << "Client (ClientStreaming) finished, received:"
                                 << *response;
                    }
                    m_clientStream.reset();
                } else {
                    qDebug() << "Client (ClientStreaming) failed:" << status;
                    qDebug("Restarting the client stream");
                    clientStreaming(createRequest(0));
                }
            });
    }
    // ! [cstream-0]

    // ! [bstream-1]
    void bidirectionalStreaming(const guide::Request &initialRequest)
    {
        m_bidiStream = m_client.BidirectionalStreaming(initialRequest);
        connect(m_bidiStream.get(), &QGrpcBidiStream::finished, this, &ClientGuide::bidiFinished);
        connect(m_bidiStream.get(), &QGrpcBidiStream::messageReceived, this,
                &ClientGuide::bidiMessageReceived);
    }
    // ! [bstream-1]

private slots:
    // ! [bstream-2]
    void bidiFinished(const QGrpcStatus &status)
    {
        if (status.isOk())
            qDebug("Client (BidirectionalStreaming) finished");
        else
            qDebug() << "Client (BidirectionalStreaming) failed:" << status;
        m_bidiStream.reset();
    }
    // ! [bstream-2]

    // ! [bstream-3]
    void bidiMessageReceived()
    {
        if (m_bidiStream->read(&m_bidiResponse)) {
            qDebug() << "Client (BidirectionalStreaming) received:" << m_bidiResponse;
            if (m_bidiResponse.num() > 0) {
                m_bidiStream->writeMessage(createRequest(m_bidiResponse.num() - 1));
                return;
            }
        } else {
            qDebug("Client (BidirectionalStreaming) deserialization failed");
        }
        m_bidiStream->writesDone();
    }
    // ! [bstream-3]

private:
    ClientGuideServiceWrapper m_client;
    std::unique_ptr<QGrpcClientStream> m_clientStream;
    // ! [bstream-0]
    std::unique_ptr<QGrpcBidiStream> m_bidiStream;
    guide::Response m_bidiResponse;
    // ! [bstream-0]
};

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    // Use the -U, -S, -C, -B options to control execution
    QCommandLineParser parser;
    QCommandLineOption enableUnary("U", "Enable UnaryCalls");
    QCommandLineOption enableSStream("S", "Enable ServerStream");
    QCommandLineOption enableCStream("C", "Enable ClientStream");
    QCommandLineOption enableBStream("B", "Enable BiDiStream");
    // Use -I to enable client-side interceptor. Works in combination with -U and -B.
    QCommandLineOption enableInterceptors("I", "Enable Interceptors");

    parser.addHelpOption();
    parser.addOption(enableUnary);
    parser.addOption(enableSStream);
    parser.addOption(enableCStream);
    parser.addOption(enableBStream);
    parser.addOption(enableInterceptors);
    parser.process(app);

    bool defaultRun = !parser.isSet(enableUnary) && !parser.isSet(enableSStream)
        && !parser.isSet(enableCStream) && !parser.isSet(enableBStream);

    qDebug("Welcome to the clientguide!");
    if (parser.isSet(enableInterceptors))
        qDebug("Running with Interceptor support");
    qDebug("Starting the server process ...");
    startServerProcess();

    std::shared_ptr<QGrpcHttp2Channel> channel;
    if (parser.isSet(enableInterceptors)) {
        //! [interceptorchain-1]
        auto interceptors = createInterceptors();
        if (interceptors.isEmpty()) {
            qWarning("Failed to create the interceptor chain");
            return EXIT_FAILURE; // or some other suitable fallback
        }
        channel = std::make_shared<QGrpcHttp2Channel>(
            QUrl("http://localhost:50056"),
            std::move(interceptors)
        );
        //! [interceptorchain-1]
    } else {
        //! [basic-0a]
        channel = std::make_shared<QGrpcHttp2Channel>(
            QUrl("http://localhost:50056")
            /* without channel options. */
        );
        //! [basic-0a]
    }

    //! [basic-0b]
    ClientGuide clientGuide(channel);
    //! [basic-0b]

    if (parser.isSet(enableInterceptors)) {
        //! [interceptors-0a]
        int delayMs = 150;
        //! [interceptors-0a]
        if (defaultRun || parser.isSet(enableUnary)) {
            //! [interceptors-0b]
            for (int i = 1; i < 6; ++i) {
                QTimer::singleShot(delayMs, [&, i] {
                    auto expected = i % 2 == 0 ? ExpectedResult::Failure : ExpectedResult::Success;
                    clientGuide.unaryCall(ClientGuide::createRequest(i, expected));
                });
                delayMs += 150;
            }
            QTimer::singleShot(delayMs, [&] {
                QGrpcCallOptions invalidOpts;
                invalidOpts.addMetadata("huge_key"_ba, "huge_value"_ba.repeated(8'000));
                clientGuide.unaryCall(guide::Request{ }, invalidOpts); // this call will fail.
            });
            delayMs += 150;
            //! [interceptors-0b]
        }

        if (defaultRun || parser.isSet(enableBStream)) {
            //! [interceptors-1]
            QTimer::singleShot(delayMs, [&] {
                clientGuide.bidirectionalStreaming(ClientGuide::createRequest(666));
            });
            //! [interceptors-1]
        }
    } else {
        if (defaultRun || parser.isSet(enableUnary)) {
            //! [unary-1]
            clientGuide.unaryCall(ClientGuide::createRequest(1));
            clientGuide.unaryCall(ClientGuide::createRequest(2, ExpectedResult::Failure));
            clientGuide.unaryCall(ClientGuide::createRequest(3));
            //! [unary-1]
        }

        if (defaultRun || parser.isSet(enableSStream)) {
            //! [sstream-2]
            clientGuide.serverStreaming(ClientGuide::createRequest(3));
            // ! [sstream-2]
        }

        if (defaultRun || parser.isSet(enableCStream)) {
            // ! [cstream-1]
            clientGuide.clientStreaming(ClientGuide::createRequest(0, ExpectedResult::Failure));
            // ! [cstream-1]
        }

        if (defaultRun || parser.isSet(enableBStream)) {
            // ! [bstream-4]
            clientGuide.bidirectionalStreaming(ClientGuide::createRequest(3));
            // ! [bstream-4]
        }
    }

    return QCoreApplication::exec();
}

void startServerProcess()
{
    // For the purpose of this example, we launch the server directly from the
    // client. In a real-world scenario, the server should be running
    // independently, and this code would not be necessary. This approach is
    // used here solely for convenience in demonstrating the full interaction.
    static QProcess serverProcess;
    QObject::connect(&serverProcess, &QProcess::readyReadStandardOutput, [] {
        while (serverProcess.canReadLine()) {
            QByteArray line = serverProcess.readLine().trimmed();
            if (!line.isEmpty())
                qDebug().noquote().nospace() << "    " << line;
        }
    });
    serverProcess.setProcessChannelMode(QProcess::MergedChannels);
    serverProcess.setReadChannel(QProcess::StandardOutput);
    // The server must be located next to the client.
    serverProcess.start(QCoreApplication::applicationDirPath()
            + QDir::separator() + SERVER_FILE_NAME);
    if (!serverProcess.waitForStarted()) {
        qFatal() << "Couldn't start the server: " << serverProcess.errorString();
        exit(EXIT_FAILURE);
    }
    QObject::connect(QCoreApplication::instance(), &QCoreApplication::aboutToQuit, [] {
        if (serverProcess.state() == QProcess::Running) {
            serverProcess.kill();
            serverProcess.waitForFinished(5'000); // 5s timeout
        }
    });
    // give the process some time to properly start up the server
    QThread::sleep(1);
}

QDebug operator<<(QDebug debug, const guide::Response &response)
{
    QDebugStateSaver saver(debug);
    return debug.nospace() << "Response( time: " << response.time()
        << ", num: " << response.num() << " )";
}
