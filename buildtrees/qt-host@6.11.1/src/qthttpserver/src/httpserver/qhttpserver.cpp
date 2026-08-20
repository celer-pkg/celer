// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include <QtHttpServer/qhttpserver.h>

#include <QtHttpServer/qhttpserverrequest.h>
#include <QtHttpServer/qhttpserverresponder.h>
#include <QtHttpServer/qhttpserverresponse.h>

#include <private/qhttpserver_p.h>
#include <private/qhttpserverstream_p.h>

#include <QtCore/qloggingcategory.h>

#include <QtNetwork/qtcpsocket.h>

QT_BEGIN_NAMESPACE

Q_STATIC_LOGGING_CATEGORY(lcHS, "qt.httpserver");

QHttpServerPrivate::QHttpServerPrivate(QHttpServer *p) :
    router(p)
{
}

void QHttpServerPrivate::callMissingHandler(const QHttpServerRequest &request,
                                            QHttpServerResponder &responder)
{
    Q_Q(QHttpServer);

    if (missingHandler.context && missingHandler.slotObject &&
        verifyThreadAffinity(missingHandler.context)) {
        void *args[] = { nullptr, const_cast<QHttpServerRequest *>(&request), &responder };
        missingHandler.slotObject->call(const_cast<QObject *>(missingHandler.context.data()), args);
    } else {
        qCDebug(lcHS) << "missing handler:" << request.url().path();
        q->sendResponse(QHttpServerResponder::StatusCode::NotFound, request, std::move(responder));
    }
}

/*!
    \class QHttpServer
    \since 6.4
    \inmodule QtHttpServer
    \brief QHttpServer is a simplified API for QAbstractHttpServer and QHttpServerRouter.

    QHttpServer is used to create a simple HTTP server by registering a range
    of request handlers.

    The \l route function can be used to conveniently add rules to the
    server's \l QHttpServerRouter. To register a handler that is called after
    every request to further process the response use \l
    addAfterRequestHandler, but this mechanism only works for routes returning
    \l QHttpServerResponse or QFuture<QHttpServerResponse>. To register a
    handler for all unhandled requests use \l setMissingHandler.

    Minimal example:

    \code

    QHttpServer server;

    server.route("/", [] () {
        return "hello world";
    });

    auto tcpserver = new QTcpServer();
    if (!tcpserver->listen() || !server.bind(tcpserver)) {
        delete tcpserver;
        return -1;
    }
    qDebug() << "Listening on port" << tcpserver->serverPort();

    \endcode
*/

/*!
    Creates an instance of QHttpServer with parent \a parent.
*/
QHttpServer::QHttpServer(QObject *parent)
    : QAbstractHttpServer(*new QHttpServerPrivate(this), parent)
{
}

/*! \fn template <typename Rule = QHttpServerRouterRule, typename Functor> Rule *QHttpServer::route(const QString &pathPattern, QHttpServerRequest::Methods method, const QObject *context, Functor &&slot)

    This method adds a new routing \c Rule to the server's
    \l{QHttpServerRouter} member. The \c Rule template parameter can be any
    class derived from \l{QHttpServerRouterRule}. The parameters are passed
    to \c Rule. The server matches incoming HTTP requests against registered
    rules based on the URL path and HTTP method, and the first match of both
    is executed.

    The \a pathPattern parameter is compared with the \l{QUrl::}{path()} of
    the URL of the incoming request. The \a method parameter is compared with
    the HTTP method of the incoming request. The \a slot parameter is the
    request handler. It can be a member function pointer of \a context, a
    function pointer, non-mutable lambda, or any copyable callable with a
    const call operator. If \a context is provided, the rule remains valid
    as long as context exists. The \a context must share the same thread
    affinity as QHttpServer.

    The \a slot takes as arguments any number of parsed arguments, that are
    extracted from the \a pathPattern by matching the \c "<arg>" placeholders,
    followed by an optional QHttpServerRequest and optional
    QHttpServerResponder. These two classes are called specials.

    The \a slot can return a QHttpServerResponse or a convertible type:

    \code
    QHttpServer server;
    server.route("/test/", this, [] () { return ""; });
    \endcode

    \note This function, \l route(), must not be called from \a slot, so no
    route handlers can register other route handlers.

    Alternatively, if an optional \l{QHttpServerResponder} argument is
    provided, the response has to be written using it and the function
    must return \c void or QFuture<void>. QFuture<void> support was
    added in Qt 6.11. The \l {QHttpServerResponder} is not copyable, and can
    be passed as reference or rvalue reference, except when returning a
    QFuture<void>, in which case it must be passed as a rvalue reference to
    prevent it from going out of scope.

    \code
    server.route("/test2", this,
                 [] (QHttpServerResponder &&responder) {
                    responder.write(QHttpServerResponder::StatusCode::Forbidden);
                 });
    \endcode

    \note If a request was processed by a \a slot accepting \l
    {QHttpServerResponder} as an argument, none of the after request handlers
    (see \l addAfterRequestHandler) will be called.

    Furthermore \l{QHttpServerRequest} can be used as a last argument or,
    if there is a QHttpServerResponder argument, as a second to last
    argument to get detailed information of the request. It can be passed
    as a const reference (only for non-concurrent callbacks) or by value, and
    can be used to access the body of the request:

    \code
    server.route("/test3", QHttpServerRequest::Method::Post, this,
                 [] (QHttpServerRequest request, QHttpServerResponder &&responder) {
                    responder.write(request.body(), "text/plain"_ba);
                 });
    \endcode

    Any placeholder ( \c{"<arg>"} ) in \a pathPattern is automatically
    converted to match the handler's argument types. Supported types
    include integers, floating point numbers, QString, QByteArray, and
    QUrl. The QUrl class can be used as the last parameter to handle the end
    of the \a pathPattern, and by splitting it an arbitrary number of
    arguments can be supported. Custom converters can be added using
    \l{QHttpServerRouter::addConverter()}.

    Each registered type has an associated regex that is used to match and
    convert placeholders in the \a pathPattern. These regex patterns
    are combined to construct a parser for the entire path. The resulting
    parser is then used to verify if the path matches the pattern.
    If parsing succeeds, the corresponding function is called with the
    converted parameters. If parsing fails, the next registered callback is
    attempted. If parsing fails for all callbacks, the missingHandler is
    called.

    In the example below, the value in the request path replacing \c "<arg>"
    is converted to an \c int because the lambda expects an \c int parameter.
    When an HTTP request matches the route, the converted value is passed to
    the lambda's \c page argument:
    \code
    QHttpServer server;
    server.route("/showpage/<arg>", this, [] (int page) { return getPage(page); });
    \endcode

    This function returns, if successful, a pointer to the newly created Rule,
    otherwise a \c nullptr. The pointer can be used to set parameters on any
    custom \l{QHttpServerRouter} class:

    \code
    auto rule = server.route<MyRule>("/test4", this, [] () {return "";});
    rule->setParameter("test");
    \endcode

    Requests are processed sequentially inside the \l {QHttpServer}'s thread
    by default. The request handler may return \c {QFuture<QHttpServerResponse>}
    if concurrent processing is desired:

    \code
    server.route("/feature/<arg>", [] (int ms) {
        return QtConcurrent::run(pool, [ms] () {
            QThread::msleep(ms);
            return QHttpServerResponse("the future is coming");
        });
    });
    \endcode

    The lambda of the QtConcurrent::run() is executed concurrently,
    but all the network communication is executed in the thread the
    \c {QHttpServer} belongs.

    Routes returning futures are only executed fully concurrently on an
    HTTP/2 connection. Earlier versions of HTTP do not support interleaving
    parts of different responses: The responses must come back in full
    in the same order as the requests come in. So even if a route handler
    returns a QFuture<void> the next incoming request on that HTTP/1 connection
    will not be processed until processing of the current one is done.
    Route handlers on different connections can be executed concurrently
    though.

    Making a route handler perform its work in a concurrent run can
    have benefits for all versions of HTTP because the thread that
    \c {QHttpServer} belongs to will be relieved of the work that the
    concurrent run performs.

    QHttpServerRequest is copyable, and must be captured by value when
    returning a future, because the variables passed to \a slot may go
    out of scope before the future is finished.

    \code
    server.route("/test4", QHttpServerRequest::Method::Post, this,
                 [] (QHttpServerRequest request) {
                    return QtConcurrent::run(pool, [request]() {
                        return QHttpServerResponse("text/plain"_ba, request.body());
                    }
                 });
    \endcode

    A \a slot that captures QHttpServerRequest or QHttpServerResponder by reference
    when returning a future causes an assertion with an explanation of why it
    forbidden.

    Not all platforms support moving QHttpServerResponder into a mutable lambda
    that is passed to QConcurrent::run, so the workaround is to move
    QHttpServerResponder into a std::shared_ptr and copying that.

    \code
    server.route("/concurrent-multipart-back/<arg>",
                 [](QString message, QHttpServerResponder &&responder) {
                    return QtConcurrent::run(pool,
                        [=, r = std::make_shared<QHttpServerResponder>(std::move(responder))] {
                            QByteArray ba = message.toUtf8();
                            r->writeBeginChunked("text/plain"_ba);
                            for (ushort i = 1; i < 8; ++i) {
                                r->writeChunk(ba);
                            }
                            r->writeEndChunked(ba);
                        });
                });
    \endcode

    \sa QHttpServerRouter::addRule, addAfterRequestHandler
*/

/*! \fn template <typename Rule = QHttpServerRouterRule, typename Functor> Rule *QHttpServer::route(const QString &pathPattern, const QObject *context, Functor &&slot)

    \overload

    Overload of \l QHttpServer::route to create a Rule for \a pathPattern and
    the method \l{QHttpServerRequest::Method::AnyKnown}. All requests are
    forwarded to \a context and \a slot.
*/

/*! \fn template <typename Rule = QHttpServerRouterRule, typename Functor> Rule *QHttpServer::route(const QString &pathPattern, QHttpServerRequest::Methods method, Functor &&handler)

    \overload

    Overload of \l QHttpServer::route to create a Rule for \a pathPattern and
    \a method. All requests are forwarded to \a handler, which can be a
    function pointer, a non-mutable lambda, or any other copyable callable with
    const call operator. The rule will be valid until the QHttpServer is
    destroyed.
*/

/*! \fn template <typename Rule = QHttpServerRouterRule, typename Functor> Rule *QHttpServer::route(const QString &pathPattern, Functor &&handler)

    \overload

    Overload of \l QHttpServer::route to create a Rule for \a pathPattern and
    \l QHttpServerRequest::Method::AnyKnown. All requests are forwarded to \a
    handler, which can be a function pointer, a non-mutable lambda, or any
    other copyable callable with const call operator. The rule will be valid
    until the QHttpServer is destroyed.
*/

/*!
    Destroys a QHttpServer.
*/
QHttpServer::~QHttpServer()
{
}

/*!
    Returns a pointer to the router object.
*/
QHttpServerRouter *QHttpServer::router()
{
    Q_D(QHttpServer);
    return &d->router;
}

/*!
    Returns a pointer to the constant router object.
*/
const QHttpServerRouter *QHttpServer::router() const
{
    Q_D(const QHttpServer);
    return &d->router;
}

/*! \fn template <typename Functor> void QHttpServer::setMissingHandler(const QObject *context, Functor &&slot)
    Set a handler for unhandled requests.

    All unhandled requests will be forwarded to the \a{context}'s \a slot.

    The \a slot has to implement the signature \c{void (*)(const
    QHttpServerRequest &, QHttpServerResponder &)}. The \a slot can also be a
    function pointer, non-mutable lambda, or any other copyable callable with
    const call operator. In that case the \a context will be a context object.
    The handler will be valid until the context object is destroyed.

    The default handler replies with status \c{404 Not Found}.

    \sa clearMissingHandler
*/

/*!
    \internal
*/
void QHttpServer::setMissingHandlerImpl(const QObject *context, QtPrivate::QSlotObjectBase *handler)
{
    Q_D(QHttpServer);
    auto slot = QtPrivate::SlotObjUniquePtr(handler);
    if (!d->verifyThreadAffinity(context))
        return;
    d->missingHandler = {context, std::move(slot)};
}

/*!
    Resets the handler to the default one that produces replies with
    status \c{404 Not Found}.

    \sa setMissingHandler
*/
void QHttpServer::clearMissingHandler()
{
    Q_D(QHttpServer);
    d->missingHandler.slotObject.reset();
}


/*! \fn template <typename Functor> void QHttpServer::addAfterRequestHandler(const QObject *context, Functor &&slot)
    Register a \a context and \a slot to be called after each request is
    handled.

    The \a slot has to implement the signature \c{void (*)(const QHttpServerRequest &,
    QHttpServerResponse &)}.

    The \a slot can also be a function pointer, non-mutable lambda, or any other
    copyable callable with const call operator. In that case the \a context will
    be a context object and the handler will be valid until the context
    object is destroyed.

    Example:

    \code
    server.addAfterRequestHandler(&server, [] (const QHttpServerRequest &req, QHttpServerResponse &resp) {
        auto h = resp.headers();
        h.append(QHttpHeaders::WellKnownHeader::Cookie, "PollyWants=Cracker");
        resp.setHeaders(std::move(h));
    }
    \endcode

    \note These handlers will only be called for requests processed by route handlers
    which return \l{QHttpServerResponse} or QFuture<QHttpServerResponse>.
*/

/*!
    \internal
*/
void QHttpServer::addAfterRequestHandlerImpl(const QObject *context, QtPrivate::QSlotObjectBase *handler)
{
    Q_D(QHttpServer);
    auto slot = QtPrivate::SlotObjUniquePtr(handler);
    if (!d->verifyThreadAffinity(context))
        return;
    d->afterRequestHandlers.push_back({context, std::move(slot)});
}

/*!
    \internal
*/
void QHttpServer::sendResponse(QHttpServerResponse &&response, const QHttpServerRequest &request,
                               QHttpServerResponder &&responder)
{
    Q_D(QHttpServer);
    for (auto &afterRequestHandler : d->afterRequestHandlers) {
        if (afterRequestHandler.context && afterRequestHandler.slotObject &&
            d->verifyThreadAffinity(afterRequestHandler.context)) {
            void *args[] = { nullptr, const_cast<QHttpServerRequest *>(&request), &response };
            afterRequestHandler.slotObject->call(const_cast<QObject *>(afterRequestHandler.context.data()), args);
        }
    }
    responder.sendResponse(response);
}

#if QT_CONFIG(future)
/*!
    \internal
*/
void QHttpServer::sendResponse(QFuture<QHttpServerResponse> &&response,
                               const QHttpServerRequest &request, QHttpServerResponder &&responder)
{
    response.then(this,
                  [this, request,
                   responder = std::move(responder)](QHttpServerResponse &&response) mutable {
                      sendResponse(std::move(response), request, std::move(responder));
                  });
}
#endif // QT_CONFIG(future)

/*!
    \internal
*/
bool QHttpServer::handleRequest(const QHttpServerRequest &request, QHttpServerResponder &responder)
{
    Q_D(QHttpServer);
    return d->router.handleRequest(request, responder);
}

/*!
    \internal
*/
void QHttpServer::missingHandler(const QHttpServerRequest &request, QHttpServerResponder &responder)
{
    Q_D(QHttpServer);
    return d->callMissingHandler(request, responder);
}

QT_END_NAMESPACE

#include "moc_qhttpserver.cpp"
