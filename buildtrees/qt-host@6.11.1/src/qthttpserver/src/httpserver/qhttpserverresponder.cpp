// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include <QtHttpServer/qhttpserverresponder.h>
#include <QtHttpServer/qhttpserverrequest.h>
#include <QtHttpServer/qhttpserverresponse.h>
#include <private/qhttpserverresponder_p.h>
#include <private/qhttpserverliterals_p.h>
#include <private/qhttpserverrequest_p.h>
#include <private/qhttpserverresponse_p.h>
#include <private/qhttpserverstream_p.h>
#include <QtCore/qjsondocument.h>
#include <QtCore/qloggingcategory.h>
#include <memory>

QT_BEGIN_NAMESPACE

/*!
    \class QHttpServerResponder
    \since 6.4
    \inmodule QtHttpServer
    \brief API for sending replies from an HTTP server.

    Provides functions for writing back to an HTTP client with overloads for
    serializing JSON objects. It also has support for writing HTTP headers and
    status code.
*/

/*!
    \enum QHttpServerResponder::StatusCode

    HTTP status codes

    \value Continue
    \value SwitchingProtocols
    \value Processing

    \value Ok
    \value Created
    \value Accepted
    \value NonAuthoritativeInformation
    \value NoContent
    \value ResetContent
    \value PartialContent
    \value MultiStatus
    \value AlreadyReported
    \value IMUsed

    \value MultipleChoices
    \value MovedPermanently
    \value Found
    \value SeeOther
    \value NotModified
    \value UseProxy

    \value TemporaryRedirect
    \value PermanentRedirect

    \value BadRequest
    \value Unauthorized
    \value PaymentRequired
    \value Forbidden
    \value NotFound
    \value MethodNotAllowed
    \value NotAcceptable
    \value ProxyAuthenticationRequired
    \value RequestTimeout
    \value Conflict
    \value Gone
    \value LengthRequired
    \value PreconditionFailed
    \value PayloadTooLarge
    \value UriTooLong
    \value UnsupportedMediaType
    \value RequestRangeNotSatisfiable
    \value ExpectationFailed
    \value ImATeapot
    \value MisdirectedRequest
    \value UnprocessableEntity
    \value Locked
    \value FailedDependency
    \value UpgradeRequired
    \value PreconditionRequired
    \value TooManyRequests
    \value RequestHeaderFieldsTooLarge
    \value UnavailableForLegalReasons

    \value InternalServerError
    \value NotImplemented
    \value BadGateway
    \value ServiceUnavailable
    \value GatewayTimeout
    \value HttpVersionNotSupported
    \value VariantAlsoNegotiates
    \value InsufficientStorage
    \value LoopDetected
    \value NotExtended
    \value NetworkAuthenticationRequired
    \value NetworkConnectTimeoutError
*/

/*!
    \internal
*/
QHttpServerResponderPrivate::QHttpServerResponderPrivate(QHttpServerStream *stream) : stream(stream)
{
    Q_ASSERT(stream);
    QMetaObject::invokeMethod(stream, &QHttpServerStream::startHandlingRequest);
}

/*!
    \internal
*/
QHttpServerResponderPrivate::~QHttpServerResponderPrivate()
{
    Q_ASSERT(stream);
    QMetaObject::invokeMethod(stream, &QHttpServerStream::responderDestroyed, m_streamId);
}

/*!
    \internal
*/
void QHttpServerResponderPrivate::cancel()
{
    canceled = true;
}

/*!
    \internal
*/
void QHttpServerResponderPrivate::write(QHttpServerResponder::StatusCode status)
{
    if (canceled)
        return;
    Q_ASSERT(stream);
    QMetaObject::invokeMethod(
            stream, qOverload<QHttpServerResponder::StatusCode, quint32>(&QHttpServerStream::write),
            status, m_streamId);
}

/*!
    \internal
*/
void QHttpServerResponderPrivate::write(const QByteArray &body, const QHttpHeaders &headers,
                                        QHttpServerResponder::StatusCode status)
{
    if (canceled)
        return;
    Q_ASSERT(stream);
    QMetaObject::invokeMethod(
            stream,
            qOverload<const QByteArray &, const QHttpHeaders &, QHttpServerResponder::StatusCode,
                      quint32>(&QHttpServerStream::write),
            body, headers, status, m_streamId);
}

/*!
    \internal
*/
void QHttpServerResponderPrivate::write(QIODevice *data, const QHttpHeaders &headers,
                                        QHttpServerResponder::StatusCode status)
{
    if (canceled)
        return;
    Q_ASSERT(stream);
    Q_ASSERT(data);
    data->setParent(nullptr);
    data->moveToThread(stream->thread());
    QMetaObject::invokeMethod(
            stream,
            qOverload<QIODevice *, const QHttpHeaders &, QHttpServerResponder::StatusCode, quint32>(
                    &QHttpServerStream::write),
            data, headers, status, m_streamId);
}

/*!
    \internal
*/
void QHttpServerResponderPrivate::writeBeginChunked(const QHttpHeaders &headers,
                                                    QHttpServerResponder::StatusCode status)
{
    if (canceled)
        return;
    Q_ASSERT(stream);
    QMetaObject::invokeMethod(stream, &QHttpServerStream::writeBeginChunked, headers, status,
                              m_streamId);
}

/*!
    \internal
*/
void QHttpServerResponderPrivate::writeChunk(const QByteArray &data)
{
    if (canceled)
        return;
    Q_ASSERT(stream);
    QMetaObject::invokeMethod(stream, &QHttpServerStream::writeChunk, data, m_streamId);
}

/*!
    \internal
*/
void QHttpServerResponderPrivate::writeEndChunked(const QByteArray &data,
                                                  const QHttpHeaders &trailers)
{
    if (canceled)
        return;
    Q_ASSERT(stream);
    QMetaObject::invokeMethod(stream, &QHttpServerStream::writeEndChunked, data, trailers,
                              m_streamId);
}

/*!
    Constructs a QHttpServerResponder instance using a \a stream
    to output the response to.
*/
QHttpServerResponder::QHttpServerResponder(QHttpServerStream *stream)
    : d_ptr(new QHttpServerResponderPrivate(stream))
{
    Q_ASSERT(stream);
}

/*!
    \fn QHttpServerResponder::QHttpServerResponder(QHttpServerResponder &&other)

    Move-constructs a QHttpServerResponder instance, making it point
    at the same object that \a other was pointing to.
*/

/*!
    Destroys a QHttpServerResponder.
*/
QHttpServerResponder::~QHttpServerResponder()
{
    delete d_ptr;
};

/*!
    \fn void QHttpServerResponder::swap(QHttpServerResponder &other) noexcept

    Swaps QHttpServerResponder \a other with this QHttpServerResponder.
    This operation is very fast and never fails.

    \since 6.8
*/

/*!
    Answers a request with an HTTP status code \a status and
    HTTP headers \a headers. The I/O device \a data provides the body
    of the response. If \a data is sequential, the body of the
    message is sent in chunks: otherwise, the function assumes all
    the content is available and sends it all at once but the read
    is done in chunks.

    \note This function takes the ownership of \a data.
*/
void QHttpServerResponder::write(QIODevice *data,
                                 const QHttpHeaders &headers,
                                 StatusCode status)
{
    Q_D(QHttpServerResponder);
    d->write(data, headers, status);
}

/*!
    Answers a request with an HTTP status code \a status and a
    MIME type \a mimeType. The I/O device \a data provides the body
    of the response. If \a data is sequential, the body of the
    message is sent in chunks: otherwise, the function assumes all
    the content is available and sends it all at once but the read
    is done in chunks.

    \note This function takes the ownership of \a data.
*/
void QHttpServerResponder::write(QIODevice *data,
                                 const QByteArray &mimeType,
                                 StatusCode status)
{
    QHttpHeaders headers;
    headers.append(QHttpHeaders::WellKnownHeader::ContentType, mimeType);
    write(data, headers, status);
}

/*!
    Answers a request with an HTTP status code \a status, JSON
    document \a document and HTTP headers \a headers.

    Note: This function sets HTTP Content-Type header as
    \c{"application/json"}.
*/
void QHttpServerResponder::write(const QJsonDocument &document,
                                 const QHttpHeaders &headers,
                                 StatusCode status)
{
    Q_D(QHttpServerResponder);
    const QByteArray &json = document.toJson();
    QHttpHeaders allHeaders(headers);
    allHeaders.append(QHttpHeaders::WellKnownHeader::ContentType,
                      QHttpServerLiterals::contentTypeJson());
    allHeaders.append(QHttpHeaders::WellKnownHeader::ContentLength,
                      QByteArray::number(json.size()));
    d->write(document.toJson(), allHeaders, status);
}

/*!
    Answers a request with an HTTP status code \a status, and JSON
    document \a document.

    Note: This function sets HTTP Content-Type header as
    \c{"application/json"}.
*/
void QHttpServerResponder::write(const QJsonDocument &document,
                                 StatusCode status)
{
    write(document, {}, status);
}

/*!
    Answers a request with an HTTP status code \a status,
    HTTP Headers \a headers and a body \a data.

    Note: This function sets HTTP Content-Length header.
*/
void QHttpServerResponder::write(const QByteArray &data,
                                 const QHttpHeaders &headers,
                                 StatusCode status)
{
    Q_D(QHttpServerResponder);
    QHttpHeaders allHeaders(headers);
    allHeaders.append(QHttpHeaders::WellKnownHeader::ContentLength,
                      QByteArray::number(data.size()));
    d->write(data, allHeaders, status);
}

/*!
    Answers a request with an HTTP status code \a status, a
    MIME type \a mimeType and a body \a data.
*/
void QHttpServerResponder::write(const QByteArray &data,
                                 const QByteArray &mimeType,
                                 StatusCode status)
{
    QHttpHeaders headers;
    headers.append(QHttpHeaders::WellKnownHeader::ContentType, mimeType);
    write(data, headers, status);
}

/*!
    Answers a request with an HTTP status code \a status.

    Note: This function sets HTTP Content-Type header as
    \c{"application/x-empty"}.
*/
void QHttpServerResponder::write(StatusCode status)
{
    write(QByteArray(), QHttpServerLiterals::contentTypeXEmpty(), status);
}

/*!
    Answers a request with an HTTP status code \a status and
    HTTP Headers \a headers.
*/
void QHttpServerResponder::write(const QHttpHeaders &headers, StatusCode status)
{
    write(QByteArray(), headers, status);
}

/*!
    Sends a HTTP \a response to the client.

    \since 6.5
*/
void QHttpServerResponder::sendResponse(const QHttpServerResponse &response)
{
    Q_D(QHttpServerResponder);
    const auto &r = response.d_ptr;
    QHttpHeaders allHeaders(r->headers);
    allHeaders.append(QHttpHeaders::WellKnownHeader::ContentLength,
                      QByteArray::number(r->data.size()));

    d->write(r->data, allHeaders, r->statusCode);
}

/*!
    Start sending chunks of data with \a headers and and the status
    code \a status. This call must be followed up with an arbitrary
    number of repeated \c writeChunk calls and and a single call to
    \c writeEndChunked.

    \since 6.8
    \sa writeChunk, writeEndChunked
*/
void QHttpServerResponder::writeBeginChunked(const QHttpHeaders &headers, StatusCode status)
{
    Q_D(QHttpServerResponder);
    d->writeBeginChunked(headers, status);
}

/*!
    Start sending chunks of data with the mime type \a mimeType and
    and the given status code \a status. This call must be followed
    up with an arbitrary number of repeated \c writeChunk calls and
    and a single call to \c writeEndChunked.

    \since 6.8
    \sa writeChunk, writeEndChunked
*/
void QHttpServerResponder::writeBeginChunked(const QByteArray &mimeType, StatusCode status)
{
    QHttpHeaders headers;
    headers.append(QHttpHeaders::WellKnownHeader::ContentType, mimeType);
    writeBeginChunked(headers, status);
}

/*!
    Start sending chunks of data with \a headers and and the given
    status code \a status. This call must be followed up with an
    arbitrary number of repeated \c writeChunk calls and and a single
    call to \c writeEndChunked with the same trailers given in
    \a trailers.

    \since 6.8
    \sa writeChunk, writeEndChunked
*/
void QHttpServerResponder::writeBeginChunked(const QHttpHeaders &headers,
                                             QList<QHttpHeaders::WellKnownHeader> trailers,
                                             StatusCode status)
{
    QHttpHeaders allHeaders(headers);
    QByteArray trailerList;
    for (qsizetype i = 0; i < trailers.size(); ++i) {
        if (i != 0)
            trailerList.append(", ");

        trailerList.append(QHttpHeaders::wellKnownHeaderName(trailers[i]).toByteArray());
    }
    allHeaders.append(QHttpHeaders::WellKnownHeader::Trailer, trailerList);
    writeBeginChunked(allHeaders, status);
}

/*!
    Write \a data back to the client. To be called when data is
    available to write. This can be called multiple times, but before
    calling this \c writeBeginChunked must called, and afterwards
    \c writeEndChunked must be called.

    \sa writeBeginChunked, writeEndChunked
    \since 6.8
*/
void QHttpServerResponder::writeChunk(const QByteArray &data)
{
    Q_D(QHttpServerResponder);
    d->writeChunk(data);
}

/*!
    Write \a data back to the client with the \a trailers
    announced in \c writeBeginChunked.

    \since 6.8
    \sa writeBeginChunked, writeChunk
*/
void QHttpServerResponder::writeEndChunked(const QByteArray &data, const QHttpHeaders &trailers)
{
    Q_D(QHttpServerResponder);
    d->writeEndChunked(data, trailers);
}

/*!
    Write \a data back to the client. Must be preceded
    by a call to \c writeBeginChunked.

    \since 6.8
    \sa writeBeginChunked, writeChunk
*/
void QHttpServerResponder::writeEndChunked(const QByteArray &data)
{
    writeEndChunked(data, {});
}

/*!
    Returns true if the client has canceled the response.
    Used to make it possible to exit early in handler
    when the response is canceled.

    \since 6.11
*/
bool QHttpServerResponder::isResponseCanceled() const
{
    Q_D(const QHttpServerResponder);
    return d->canceled;
}

QT_END_NAMESPACE
