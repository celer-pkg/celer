// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qiopipe.h"

#include <QtJsonRpc/private/qjsonrpcprotocol_p.h>
#include <QtLanguageServer/private/qlanguageserverjsonrpctransport_p.h>
#include <QtLanguageServer/private/qlanguageserverprotocol_p.h>

#include <QtCore/qjsonarray.h>
#include <QtCore/qjsonobject.h>
#include <QtCore/qjsondocument.h>
#include <QtCore/qstring.h>
#include <QtTest/qsignalspy.h>
#include <QtTest/qtest.h>

using namespace Qt::StringLiterals;
using namespace QLspSpecification;

class TestRigEventHandler
{
public:
    TestRigEventHandler(QLanguageServerProtocol *protocol, QIODevice *device) : m_device(device)
    {
        protocol->registerInitializeRequestHandler(
                    [this](const QByteArray &, const InitializeParams &clientCapabilities,
                           auto &&response) {
            Q_UNUSED(clientCapabilities);
            m_isInitialized = true;
            InitializeResult result;
            result.capabilities = m_serverCapabilities;
            response.sendResponse(result);
        });
        protocol->registerInitializedNotificationHandler(
                [](const QByteArray &, const InitializedParams &) {});
        protocol->registerShutdownRequestHandler(
                [](const QByteArray &, const auto &, auto &&response) { response.sendResponse(); });
        protocol->registerExitNotificationHandler(
                [this](const QByteArray &, const auto &) { m_hasExited = true; });

        // TODO: There should be a way to do this without using the typed RPC layer.
        protocol->typedRpc()->installMessagePreprocessor(
                    [this](const QJsonDocument &doc, const QJsonParseError &,
                           const QJsonRpcProtocol::Handler<QJsonRpcProtocol::Response> &responder) {
            if (m_isInitialized)
                return QJsonRpcProtocol::Processing::Continue;

            if (doc.object()[u"method"].toString()
                    == QString::fromUtf8(QLspSpecification::Requests::InitializeMethod)) {
                return QJsonRpcProtocol::Processing::Continue;
            }

            responder(QJsonRpcProtocol::MessageHandler::error(
                          int(QLspSpecification::ErrorCodes::ServerNotInitialized),
                          u"Method called on non initialized Language Server"_s));
            return QJsonRpcProtocol::Processing::Stop;
        });

        QObject::connect(device, &QIODevice::readyRead, m_device,
                         [this, protocol]() { protocol->receiveData(m_device->readAll()); });
    }

    bool isInitialized() const { return m_isInitialized; }
    bool hasExited() const { return m_hasExited; }

    void setServerCapabilities(QLspSpecification::ServerCapabilities capabilities)
    {
        m_serverCapabilities = std::move(capabilities);
    }

    QLspSpecification::ServerCapabilities serverCapabilities() const
    {
        return m_serverCapabilities;
    }

private:
    QLspSpecification::ServerCapabilities m_serverCapabilities;
    QIODevice *m_device = nullptr;
    bool m_isInitialized = false;
    bool m_hasExited = false;
};

static void setTransportDevice(QLanguageServerJsonRpcTransport *transport, QIODevice *device)
{
    transport->setDataHandler([device](const QByteArray &data) {
        device->write(data);
    });

    QObject::connect(device, &QIODevice::readyRead, device, [device, transport]() {
        transport->receiveData(device->readAll());
    });
}

static void sendAndWaitJsonRpc(QJsonRpcProtocol *protocol,
                               const QJsonRpcProtocol::Request &request,
                               const QJsonRpcProtocol::Handler<QJsonRpcProtocol::Response> &handler)
{
    bool handled = false;
    protocol->sendRequest(request, [&](const QJsonRpcProtocol::Response &response) {
        handled = true;
        handler(response);
    });
    QTRY_VERIFY(handled);
}

class TestRig
{
public:
    QLanguageServerProtocol protocol;
    QJsonRpcProtocol client;
    std::unique_ptr<TestRigEventHandler> handler;

    TestRig() : protocol([this](const QByteArray &data) { pipe.end1()->write(data); })
    {
        handler = std::make_unique<TestRigEventHandler>(&protocol, pipe.end1());
        setTransportDevice(&clientTransport, pipe.end2());
        client.setTransport(&clientTransport);
    }

    void open(const QLspSpecification::ServerCapabilities &capabilities
              = QLspSpecification::ServerCapabilities())
    {
        handler->setServerCapabilities(capabilities);
        QVERIFY(pipe.open(QIODevice::ReadWrite));
    }

    void initialize(const QLspSpecification::ClientCapabilities &clientCapabilities
                    = QLspSpecification::ClientCapabilities())
    {
        QLspSpecification::InitializeParams params;
        params.capabilities = clientCapabilities;
        sendAndWaitJsonRpc(&client, QJsonRpcProtocol::Request {
            0,
            "initialize",
            QTypedJson::toJsonValue(params)
        }, [&](const QJsonRpcProtocol::Response &response) {
            QCOMPARE(response.data["capabilities"],
                    QTypedJson::toJsonValue(handler->serverCapabilities()));
        });
    }

private:
    QLanguageServerJsonRpcTransport clientTransport;
    QIOPipe pipe;
};

class tst_QLanguageServer : public QObject
{
    Q_OBJECT

private slots:
    void jsonRpcTransport();
    void jsonRpcTransportHeaderCase();
    void invalidHeaderField();
    void protocolHandlesTransportErrors();
    void invalidJson();
    void protocolError();

    void lifecycle();
    void windowShowMessage();
    void windowShowMessageRequest();
    void windowLogMessage();
    void telemetryEvent();

    void clientRegisterCapability();
    void setRequestHandler();

private:
    void logOrShowMessage(const QString &method);
};

class NotificationHandler : public QJsonRpcProtocol::MessageHandler
{
public:
    using Handler = std::function<void(QJsonRpcProtocol::Notification)>;

    NotificationHandler(const Handler &handler) : m_notificationHandler(handler) {}

    void handleRequest(const QJsonRpcProtocol::Request &request, const ResponseHandler &handler) final;
    void handleNotification(const QJsonRpcProtocol::Notification &notification) final;

private:
    Handler m_notificationHandler;
};

class RequestHandler : public QJsonRpcProtocol::MessageHandler
{
public:
    using Handler = std::function<QJsonRpcProtocol::Response(QJsonRpcProtocol::Request)>;

    RequestHandler(const Handler &handler) : m_requestHandler(handler) {}

    void handleRequest(const QJsonRpcProtocol::Request &request,
                       const ResponseHandler &handler) final;
    void handleNotification(const QJsonRpcProtocol::Notification &notification) final;

private:
    Handler m_requestHandler;
};

void NotificationHandler::handleRequest(const QJsonRpcProtocol::Request &request,
                                        const ResponseHandler &responseHandler)
{
    Q_UNUSED(request);
    Q_UNUSED(responseHandler);
    QFAIL("request received when notification was expected");
}

void NotificationHandler::handleNotification(const QJsonRpcProtocol::Notification &notification)
{
    m_notificationHandler(notification);
}


void RequestHandler::handleRequest(const QJsonRpcProtocol::Request &request,
                                   const ResponseHandler &responseHandler)
{
    responseHandler(m_requestHandler(request));
}

void RequestHandler::handleNotification(const QJsonRpcProtocol::Notification &notification)
{
    Q_UNUSED(notification);
    QFAIL("notification received when request was expected");
}

void tst_QLanguageServer::jsonRpcTransport()
{
    static const QByteArray header = "Content-Length: 15\r\n\r\n";
    static const QByteArray content = "{\"some\":\"json\"}";

    {
        QIOPipe pipe;
        QVERIFY(pipe.open(QIODevice::WriteOnly));

        QLanguageServerJsonRpcTransport transport;
        setTransportDevice(&transport, pipe.end2());

        int messages = 0;
        transport.setMessageHandler([&](const QJsonDocument &message,
                                        const QJsonParseError &error) {
            QCOMPARE(error.error, QJsonParseError::NoError);
            QCOMPARE(message.toJson(QJsonDocument::Compact), content);
            ++messages;
        });

        for (int i = 0; i < 5; ++i) {
            QCOMPARE(pipe.end1()->write(header), header.size());
            QCOMPARE(pipe.end1()->write(content), content.size());

            QTRY_COMPARE(messages, i + 1);
        }
    }

    {
        QIOPipe pipe;
        QVERIFY(pipe.open(QIODevice::ReadOnly));

        QByteArray received;
        QIODevice *end1 = pipe.end1();
        connect(end1, &QIODevice::readyRead, end1, [&]() {
            received.append(end1->read(end1->bytesAvailable()));
        });

        QLanguageServerJsonRpcTransport protocol;
        setTransportDevice(&protocol, pipe.end2());

        for (int i = 0; i < 5; ++i) {
            protocol.sendMessage(QJsonDocument::fromJson(content));
            QTRY_COMPARE(received, header + content);
            received.clear();
        }
    }
}

void tst_QLanguageServer::jsonRpcTransportHeaderCase()
{
    static const QList<QByteArray> headers = { { "Content-Length: 15\r\n\r\n" },
                                               { "content-length: 15\r\n\r\n" },
                                               { "ConTEnt-lEngTh:  15 \r\n\r\n" },
                                               { "CONTENT-LENGTH:  15\r\n\r\n" },
                                               { "cONtENt-LENgTh: 15  \r\n\r\n" } };
    static const QByteArray content = "{\"some\":\"json\"}";

    {
        QIOPipe pipe;
        QVERIFY(pipe.open(QIODevice::WriteOnly));

        QLanguageServerJsonRpcTransport transport;
        setTransportDevice(&transport, pipe.end2());

        int messages = 0;
        transport.setMessageHandler(
                [&](const QJsonDocument &message, const QJsonParseError &error) {
                    QCOMPARE(error.error, QJsonParseError::NoError);
                    QCOMPARE(message.toJson(QJsonDocument::Compact), content);
                    ++messages;
                });

        for (int i = 0; i < 5; ++i) {
            QCOMPARE(pipe.end1()->write(headers.at(i)), headers.at(i).size());
            QCOMPARE(pipe.end1()->write(content), content.size());

            QTRY_COMPARE(messages, i + 1);
        }
    }

    {
        QIOPipe pipe;
        QVERIFY(pipe.open(QIODevice::ReadOnly));

        QByteArray received;
        QIODevice *end1 = pipe.end1();
        connect(end1, &QIODevice::readyRead, end1,
                [&]() { received.append(end1->read(end1->bytesAvailable())); });

        QLanguageServerJsonRpcTransport protocol;
        setTransportDevice(&protocol, pipe.end2());

        for (int i = 0; i < 5; ++i) {
            protocol.sendMessage(QJsonDocument::fromJson(content));
            QTRY_COMPARE(received, headers.at(0) + content);
            received.clear();
        }
    }
}

void tst_QLanguageServer::invalidHeaderField()
{
    static const QByteArray header = "Broken-Mess\r\n"
                                     "Content-Length: 15\r\n\r\n";
    static const QByteArray content = "{\"some\":\"json\"}";

    QIOPipe pipe;

    QVERIFY(pipe.open(QIODevice::ReadWrite));

    QLanguageServerJsonRpcTransport transport;
    setTransportDevice(&transport, pipe.end1());

    int messages = 0;
    transport.setMessageHandler([&](const QJsonDocument &message,
                                    const QJsonParseError &error) {
        QCOMPARE(error.error, QJsonParseError::NoError);
        QCOMPARE(message.toJson(QJsonDocument::Compact), content);
        ++messages;
    });

    int warnings = 0;
    transport.setDiagnosticHandler([&](QJsonRpcTransport::DiagnosticLevel level,
                                   const QString &message) {
        QVERIFY2(level == QJsonRpcTransport::Warning, qPrintable(message));
        ++warnings;
    });

    QIODevice *end2 = pipe.end2();
    for (int i = 0; i < 5; ++i) {
        QCOMPARE(end2->write(header), header.size());
        QCOMPARE(end2->write(content), content.size());

        QTRY_COMPARE(messages, i + 1);
        QCOMPARE(warnings, 2 * (i + 1));
    }
}

void tst_QLanguageServer::protocolHandlesTransportErrors()
{
    static const QByteArray header = "Broken-Mess\r\n"
                                     "Content-Length: 15\r\n\r\n";
    static const QByteArray content = "{\"some\":\"json\"}";

    QIOPipe pipe;

    QVERIFY(pipe.open(QIODevice::ReadWrite));

    auto device = pipe.end1();
    QLanguageServerProtocol protocol([device](const QByteArray &data) { device->write(data); });

    QObject::connect(device, &QIODevice::readyRead, device,
                     [device, &protocol]() { protocol.receiveData(device->readAll()); });

    int warnings = 0;
    protocol.registerResponseErrorHandler(
            [&](const QLspSpecification::ResponseError &) { ++warnings; });

    QIODevice *end2 = pipe.end2();
    for (int i = 0; i < 5; ++i) {
        QCOMPARE(end2->write(header), header.size());
        QCOMPARE(end2->write(content), content.size());
        QTRY_COMPARE(warnings, 2 * (i + 1));
    }
}

void tst_QLanguageServer::invalidJson()
{
    static const QByteArray header = "Content-Length: 15\r\n\r\n";
    static const QByteArray content = "{\"someZ:\"json\"}";

    QIOPipe pipe;
    QVERIFY(pipe.open(QIODevice::ReadWrite));

    QLanguageServerJsonRpcTransport transport;
    setTransportDevice(&transport, pipe.end2());

    int messages = 0;
    transport.setMessageHandler([&](const QJsonDocument &doc, const QJsonParseError &error) {
        QVERIFY(error.error != QJsonParseError::NoError);
        QVERIFY(doc.isNull());
        ++messages;
    });

    QIODevice *end1 = pipe.end1();
    for (int i = 0; i < 5; ++i) {
        QCOMPARE(end1->write(header), header.size());
        QCOMPARE(end1->write(content), content.size());
        QTRY_COMPARE(messages, i + 1);
    }
}

void tst_QLanguageServer::protocolError()
{
    static const QByteArray header = "Total-Mess: schalala\r\n\r\n";

    QIOPipe pipe;
    QVERIFY(pipe.open(QIODevice::WriteOnly));

    QLanguageServerJsonRpcTransport transport;
    setTransportDevice(&transport, pipe.end2());

    int errors = 0;
    transport.setDiagnosticHandler([&](QJsonRpcTransport::DiagnosticLevel level, const QString &) {
        if (level == QJsonRpcTransport::Error)
            ++errors;
    });

    int messages = 0;
    transport.setMessageHandler([&](const QJsonDocument &doc, const QJsonParseError &error) {
        Q_UNUSED(doc);
        QVERIFY(error.error != QJsonParseError::NoError);
        ++messages;
    });

    QCOMPARE(pipe.end1()->write(header), header.size());
    QTRY_COMPARE(errors, 1);
    QTRY_COMPARE(messages, 1);
}

void tst_QLanguageServer::lifecycle()
{
    QIOPipe pipe;
    pipe.open(QIODevice::ReadWrite);

    QLanguageServerProtocol server([&pipe](const QByteArray &data) {
        QMetaObject::invokeMethod(pipe.end1(), [&pipe, data]() { pipe.end1()->write(data); });
    });
    TestRigEventHandler handler(&server, pipe.end1());

    QLanguageServerJsonRpcTransport transport;
    setTransportDevice(&transport, pipe.end2());

    QJsonRpcProtocol protocol;
    protocol.setTransport(&transport);

    int id = 0;
    sendAndWaitJsonRpc(&protocol, {
        ++id,
        "initialize",
        QJsonObject({
            { "processId", QJsonValue::Null },
            { "rootUri", QJsonValue::Null },
            { "capabilities", QJsonObject() }
        })
    }, [&](const QJsonRpcProtocol::Response &response) {
        QCOMPARE(response.id, QJsonValue(id));
        QVERIFY2(response.errorMessage.isEmpty(), qPrintable(response.errorMessage));
        QVERIFY(!response.errorCode.isDouble());
        QCOMPARE(QJsonDocument(response.data.toObject()).toJson(QJsonDocument::Compact),
                 "{\"capabilities\":{}}");
    });

    protocol.sendNotification({
        "initialized",
        QJsonObject()
    });

    sendAndWaitJsonRpc(&protocol, {
        ++id,
        "shutdown",
        QJsonValue::Undefined
    }, [&](const QJsonRpcProtocol::Response &response) {
        QCOMPARE(response.id, QJsonValue(id));
        QVERIFY2(response.errorMessage.isEmpty(), qPrintable(response.errorMessage));
        QVERIFY(!response.errorCode.isDouble());
        QVERIFY(response.data.isNull());
    });

    QVERIFY(!handler.hasExited());

    protocol.sendNotification({
        "exit",
        QJsonValue::Undefined
    });

    QTRY_VERIFY(handler.hasExited());
}

void tst_QLanguageServer::logOrShowMessage(const QString &method)
{
    TestRig test;
    test.open();

    const QString message = QStringLiteral("msmsmsmsmsg");
    bool messageReceived = false;
    test.client.setMessageHandler(
                method,
                new NotificationHandler([&](const QJsonRpcProtocol::Notification &notification) {
        const QJsonObject params = notification.params.toObject();
        QCOMPARE(params["type"].toInt(), 4); // MessageType::Log
        QCOMPARE(params["message"].toString(), message);
        messageReceived = true;
    }));

    if (method == "window/showMessage") {
        QLspSpecification::ShowMessageParams params;
        params.type = QLspSpecification::MessageType::Log;
        params.message = message.toUtf8();
        test.protocol.notifyShowMessage(params);
    } else if (method == "window/logMessage") {
        QLspSpecification::LogMessageParams params;
        params.type = QLspSpecification::MessageType::Log;
        params.message = message.toUtf8();
        test.protocol.notifyLogMessage(params);
    } else {
        QFAIL("wrong method");
    }

    QTRY_VERIFY(messageReceived);
}

void tst_QLanguageServer::windowShowMessage()
{
    logOrShowMessage("window/showMessage");
}

void tst_QLanguageServer::windowShowMessageRequest()
{
    TestRig test;
    test.open();

    const QString message = QStringLiteral("msmsmsmsmsg");
    auto item = [](const char *title) {
        QLspSpecification::MessageActionItem item;
        item.title = title;
        return item;
    };

    const QList<QLspSpecification::MessageActionItem> actions({
        item("aaa"), item("bbb"), item("ccc")
    });

    bool messageReceived = false;
    test.client.setMessageHandler(
                "window/showMessageRequest",
                new RequestHandler([&](const QJsonRpcProtocol::Request &request) {
        const QJsonObject params = request.params.toObject();
        [&]() {
            QCOMPARE(params["type"].toInt(), 4); // MessageType::Log
            QCOMPARE(params["message"].toString(), message);
            QCOMPARE(params["actions"].toArray(), QJsonArray({
                QJsonObject{{"title", "aaa"}},
                QJsonObject{{"title", "bbb"}},
                QJsonObject{{"title", "ccc"}},
            }));
        }();
        messageReceived = true;
        const QJsonRpcProtocol::Response response = {
            request.id,
            QJsonObject{{"title", "aaa"}},
            QJsonValue::Undefined,
            QString()
        };
        return response;
    }));

    QLspSpecification::ShowMessageRequestParams params;
    params.type = QLspSpecification::MessageType::Log;
    params.message = message.toUtf8();
    params.actions = actions;

    test.protocol.requestShowMessageRequest(
                params,
                [&](const std::variant<QLspSpecification::MessageActionItem, std::nullptr_t> &r) {
        QCOMPARE(r.index(), std::size_t(0));
        QCOMPARE(std::get<QLspSpecification::MessageActionItem>(r).title, "aaa");
    });

    QTRY_VERIFY(messageReceived);
}

void tst_QLanguageServer::windowLogMessage()
{
    logOrShowMessage("window/logMessage");
}

void tst_QLanguageServer::telemetryEvent()
{
    TestRig test;
    test.open();

    const QJsonObject message { { QStringLiteral("a"), 42 } };
    bool messageReceived = false;
    test.client.setMessageHandler(
                "telemetry/event",
                new NotificationHandler([&](const QJsonRpcProtocol::Notification &notification) {
        QCOMPARE(notification.params, message);
        messageReceived = true;
    }));

    test.protocol.notifyTelemetryEvent(message);
    QTRY_VERIFY(messageReceived);
}

static void checkRequestInvalid(TestRig *test, const QLspSpecification::RegistrationParams &params)
{
    test->protocol.requestRegistration(params, []() {
        QFAIL("server responded to invalid request");
    });
}

static void checkRequestValid(TestRig *test, const QLspSpecification::RegistrationParams &params)
{
    test->protocol.requestRegistration(
                params, [](){}, [](const QLspSpecification::ResponseError &) {
        QFAIL("server failed on valid request");
    });
}

static void checkRequestInvalid(
        TestRig *test, const QLspSpecification::UnregistrationParams &params)
{
    test->protocol.requestUnregistration(params, []() {
        QFAIL("server responded to invalid request");
    });
}

static void checkRequestValid(TestRig *test, const QLspSpecification::UnregistrationParams &params)
{
    test->protocol.requestUnregistration(
                params, [](){}, [](const QLspSpecification::ResponseError &) {
        QFAIL("server failed on valid request");
    });
}

void tst_QLanguageServer::clientRegisterCapability()
{
    TestRig test;
    QLspSpecification::ServerCapabilities serverCapabilities;
    serverCapabilities.referencesProvider = true;
    test.open(serverCapabilities);

    QLspSpecification::RegistrationParams params;
    QLspSpecification::Registration referencesRegistration;
    referencesRegistration.method = "textDocument/references";
    params.registrations = { referencesRegistration };

    QTest::ignoreMessage(QtWarningMsg, QRegularExpression(u"ERROR -32601"_s));
    checkRequestInvalid(&test, params);

    QLspSpecification::ReferenceClientCapabilities references;
    references.dynamicRegistration = true;
    QLspSpecification::TextDocumentClientCapabilities textDocumentCapabilities;
    textDocumentCapabilities.references = references;
    QLspSpecification::ClientCapabilities clientCapabilities;
    clientCapabilities.textDocument = textDocumentCapabilities;

    test.initialize(clientCapabilities);

    QTest::ignoreMessage(QtWarningMsg, QRegularExpression(u"ERROR -32601"_s));
    checkRequestInvalid(&test, params);

    auto handler = [&](const QJsonRpcProtocol::Request &request) {
        [&]() {
            const QJsonObject params = request.params.toObject();
            QCOMPARE(params.length(), 1);
            const QJsonArray registrations = params.begin().value().toArray();
            QCOMPARE(registrations.count(), 1);
            const QJsonObject registration = registrations[0].toObject();
            QCOMPARE(registration["method"].toString(), QStringLiteral("textDocument/references"));
        }();

        const QJsonRpcProtocol::Response response = {
            request.id,
            QJsonValue::Null,
            QJsonValue::Undefined,
            QString()
        };
        return response;
    };

    test.client.setMessageHandler("client/registerCapability", new RequestHandler(handler));

    checkRequestValid(&test, params);

    const QLspSpecification::Registration registration = params.registrations.first();
    QLspSpecification::UnregistrationParams unregisterParams;
    unregisterParams.unregisterations = {
        QLspSpecification::Unregistration { registration.id, registration.method }
    };

    checkRequestInvalid(&test, unregisterParams);

    test.client.setMessageHandler("client/unregisterCapability", new RequestHandler(handler));

    checkRequestValid(&test, unregisterParams);
}

void tst_QLanguageServer::setRequestHandler()
{
    TestRig test;

    const QList<QLspSpecification::Location> locations = {
        QLspSpecification::Location { "a.b.x", QLspSpecification::Range() }
    };

    test.protocol.registerReferenceRequestHandler(
                [locations](const QByteArray &, const QLspSpecification::ReferenceParams &,
                   QLspSpecification::LSPPartialResponse<std::variant<
                        QList<QLspSpecification::Location>, std::nullptr_t>,
                   QList<QLspSpecification::Location>> &&result) {
        result.sendResponse(locations);
    });

    test.open();

    QJsonRpcProtocol::Response response;
    auto requestReferences = [&]() {
        sendAndWaitJsonRpc(&test.client, {
            4,
            "textDocument/references",
            QJsonValue()
        }, [&](const QJsonRpcProtocol::Response &received) {
            response = received;
        });
    };

    requestReferences();
    QCOMPARE(response.errorCode.toInt(), -32002);

    test.initialize();
    QVERIFY(test.handler->isInitialized());
    requestReferences();
    QVERIFY(response.errorCode.isUndefined());
    QCOMPARE(response.data, QTypedJson::toJsonValue(locations));
}

QTEST_MAIN(tst_QLanguageServer)

#include <tst_qlanguageserver.moc>
