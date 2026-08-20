// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:critical reason:network-protocol

#include "qhttpserverrequest_p.h"
#include "qhttpserverparser_p.h"

#include <QtCore/qloggingcategory.h>
#include <QtNetwork/qhttpheaders.h>
#if QT_CONFIG(http)
#include <QtNetwork/private/qhttp2connection_p.h>
#endif
#include <limits>

QT_BEGIN_NAMESPACE

Q_STATIC_LOGGING_CATEGORY(lcHttpServerParser, "qt.httpserver.parser")

using namespace Qt::StringLiterals;

namespace {

QHttpServerRequest::Method parseRequestMethod(QByteArrayView str)
{
    if (str == "GET")
        return QHttpServerRequest::Method::Get;
    else if (str == "PUT")
        return QHttpServerRequest::Method::Put;
    else if (str == "DELETE")
        return QHttpServerRequest::Method::Delete;
    else if (str == "POST")
        return QHttpServerRequest::Method::Post;
    else if (str == "HEAD")
        return QHttpServerRequest::Method::Head;
    else if (str == "OPTIONS")
        return QHttpServerRequest::Method::Options;
    else if (str == "PATCH")
        return QHttpServerRequest::Method::Patch;
    else if (str == "CONNECT")
        return QHttpServerRequest::Method::Connect;
    else
        return QHttpServerRequest::Method::Unknown;
}

const char* getErrorString(QHttpServerResponder::StatusCode code)
{
    switch (code) {
    case QHttpServerResponder::StatusCode::PayloadTooLarge:
        return "Request Entity Too Large";
    case QHttpServerResponder::StatusCode::UriTooLong:
        return "Request-URI Too Long";
    case QHttpServerResponder::StatusCode::RequestHeaderFieldsTooLarge:
        return "Request Header Fields Too Large";
    default:
        return "Bad Request";
    };
}

} // anonymous namespace

/*!
    \internal
*/
bool QHttpServerParser::parseRequestLine(QByteArrayView line)
{
    // Request-Line   = Method SP Request-URI SP HTTP-Version CRLF
    auto i = line.indexOf(' ');
    if (i == -1)
        return false;
    const auto requestMethod = line.first(i);
    i++;

    while (i < line.size() && line[i] == ' ')
        i++;

    auto j = line.indexOf(' ', i);
    if (j == -1)
        return false;

    const auto requestUrl = line.sliced(i, j - i);
    i = j + 1;

    while (i < line.size() && line[i] == ' ')
        i++;

    if (i >= line.size())
        return false;

    j = line.indexOf(' ', i);

    const auto protocol = j == -1 ? line.sliced(i) : line.sliced(i, j - i);
    if (protocol.size() != 8 || !protocol.startsWith("HTTP"))
        return false;

    headerParser.setMajorVersion(protocol[5] - '0');
    headerParser.setMinorVersion(protocol[7] - '0');
    majorVersion = protocol[5] - '0';
    minorVersion = protocol[7] - '0';

    method = parseRequestMethod(requestMethod);
    url = QUrl::fromEncoded(requestUrl.toByteArray());
    return true;
}

/*!
    \internal
*/
qsizetype QHttpServerParser::readRequestLine(QIODevice *socket)
{
    if (fragment.isEmpty()) {
        // reserve bytes for the request line. This is better than always append() which reallocs
        // the byte array
        fragment.reserve(32);
    }

    qsizetype bytes = 0;
    char c;
    qsizetype haveRead = 0;

    do {
        haveRead = socket->read(&c, 1);
        if (haveRead == -1)
            return -1; // unexpected EOF
        else if (haveRead == 0)
            break; // read more later
        else if (haveRead == 1 && fragment.size() == 0
                 && (c == '\v' || c == '\n' || c == '\r' || c == ' ' || c == '\t'))
            continue; // Ignore all whitespace that was trailing from a previous request on that
                      // socket

        bytes++;

        // allow both CRLF & LF (only) line endings
        if (c == '\n') {
            // remove the CR at the end
            if (fragment.endsWith('\r')) {
                fragment.truncate(fragment.size() - 1);
            }
            bool ok = parseRequestLine(fragment);
            state = State::ReadingHeader;
            fragment.clear();
            if (!ok) {
                return -1;
            }
            break;
        } else {
            fragment.append(c);
            if (!isUrlSizeAllowed(fragment.first(bytes))) {
                fragment.clear();
                sendError(socket, QHttpServerResponder::StatusCode::UriTooLong);
                qCDebug(lcHttpServerParser) << "URI too long at" << bytes << "bytes from client"
                                            << getClientIpAddressAndPort();
                return -1;
            }
        }
    } while (haveRead == 1);

    return bytes;
}

bool QHttpServerParser::isUrlSizeAllowed(QByteArrayView requestLine)
{
    if (filter->isUrlSizeAllowed(requestLine.size()))
        return true; // Succeed if entire request line is not too long

    // Request-Line   = Method SP Request-URI SP HTTP-Version CRLF
    constexpr qsizetype shortestCaseExtraCharacters = 3 + 1 + 1 + 8 + 2;
    if (!filter->isUrlSizeAllowed(requestLine.size() - shortestCaseExtraCharacters))
        return false;

    qsizetype firstSpace = requestLine.indexOf(' ');
    if (firstSpace < 0)
        return false; // First space not read yet

    qsizetype uriStart = firstSpace;
    while (uriStart < requestLine.size() && requestLine[uriStart] == ' ')
        uriStart++;

    qsizetype secondSpace = requestLine.indexOf(' ', uriStart);
    if (secondSpace < 0)
        secondSpace = requestLine.size(); // Second space not read yet

    return filter->isUrlSizeAllowed(secondSpace - uriStart);
}

void QHttpServerParser::sendError(QIODevice *socket, QHttpServerResponder::StatusCode error)
{
    QByteArray buffer;
    buffer.append("HTTP/1.1 ");
    buffer.append(QByteArray::number(int(error)));
    buffer.append(" ");
    buffer.append(getErrorString(error));
    buffer.append("\r\nConnection: close\r\n\r\n");
    socket->write(buffer);
}

/*!
    \internal
*/
qint64 QHttpServerParser::contentLength() const
{
    bool ok = false;
    QByteArray value = headerParser.firstHeaderField("content-length");
    qint64 length = value.toULongLong(&ok);
    if (ok)
        return length;
    return -1; // the header field is not set
}

/*!
    \internal
*/
qsizetype QHttpServerParser::readHeader(QIODevice *socket)
{
    if (fragment.isEmpty()) {
        // according to
        // https://maqentaer.com/devopera-static-backup/http/dev.opera.com/articles/view/mama-http-headers/index.html
        // the average size of the header block is 381 bytes. reserve bytes. This is better than
        // always append() which reallocs the byte array.
        fragment.reserve(512);
    }

    qint64 bytes = 0;
    char c = 0;
    bool allHeaders = false;
    qint64 haveRead = 0;
    do {
        haveRead = socket->read(&c, 1);
        if (haveRead == 0) {
            // read more later
            break;
        } else if (haveRead == -1) {
            // connection broke down
            return -1;
        } else {
            fragment.append(c);
            bytes++;
            if (!filter->isTotalHeaderSizeAllowed(bytes)) {
                sendError(socket, QHttpServerResponder::StatusCode::RequestHeaderFieldsTooLarge);
                qCDebug(lcHttpServerParser) << "Total header size too large, currently at" << bytes
                                            << "bytes from client" << getClientIpAddressAndPort();
                return -1;
            }

            if (c == '\n') {
                // check for possible header endings. As per HTTP rfc,
                // the header endings will be marked by CRLFCRLF. But
                // we will allow CRLFCRLF, CRLFLF, LFCRLF, LFLF
                if (fragment.endsWith("\n\r\n") || fragment.endsWith("\n\n"))
                    allHeaders = true;

                // there is another case: We have no headers. Then the fragment equals just the line
                // ending
                if ((fragment.size() == 2 && fragment.endsWith("\r\n"))
                    || (fragment.size() == 1 && fragment.endsWith("\n")))
                    allHeaders = true;
            }
        }
    } while (!allHeaders && haveRead > 0);

    // we received all headers now parse them
    if (allHeaders) {
        bool parseSuccess = headerParser.parseHeaders(fragment);
        if (!parseSuccess) {
            sendError(socket, QHttpServerResponder::StatusCode::RequestHeaderFieldsTooLarge);
            qCDebug(lcHttpServerParser)
                    << "Error parsing headers from client" << getClientIpAddressAndPort();
            return -1;
        }
        headers = headerParser.headers();
        fragment.clear(); // next fragment

        if (!filter->isNumberOfHeaderFieldsAllowed(headers.size())) {
            sendError(socket, QHttpServerResponder::StatusCode::RequestHeaderFieldsTooLarge);
            qCDebug(lcHttpServerParser) << "Number of header fields" << headers.size()
                                        << "too large from client" << getClientIpAddressAndPort();
            return -1;
        }

        for (qsizetype i = 0; i < headers.size(); ++i) {
            qsizetype headerFieldSize = headers.nameAt(i).size() + 1 + headers.valueAt(i).size();
            if (!filter->isHeaderFieldSizeAllowed(headerFieldSize)) {
                sendError(socket, QHttpServerResponder::StatusCode::RequestHeaderFieldsTooLarge);
                QByteArrayView name(headers.nameAt(i).data(), qMin(50, headers.nameAt(i).size()));
                qCDebug(lcHttpServerParser)
                        << "Header field" << name << "too large at" << headerFieldSize
                        << "bytes from client" << getClientIpAddressAndPort();
                return -1;
            }
        }

        auto hostUrl = QString::fromUtf8(headerField("host"));
        if (!hostUrl.isEmpty())
            url.setAuthority(hostUrl);

        if (url.scheme().isEmpty()) {
#if QT_CONFIG(ssl)
            auto sslSocket = qobject_cast<QSslSocket *>(socket);
            url.setScheme(sslSocket && sslSocket->isEncrypted() ? u"https"_s : u"http"_s);
#else
            url.setScheme(u"http"_s);
#endif
        }

        if (url.host().isEmpty())
            url.setHost(u"127.0.0.1"_s);

        if (url.port() == -1)
            url.setPort(port);

        bodyLength = contentLength(); // cache the length
        if (bodyLength > 0 && !filter->isBodySizeAllowed(bodyLength)) {
            sendError(socket, QHttpServerResponder::StatusCode::PayloadTooLarge);
            qCDebug(lcHttpServerParser) << "Body size too large at" << bodyLength
                                        << "bytes from client" << getClientIpAddressAndPort();
            return -1;
        }
        // cache isChunked() since it is called often
        // FIXME: the RFC says that anything but "identity" should be interpreted as chunked (4.4
        // [2])
        chunkedTransferEncoding = headerField("transfer-encoding").toLower().contains("chunked");

        QByteArray connectionHeaderField = headerField("connection");
        upgrade = connectionHeaderField.toLower().contains("upgrade");

        if (chunkedTransferEncoding || bodyLength > 0) {
            if (headerField("expect").compare("100-continue", Qt::CaseInsensitive) == 0)
                state = State::ExpectContinue;
            else
                state = State::ReadingData;
         } else {
            state = State::AllDone;
         }
    }
    return bytes;
}

/*!
    \internal
*/
qsizetype QHttpServerParser::sendContinue(QIODevice *socket)
{
    qsizetype ret = socket->write("HTTP/1.1 100 Continue\r\n\r\n");
    state = State::ReadingData;
    return ret;
}

/*!
    \internal
*/
QHttpServerParser::QHttpServerParser(const QHostAddress &remoteAddress, quint16 remotePort,
                                     const QHostAddress &localAddress, quint16 localPort,
                                     QHttpServerRequestFilter *filter)
    : remoteAddress(remoteAddress),
      remotePort(remotePort),
      localAddress(localAddress),
      localPort(localPort),
      filter(filter)
{
    clear();
    headerParser.setMaxHeaderFieldSize(std::numeric_limits<qsizetype>::max());
    headerParser.setMaxTotalHeaderSize(std::numeric_limits<qsizetype>::max());
    headerParser.setMaxHeaderFields(std::numeric_limits<qsizetype>::max());
}

/*!
    \internal
*/
bool QHttpServerParser::parse(QIODevice *socket)
{
    qsizetype read;

    do {
        switch (state) {
        case State::AllDone:
            clear();
            [[fallthrough]];
        case State::NothingDone:
            state = State::ReadingRequestLine;
            [[fallthrough]];
        case State::ReadingRequestLine:
            read = readRequestLine(socket);
            continue;
        case State::ReadingHeader:
            read = readHeader(socket);
            continue;
        case State::ExpectContinue:
            read = sendContinue(socket);
            continue;
        case State::ReadingData:
            if (chunkedTransferEncoding)
                read = readRequestBodyChunked(socket);
            else
                read = readBodyFast(socket);

            if (state == State::AllDone) {
                body = bodyBuffer.readAll();
                bodyBuffer.clear();
            }

            continue;
        }
        Q_UNREACHABLE(); // fixes GCC -Wmaybe-uninitialized warning on `read`
    } while (state != State::AllDone && read > 0);

    return read != -1;
}

#if QT_CONFIG(http)
bool QHttpServerParser::parse(QHttp2Stream *socket)
{
    clear();

    majorVersion = 2;
    minorVersion = 0;

    for (const auto &pair : socket->receivedHeaders()) {
        if (pair.name == ":method") {
            method = parseRequestMethod(pair.value);
        } else if (pair.name == ":scheme") {
            url.setScheme(QLatin1StringView(pair.value));
        } else if (pair.name == ":authority") {
            url.setAuthority(QLatin1StringView(pair.value));
        } else if (pair.name == ":path") {
            auto path = QUrl::fromEncoded(pair.value);
            url.setPath(path.path());
            url.setQuery(path.query());
        } else {
            headerParser.appendHeaderField(pair.name, pair.value);
        }
    }
    headers = headerParser.headers();

    if (url.scheme().isEmpty())
        url.setScheme(u"https"_s);

    if (url.host().isEmpty())
        url.setHost(u"127.0.0.1"_s);

    if (url.port() == -1)
        url.setPort(port);

    bodyLength = contentLength(); // cache the length

    body = socket->downloadBuffer().readAll();

    return true;
}
#endif

/*!
    \internal
*/
void QHttpServerParser::clear()
{
    headerParser.clear();
    bodyLength = -1;
    contentRead = 0;
    chunkedTransferEncoding = false;
    lastChunkRead = false;
    currentChunkRead = 0;
    currentChunkSize = 0;
    upgrade = false;
    majorVersion = 0;
    minorVersion = 0;

    fragment.clear();
    bodyBuffer.clear();

    url.clear();
    method = QHttpServerRequest::Method::Unknown;
    headers.clear();
    body.clear();
}

// The body reading functions were mostly copied from QHttpNetworkReplyPrivate

/*!
    \internal
*/
// note this function can only be used for non-chunked, non-compressed with
// known content length
qsizetype QHttpServerParser::readBodyFast(QIODevice *socket)
{

    qsizetype toBeRead = qMin(socket->bytesAvailable(), bodyLength - contentRead);
    if (!toBeRead)
        return 0;

    QByteArray bd;
    bd.resize(toBeRead);
    qsizetype haveRead = socket->read(bd.data(), toBeRead);
    if (haveRead == -1) {
        bd.clear();
        return 0; // ### error checking here;
    }
    bd.resize(haveRead);

    bodyBuffer.append(bd);

    contentRead += haveRead;

    if (contentRead == bodyLength)
        state = State::AllDone;

    return haveRead;
}

/*!
    \internal
*/
qsizetype QHttpServerParser::readRequestBodyRaw(QIODevice *socket, qsizetype size)
{
    // FIXME get rid of this function and just use readBodyFast and give it socket->bytesAvailable()
    qsizetype bytes = 0;
    Q_ASSERT(socket);

    int toBeRead = qMin<qsizetype>(128 * 1024, qMin<qint64>(size, socket->bytesAvailable()));

    while (toBeRead > 0) {
        QByteArray byteData;
        byteData.resize(toBeRead);
        qsizetype haveRead = socket->read(byteData.data(), byteData.size());
        if (haveRead <= 0) {
            // ### error checking here
            byteData.clear();
            return bytes;
        }

        byteData.resize(haveRead);
        bodyBuffer.append(byteData);
        bytes += haveRead;
        size -= haveRead;

        toBeRead = qMin<qsizetype>(128 * 1024, qMin<qsizetype>(size, socket->bytesAvailable()));
    }
    return bytes;
}

/*!
    \internal
*/
qsizetype QHttpServerParser::readRequestBodyChunked(QIODevice *socket)
{
    qsizetype bytes = 0;
    while (socket->bytesAvailable()) {
        if (!lastChunkRead && currentChunkRead >= currentChunkSize) {
            // For the first chunk and when we're done with a chunk
            currentChunkSize = 0;
            currentChunkRead = 0;
            if (bytes) {
                // After a chunk
                char crlf[2];
                // read the "\r\n" after the chunk
                qsizetype haveRead = socket->read(crlf, 2);
                // FIXME: This code is slightly broken and not optimal. What if the 2 bytes are not
                // available yet?! For nice reasons (the toLong in getChunkSize accepting \n at the
                // beginning it right now still works, but we should definitely fix this.

                if (haveRead != 2)
                    return bytes;
                bytes += haveRead;
            }
            // Note that chunk size gets stored in currentChunkSize, what is returned is the bytes
            // read
            bytes += getChunkSize(socket, &currentChunkSize);
            if (currentChunkSize == -1)
                break;
            if (!filter->isBodySizeAllowed(bodyBuffer.byteAmount() + currentChunkSize)) {
                sendError(socket, QHttpServerResponder::StatusCode::PayloadTooLarge);
                socket->skip(socket->bytesAvailable());
                qCDebug(lcHttpServerParser) << "Body size too large currently at"
                                            << (bodyBuffer.byteAmount() + currentChunkSize)
                                            << "from client" << getClientIpAddressAndPort();
                return -1;
            }
        }
        // if the chunk size is 0, end of the stream
        if (currentChunkSize == 0 || lastChunkRead) {
            lastChunkRead = true;
            // try to read the "\r\n" after the chunk
            char crlf[2];
            qsizetype haveRead = socket->read(crlf, 2);
            if (haveRead > 0)
                bytes += haveRead;

            if ((haveRead == 2 && crlf[0] == '\r' && crlf[1] == '\n')
                || (haveRead == 1 && crlf[0] == '\n')) {
                state = State::AllDone;
            } else if (haveRead == 1 && crlf[0] == '\r') {
                break; // Still waiting for the last \n
            } else if (haveRead > 0) {
                // If we read something else then CRLF, we need to close the channel.
                // FIXME forceConnectionCloseEnabled = true;
                state = State::AllDone;
            }
            break;
        }

        // otherwise, try to begin reading this chunk / to read what is missing for this chunk
        qsizetype haveRead = readRequestBodyRaw(socket, currentChunkSize - currentChunkRead);
        currentChunkRead += haveRead;
        bytes += haveRead;

        // ### error checking here
    }
    return bytes;
}

/*!
    \internal
*/
qsizetype QHttpServerParser::getChunkSize(QIODevice *socket, qsizetype *chunkSize)
{
    qsizetype bytes = 0;
    char crlf[2];
    *chunkSize = -1;

    int bytesAvailable = socket->bytesAvailable();
    // FIXME rewrite to permanent loop without bytesAvailable
    while (bytesAvailable > bytes) {
        qsizetype sniffedBytes = socket->peek(crlf, 2);
        int fragmentSize = fragment.size();

        // check the next two bytes for a "\r\n", skip blank lines
        if ((fragmentSize && sniffedBytes == 2 && crlf[0] == '\r' && crlf[1] == '\n')
            || (fragmentSize > 1 && fragment.endsWith('\r') && crlf[0] == '\n')) {
            bytes += socket->read(crlf, 1); // read the \r or \n
            if (crlf[0] == '\r')
                bytes += socket->read(crlf, 1); // read the \n
            bool ok = false;
            // ignore the chunk-extension
            fragment = fragment.mid(0, fragment.indexOf(';')).trimmed();
            *chunkSize = fragment.toLong(&ok, 16);
            fragment.clear();
            break; // size done
        } else {
            // read the fragment to the buffer
            char c = 0;
            qsizetype haveRead = socket->read(&c, 1);
            if (haveRead < 0)
                return -1;

            bytes += haveRead;
            fragment.append(c);
        }
    }

    return bytes;
}

/*!
    \internal
*/
QString QHttpServerParser::getClientIpAddressAndPort() const
{
    using namespace Qt::StringLiterals;
    QString result;
    if (remoteAddress.protocol() != QAbstractSocket::IPv4Protocol)
        result.append(u"["_s);
    result.append(remoteAddress.toString());
    if (remoteAddress.protocol() != QAbstractSocket::IPv4Protocol)
        result.append(u"]"_s);
    result.append(u":"_s);
    result.append(QString::number(remotePort));
    return result;
}

QT_END_NAMESPACE
