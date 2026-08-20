// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QHTTPSERVERPARSER_P_H
#define QHTTPSERVERPARSER_P_H

#include <QtHttpServer/qhttpserverrequest.h>
#include <QtHttpServer/qhttpserverresponder.h>
#include <QtHttpServer/private/qhttpserverrequestfilter_p.h>
#include <QtNetwork/private/qhttpheaderparser_p.h>
#if __has_include(<QtNetwork/private/qbytedatabuffer_p.h>)
#include <QtNetwork/private/qbytedatabuffer_p.h>
#else
#include <QtCore/private/qbytedata_p.h>
#endif

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of QHttpServer. This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.

QT_BEGIN_NAMESPACE

class QHttp2Stream;

class QHttpServerParser
{
    friend class QHttpServerRequest;

public:
    QHttpServerParser(const QHostAddress &remoteAddress, quint16 remotePort,
                      const QHostAddress &localAddress, quint16 localPort,
                      QHttpServerRequestFilter *filter);

    quint16 port = 0;

    enum class State {
        NothingDone,
        ReadingRequestLine,
        ReadingHeader,
        ExpectContinue,
        ReadingData,
        AllDone,
    } state = State::NothingDone;

    QHttpHeaderParser headerParser;

    bool parseRequestLine(QByteArrayView line);
    qsizetype readRequestLine(QIODevice *socket);
    qsizetype readHeader(QIODevice *socket);
    qsizetype sendContinue(QIODevice *socket);
    qsizetype readBodyFast(QIODevice *socket);
    qsizetype readRequestBodyRaw(QIODevice *socket, qsizetype size);
    qsizetype readRequestBodyChunked(QIODevice *socket);
    qsizetype getChunkSize(QIODevice *socket, qsizetype *chunkSize);
    void sendError(QIODevice *socket, QHttpServerResponder::StatusCode error);
    bool isUrlSizeAllowed(QByteArrayView requestLine);

    bool parse(QIODevice *socket);
#if QT_CONFIG(http)
    bool parse(QHttp2Stream *socket);
#endif
    void clear();

    qint64 contentLength() const;
    QByteArray headerField(const QByteArray &name) const
    { return headerParser.combinedHeaderValue(name); }
    QString getClientIpAddressAndPort() const;

    QHostAddress remoteAddress;
    quint16 remotePort;
    QHostAddress localAddress;
    quint16 localPort;
    QHttpServerRequestFilter *filter;
    QUrl url;
    QHttpServerRequest::Method method;
    QHttpHeaders headers;
    QByteArray body;

    bool handling{false};
    qsizetype bodyLength;
    qsizetype contentRead;
    bool chunkedTransferEncoding;
    bool lastChunkRead;
    qsizetype currentChunkRead;
    qsizetype currentChunkSize;
    bool upgrade;
    int majorVersion;
    int minorVersion;

    QByteArray fragment;
    QByteDataBuffer bodyBuffer;
};

QT_END_NAMESPACE

#endif // QHTTPSERVERPARSER_P_H
