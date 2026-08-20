// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant

#include "qjsonrpcprotocol_p_p.h"

#include <QtCore/qjsonarray.h>
#include <QtCore/qstring.h>
#include <QtCore/qjsonobject.h>

#include <functional>

QT_BEGIN_NAMESPACE

using namespace Qt::StringLiterals;

static QJsonObject createResponse(const QJsonValue &id, const QJsonRpcProtocol::Response &response)
{
    QJsonObject object;
    object.insert(u"jsonrpc", u"2.0"_s);
    object.insert(u"id", id);
    if (response.errorCode.isDouble()) {
        QJsonObject error;
        error.insert(u"code", response.errorCode);
        error.insert(u"message", response.errorMessage);
        if (!response.data.isUndefined())
            error.insert(u"data", response.data);
        object.insert(u"error", error);
    } else {
        object.insert(u"result", response.data);
    }
    return object;
}

static QJsonRpcProtocol::Response
createPredefinedError(QJsonRpcProtocol::ErrorCode code,
                      const QJsonValue &id = QJsonValue::Undefined)
{
    QJsonRpcProtocol::Response response;
    response.errorCode = static_cast<double>(code);
    switch (code) {
    case QJsonRpcProtocol::ErrorCode::ParseError:
        response.errorMessage = u"Parse error"_s;
        break;
    case QJsonRpcProtocol::ErrorCode::InvalidRequest:
        response.errorMessage = u"Invalid Request"_s;
        break;
    case QJsonRpcProtocol::ErrorCode::MethodNotFound:
        response.errorMessage = u"Method not found"_s;
        break;
    case QJsonRpcProtocol::ErrorCode::InvalidParams:
        response.errorMessage = u"Invalid Parameters"_s;
        break;
    case QJsonRpcProtocol::ErrorCode::InternalError:
        response.errorMessage = u"Internal Error"_s;
        break;
    }
    response.id = id;
    return response;
}

static QJsonObject createParseErrorResponse()
{
    return createResponse(QJsonValue::Null,
                          createPredefinedError(QJsonRpcProtocol::ErrorCode::ParseError));
}

static QJsonObject createInvalidRequestResponse(const QJsonValue &id = QJsonValue::Null)
{
    return createResponse(id, createPredefinedError(QJsonRpcProtocol::ErrorCode::InvalidRequest));
}

static QJsonObject createMethodNotFoundResponse(const QJsonValue &id)
{
    return createResponse(id, createPredefinedError(QJsonRpcProtocol::ErrorCode::MethodNotFound));
}

template<typename Notification>
static QJsonObject createNotification(const Notification &notification)
{
    QJsonObject object;
    object.insert(u"jsonrpc", u"2.0"_s);
    object.insert(u"method", notification.method);
    object.insert(u"params", notification.params);
    return object;
}

class RequestBatchHandler
{
    Q_DISABLE_COPY(RequestBatchHandler)
public:
    RequestBatchHandler() = default;
    RequestBatchHandler(RequestBatchHandler &&) noexcept = default;
    RequestBatchHandler &operator=(RequestBatchHandler &&) noexcept = default;
    ~RequestBatchHandler();

    void processMessages(QJsonRpcProtocolPrivate *protocol, const QJsonArray &messages);

private:
    QJsonArray m_finished;
    QJsonRpcTransport *m_transport = nullptr;
    uint m_pending = 0;
};

QJsonRpcProtocol::QJsonRpcProtocol() : d(std::make_unique<QJsonRpcProtocolPrivate>()) { }

QJsonRpcProtocol &QJsonRpcProtocol::operator=(QJsonRpcProtocol &&) noexcept = default;
QJsonRpcProtocol::QJsonRpcProtocol(QJsonRpcProtocol &&) noexcept = default;
QJsonRpcProtocol::~QJsonRpcProtocol() = default;

void QJsonRpcProtocol::setMessageHandler(const QString &method,
                                         QJsonRpcProtocol::MessageHandler *handler)
{
    d->setMessageHandler(method, std::unique_ptr<QJsonRpcProtocol::MessageHandler>(handler));
}

void QJsonRpcProtocol::setDefaultMessageHandler(QJsonRpcProtocol::MessageHandler *handler)
{
    d->setDefaultMessageHandler(std::unique_ptr<QJsonRpcProtocol::MessageHandler>(handler));
}

QJsonRpcProtocol::MessageHandler *QJsonRpcProtocol::messageHandler(const QString &method) const
{
    return d->messageHandler(method);
}

QJsonRpcProtocol::MessageHandler *QJsonRpcProtocol::defaultMessageHandler() const
{
    return d->defaultMessageHandler();
}

template<typename Request>
static QJsonObject createRequest(const Request &request)
{
    QJsonObject object;
    object.insert(u"jsonrpc", u"2.0"_s);
    object.insert(u"id", request.id);
    object.insert(u"method", request.method);
    object.insert(u"params", request.params);
    return object;
}

void QJsonRpcProtocol::sendRequest(const Request &request,
                                   const QJsonRpcProtocol::Handler<Response> &handler)
{
    switch (request.id.type()) {
    case QJsonValue::Null:
    case QJsonValue::Double:
    case QJsonValue::String:
        if (d->addPendingRequest(request.id, handler)) {
            d->sendMessage(createRequest(request));
            return;
        }
        break;
    default:
        break;
    }

    handler(createPredefinedError(QJsonRpcProtocol::ErrorCode::InvalidRequest, request.id));
}

void QJsonRpcProtocol::sendNotification(const QJsonRpcProtocol::Notification &notification)
{
    d->sendMessage(createNotification(notification));
}

void QJsonRpcProtocol::sendBatch(
        QJsonRpcProtocol::Batch &&batch,
        const QJsonRpcProtocol::Handler<QJsonRpcProtocol::Response> &handler)
{
    QJsonArray array;
    for (BatchPrivate::Item &item : batch.d->m_items) {
        if (item.id.isUndefined()) {
            array.append(createNotification(item));
        } else {
            switch (item.id.type()) {
            case QJsonValue::Null:
            case QJsonValue::Double:
            case QJsonValue::String:
                if (d->addPendingRequest(item.id, handler)) {
                    array.append(createRequest(item));
                    break;
                }
                Q_FALLTHROUGH();
            default:
                handler(createPredefinedError(QJsonRpcProtocol::ErrorCode::InvalidRequest,
                                              item.id));
                break;
            }
        }
    }
    if (!array.isEmpty())
        d->sendMessage(array);
}

void QJsonRpcProtocol::setTransport(QJsonRpcTransport *transport)
{
    d->setTransport(transport);
}

void QJsonRpcProtocol::setProtocolErrorHandler(const QJsonRpcProtocol::ResponseHandler &handler)
{
    d->setProtocolErrorHandler(handler);
}

QJsonRpcProtocol::ResponseHandler QJsonRpcProtocol::protocolErrorHandler() const
{
    return d->protocolErrorHandler();
}

void QJsonRpcProtocol::setInvalidResponseHandler(const QJsonRpcProtocol::ResponseHandler &handler)
{
    d->setInvalidResponseHandler(handler);
}

QJsonRpcProtocol::ResponseHandler QJsonRpcProtocol::invalidResponseHandler() const
{
    return d->invalidResponseHandler();
}

/*!
 * \internal
 * \typealias QJsonRpcProtocol::MessagePreprocessor
 * \brief A function preprocessing incoming messages
 *
 * A type representing a function receiving a const QJsonDocument &doc, const QJsonParseError
 * &error, const QJsonRpcProtocol::Handler<Response> &responseHandler) and returning either
 * Processing::Continue or Processing::Stop. Handler can be used to return a response when
 * Processing::Stop is returned and doc is a request, i.e. doc.object().contains(u"id") is true.
 */

QJsonRpcProtocol::MessagePreprocessor QJsonRpcProtocol::messagePreprocessor() const
{
    return d->messagePreprocessor();
}

void QJsonRpcProtocol::installMessagePreprocessor(const QJsonRpcProtocol::MessagePreprocessor &h)
{
    return d->installMessagePreprocessor(h);
}

void QJsonRpcProtocolPrivate::setTransport(QJsonRpcTransport *newTransport)
{
    if (newTransport == m_transport)
        return;

    if (m_transport)
        m_transport->setMessageHandler(nullptr);

    m_transport = newTransport;

    if (m_transport) {
        m_transport->setMessageHandler(
                [this](const QJsonDocument &message, const QJsonParseError &error) {
                    processMessage(message, error);
                });
    }
}

static QJsonRpcProtocol::Request parseRequest(const QJsonObject &object)
{
    QJsonRpcProtocol::Request request;
    request.id = object.value(u"id");
    request.method = object.value(u"method").toString();
    request.params = object.value(u"params");
    return request;
}

static QJsonRpcProtocol::Notification parseNotification(const QJsonObject &object)
{
    QJsonRpcProtocol::Notification notification;
    notification.method = object.value(u"method").toString();
    notification.params = object.value(u"params");
    return notification;
}

void RequestBatchHandler::processMessages(QJsonRpcProtocolPrivate *protocol,
                                          const QJsonArray &messages)
{
    m_transport = protocol->transport();

    for (const QJsonValue &value : messages) {
        if (!value.isObject()) {
            m_finished.append(createInvalidRequestResponse());
            continue;
        }

        QJsonObject object = value.toObject();

        if (!object.contains(u"method") || !object.value(u"method").isString()) {
            m_finished.append(createInvalidRequestResponse());
            continue;
        }

        if (!object.contains(u"id")) {
            QJsonRpcProtocol::Notification notification = parseNotification(object);
            if (QJsonRpcProtocol::MessageHandler *handler =
                        protocol->messageHandler(notification.method)) {
                handler->handleNotification(notification);
            }
            continue;
        }

        QJsonRpcProtocol::Request request = parseRequest(object);
        QJsonRpcProtocol::MessageHandler *handler = protocol->messageHandler(request.method);
        if (!handler) {
            m_finished.append(createMethodNotFoundResponse(request.id));
            continue;
        }

        ++m_pending;
        const QJsonValue id = request.id;
        handler->handleRequest(request, [this, id](const QJsonRpcProtocol::Response &response) {
            m_finished.append(createResponse(id, response));
            bool found = false;
            for (QJsonValueRef entry : m_finished) {
                if (entry.toObject()[u"id"] == id) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                m_finished.append(createResponse(
                        id,
                        { id, QJsonValue::Undefined,
                          QJsonValue(static_cast<int>(QJsonRpcProtocol::ErrorCode::InternalError)),
                          u"Message handler did not produce a result."_s }));
            }

            if (--m_pending == 0)
                delete this;
        });
    }
    if (m_pending == 0)
        delete this;
}

RequestBatchHandler::~RequestBatchHandler()
{
    if (m_transport && !m_finished.isEmpty())
        m_transport->sendMessage(QJsonDocument(m_finished));
}

void QJsonRpcProtocolPrivate::processRequest(const QJsonObject &object)
{
    QJsonRpcProtocol::Request request = parseRequest(object);
    if (auto handler = messageHandler(request.method)) {
        const QJsonValue id = request.id;
        handler->handleRequest(request, [id, this](const QJsonRpcProtocol::Response &response) {
            sendMessage(createResponse(id, response));
        });
    } else {
        sendMessage(createMethodNotFoundResponse(request.id));
    }
}

void QJsonRpcProtocolPrivate::processResponse(const QJsonObject &object)
{
    QJsonRpcProtocol::Response response;

    response.id = object.value(u"id");
    if (object.contains(u"error")) {
        const QJsonObject error = object.value(u"error").toObject();
        response.errorCode = error.value(u"code");
        response.errorMessage = error.value(u"message").toString();
        response.data = error.value(u"data");
    } else if (object.contains(u"result")) {
        response.data = object.value(u"result");
    }

    auto pending = m_pendingRequests.find(response.id);
    if (pending != m_pendingRequests.end()) {
        auto handler = pending->second;
        m_pendingRequests.erase(pending);
        handler(response);
    } else if (response.id.isNull()) {
        if (m_protocolErrorHandler)
            m_protocolErrorHandler(response);
    } else {
        if (m_invalidResponseHandler)
            m_invalidResponseHandler(response);
    }
}

void QJsonRpcProtocolPrivate::processNotification(const QJsonObject &object)
{
    QJsonRpcProtocol::Notification notification = parseNotification(object);
    if (auto handler = messageHandler(notification.method))
        handler->handleNotification(notification);
}

void QJsonRpcProtocolPrivate::processMessage(const QJsonDocument &message,
                                             const QJsonParseError &error)
{
    if (m_messagePreprocessor
        && m_messagePreprocessor(message, error,
                                 [message, this](const QJsonRpcProtocol::Response &r) {
                                     Q_ASSERT(message.object().contains(u"id"));
                                     this->sendMessage(createResponse(message.object()[u"id"], r));
                                 })
                != QJsonRpcProtocol::Processing::Continue) {
        return;
    }
    if (error.error != QJsonParseError::NoError) {
        sendMessage(createParseErrorResponse());
    } else if (message.isObject()) {
        const QJsonObject object = message.object();
        if (object.contains(u"method")) {
            if (!object.value(u"method").isString()) {
                sendMessage(createInvalidRequestResponse());
            } else if (object.contains(u"id")) {
                switch (object.value(u"id").type()) {
                case QJsonValue::Null:
                case QJsonValue::Double:
                case QJsonValue::String:
                    processRequest(object);
                    break;
                default:
                    sendMessage(createInvalidRequestResponse());
                    break;
                }
            } else {
                processNotification(object);
            }
        } else if (object.contains(u"id")) {
            processResponse(object);
        } else {
            sendMessage(createInvalidRequestResponse());
        }
    } else if (message.isArray()) {
        const QJsonArray array = message.array();
        if (array.isEmpty()) {
            sendMessage(createInvalidRequestResponse());
            return;
        }

        for (const QJsonValue &item : array) {
            if (!item.isObject()) {
                (new RequestBatchHandler)->processMessages(this, array); // Will delete itself
                return;
            }

            const QJsonObject object = item.toObject();
            // If it's not clearly a response, consider the whole batch as request-y
            if (object.contains(u"method") || !object.contains(u"id")) {
                (new RequestBatchHandler)->processMessages(this, array); // Will delete itself
                return;
            }
        }

        // As we haven't returned above, we can consider them all responseses now.
        for (const QJsonValue &item : array)
            processResponse(item.toObject());

    } else {
        sendMessage(createInvalidRequestResponse());
    }
}

QJsonRpcProtocol::MessageHandler::MessageHandler() = default;
QJsonRpcProtocol::MessageHandler::~MessageHandler() = default;

void QJsonRpcProtocol::MessageHandler::handleRequest(const QJsonRpcProtocol::Request &request,
                                                     const ResponseHandler &handler)
{
    Q_UNUSED(request);
    handler(error(QJsonRpcProtocol::ErrorCode::MethodNotFound));
}

void QJsonRpcProtocol::MessageHandler::handleNotification(
        const QJsonRpcProtocol::Notification &notification)
{
    Q_UNUSED(notification);
}

QJsonRpcProtocol::Response QJsonRpcProtocol::MessageHandler::error(QJsonRpcProtocol::ErrorCode code)
{
    return createPredefinedError(code);
}

QJsonRpcProtocol::Response QJsonRpcProtocol::MessageHandler::error(int code, const QString &message,
                                                                   const QJsonValue &data)
{
    QJsonRpcProtocol::Response response;
    response.errorCode = code;
    response.errorMessage = message;
    response.data = data;
    return response;
}

QJsonRpcProtocol::Response QJsonRpcProtocol::MessageHandler::result(const QJsonValue &result)
{
    QJsonRpcProtocol::Response response;
    response.data = result;
    return response;
}

QJsonRpcProtocol::Batch::Batch() : d(std::make_unique<BatchPrivate>()) { }
QJsonRpcProtocol::Batch::~Batch() = default;
QJsonRpcProtocol::Batch::Batch(QJsonRpcProtocol::Batch &&) noexcept = default;
QJsonRpcProtocol::Batch &
QJsonRpcProtocol::Batch::operator=(QJsonRpcProtocol::Batch &&) noexcept = default;

void QJsonRpcProtocol::Batch::addNotification(const Notification &notification)
{
    BatchPrivate::Item item;
    item.method = notification.method;
    item.params = notification.params;
    d->m_items.push_back(std::move(item));
}

void QJsonRpcProtocol::Batch::addRequest(const Request &request)
{
    BatchPrivate::Item item;
    item.id = request.id;
    item.method = request.method;
    item.params = request.params;
    d->m_items.push_back(std::move(item));
}

QT_END_NAMESPACE
