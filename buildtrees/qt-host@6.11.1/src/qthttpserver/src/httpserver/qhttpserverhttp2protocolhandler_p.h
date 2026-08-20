// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QHttpServerHttp2ProtocolHandler_H
#define QHttpServerHttp2ProtocolHandler_H

#include <QtHttpServer/qthttpserverglobal.h>
#include <QtHttpServer/qhttpserverrequest.h>
#include <QtHttpServer/private/qhttpserverstream_p.h>
#include <QtHttpServer/private/qhttpserverrequestfilter_p.h>
#include <QtHttpServer/private/qhttpserverresponder_p.h>
#include <QtNetwork/private/hpack_p.h>

#include <QtCore/qbytearray.h>
#include <QtCore/qhash.h>
#include <QtCore/qqueue.h>
#include <QtCore/qelapsedtimer.h>

#include <array>

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of QHttpServer. This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.

QT_REQUIRE_CONFIG(http);
QT_REQUIRE_CONFIG(ssl);

QT_BEGIN_NAMESPACE

class QTcpSocket;
class QAbstractHttpServer;
class QHttp2Connection;
class QHttp2Stream;

struct QHttpServerHttp2Queue
{
    QQueue<QByteArray> data;
    HPack::HttpHeader trailers;
    bool allEnqueued = false;
};

struct QHttpServerHttp2Data
{
    qsizetype numberOfHeaders = 0;
    qsizetype headersSize = 0;
    qsizetype dataSize = 0;
    bool done = false;
};

class QHttpServerHttp2ProtocolHandler : public QHttpServerStream
{
    Q_OBJECT

    friend class QAbstractHttpServerPrivate;

private:
    QHttpServerHttp2ProtocolHandler(QAbstractHttpServer *server,
                                    QIODevice *socket,
                                    QHttpServerRequestFilter *filter);

    void responderDestroyed(quint32 streamId) final;
    void startHandlingRequest() final;
    void socketDisconnected() final;

    void write(const QByteArray &body, const QHttpHeaders &headers,
               QHttpServerResponder::StatusCode status, quint32 streamId) final;
    void write(QHttpServerResponder::StatusCode status, quint32 streamId) final;
    void write(QIODevice *data, const QHttpHeaders &headers,
               QHttpServerResponder::StatusCode status, quint32 streamId) final;
    void writeBeginChunked(const QHttpHeaders &headers,
                           QHttpServerResponder::StatusCode status,
                           quint32 streamId) final;
    void writeChunk(const QByteArray &body, quint32 streamId) final;
    void writeEndChunked(const QByteArray &data,
                         const QHttpHeaders &trailers,
                         quint32 streamId) final;

    void writeHeadersAndStatus(const QHttpHeaders &headers,
                               QHttpServerResponder::StatusCode status,
                               bool endStream,
                               quint32 streamId);

    void checkKeepAliveTimeout();

private slots:
    void onStreamCreated(QHttp2Stream *stream);
    void onStreamClosed(quint32 streamId);
    void onStreamHalfClosed(quint32 streamId);
    void sendToStream(quint32 streamId);
    void onHeadersReceived(quint32 id, const HPack::HttpHeader &headers);
    void onDataReceived(quint32 id, qsizetype size);

private:
    QHttp2Stream * getStream(quint32 streamId) const;
    void enqueueChunk(const QByteArray &body, bool allEnqueued, const QHttpHeaders &trailers,
                      quint32 streamId);

    QAbstractHttpServer *m_server;
    QIODevice *m_socket;
    QTcpSocket *m_tcpSocket;
    QHttpServerRequestFilter *m_filter;
    QHttp2Connection *m_connection;
    QHash<quint32, std::array<QMetaObject::Connection, 4>> m_streamConnections;
    QHash<quint32, QHttpServerHttp2Queue> m_streamQueue;
    QHash<quint32, QHttpServerHttp2Data> m_streamData;
    QHash<quint32, QHttpServerResponderPrivate *> m_responders;
    qint32 m_responderCounter = 0;
    QElapsedTimer lastActiveTimer;
};

QT_END_NAMESPACE

#endif // QHttpServerHttp2ProtocolHandler_H
