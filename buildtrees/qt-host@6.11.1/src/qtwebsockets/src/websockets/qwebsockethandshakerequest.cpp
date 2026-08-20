// Copyright (C) 2016 Kurt Pattyn <pattyn.kurt@gmail.com>.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:critical reason:network-protocol

#include "qwebsockethandshakerequest_p.h"
#include "qwebsocketprotocol.h"
#include "qwebsocketprotocol_p.h"

#include <QtCore/QString>
#include <QtCore/QMap>
#include <QtCore/QTextStream>
#include <QtCore/QUrl>
#include <QtCore/QList>
#include <QtCore/QStringList>
#include <functional>   //for std::greater

QT_BEGIN_NAMESPACE

/*!
    \brief Constructs a new QWebSocketHandshakeRequest given \a port and \a isSecure
    \internal
 */
QWebSocketHandshakeRequest::QWebSocketHandshakeRequest(int port, bool isSecure) :
    m_port(port),
    m_isSecure(isSecure),
    m_isValid(false),
    m_parser(),
    m_versions(),
    m_key(),
    m_origin(),
    m_protocols(),
    m_extensions(),
    m_requestUrl()
{
}

/*!
    \internal
 */
QWebSocketHandshakeRequest::~QWebSocketHandshakeRequest()
{
}

/*!
    \brief Clears the request
    \internal
 */
void QWebSocketHandshakeRequest::clear()
{
    m_isValid = false;
    m_parser.clear();
    m_versions.clear();
    m_key.clear();
    m_origin.clear();
    m_protocols.clear();
    m_extensions.clear();
    m_requestUrl.clear();
}

/*!
    \internal
 */
int QWebSocketHandshakeRequest::port() const
{
    return m_requestUrl.port(m_port);
}

/*!
    \internal
 */
bool QWebSocketHandshakeRequest::isSecure() const
{
    return m_isSecure;
}

/*!
    \internal
 */
bool QWebSocketHandshakeRequest::isValid() const
{
    return m_isValid;
}

/*!
    \internal
 */
QHttpHeaders QWebSocketHandshakeRequest::headers() const
{
    return m_parser.headers();
}

/*!
    \internal
 */
bool QWebSocketHandshakeRequest::hasHeader(const QByteArray &name) const
{
    return !m_parser.firstHeaderField(name).isEmpty();
}

/*!
    \internal
 */
QList<QWebSocketProtocol::Version> QWebSocketHandshakeRequest::versions() const
{
    return m_versions;
}

/*!
    \internal
 */
QString QWebSocketHandshakeRequest::resourceName() const
{
    return m_requestUrl.path();
}

/*!
    \internal
 */
QString QWebSocketHandshakeRequest::key() const
{
    return m_key;
}

/*!
    \internal
 */
QString QWebSocketHandshakeRequest::host() const
{
    return m_requestUrl.host();
}

/*!
    \internal
 */
QString QWebSocketHandshakeRequest::origin() const
{
    return m_origin;
}

/*!
    \internal
 */
QList<QString> QWebSocketHandshakeRequest::protocols() const
{
    return m_protocols;
}

/*!
    \internal
 */
QList<QString> QWebSocketHandshakeRequest::extensions() const
{
    return m_extensions;
}

/*!
    \internal
 */
QUrl QWebSocketHandshakeRequest::requestUrl() const
{
    return m_requestUrl;
}

/*!
    Reads a line of text from the given QByteArrayView (terminated by CR/LF),
    and removes the read bytes from the QByteArrayView.
    If an empty line was detected, an empty string is returned.
    When an error occurs, a null string is returned.
    \internal
 */
static QByteArrayView readLine(QByteArrayView &header, int maxHeaderLineLength)
{
    qsizetype length = qMin(maxHeaderLineLength, header.size());
    QByteArrayView line = header.first(length);
    qsizetype end = line.indexOf('\n');
    if (end != -1) {
        line = line.first(end);
        if (line.endsWith('\r'))
            line.chop(1);
        header = header.sliced(end + 1);
        return line;
    }
    return QByteArrayView();
}

/*!
    \internal
 */
static void appendCommmaSeparatedLineToList(QStringList &list, QByteArrayView line)
{
    for (auto &c : QLatin1String(line).tokenize(QLatin1String(","), Qt::SkipEmptyParts))
        list << c.trimmed().toString();
}

/*!
    \internal
 */
void QWebSocketHandshakeRequest::readHandshake(QByteArrayView header, int maxHeaderLineLength)
{
    clear();
    m_parser.setMaxHeaderFieldSize(maxHeaderLineLength);
    QString requestLine = QString::fromLatin1(readLine(header, maxHeaderLineLength));
    if (requestLine.isNull()) {
        clear();
        return;
    }
    const QStringList tokens = requestLine.split(QLatin1Char(' '), Qt::SkipEmptyParts);
    if (Q_UNLIKELY(tokens.size() < 3)) {
        clear();
        return;
    }
    const QString verb(tokens.at(0));
    const QString resourceName(tokens.at(1));
    const QString httpProtocol(tokens.at(2));
    bool conversionOk = false;
    const float httpVersion = QStringView(httpProtocol).mid(5).toFloat(&conversionOk);

    if (Q_UNLIKELY(!conversionOk)) {
        clear();
        return;
    }

    bool parsed = m_parser.parseHeaders(header);
    if (!parsed) {
        clear();
        return;
    }

    m_requestUrl = QUrl::fromEncoded(resourceName.toLatin1());
    QString host = QString::fromLatin1(m_parser.firstHeaderField("host"));
    if (m_requestUrl.isRelative()) {
        // see https://tools.ietf.org/html/rfc6455#page-17
        // No. 4 item in "The requirements for this handshake"
        m_requestUrl.setAuthority(host);
        if (!m_requestUrl.userName().isNull()) { // If the username is null, the password must be too.
            m_isValid = false;
            clear();
            return;
        }
    }
    if (m_requestUrl.scheme().isEmpty()) {
        const QString scheme =  isSecure() ? QStringLiteral("wss") : QStringLiteral("ws");
        m_requestUrl.setScheme(scheme);
    }

    const QList<QByteArray> versionLines = m_parser.headerFieldValues("sec-websocket-version");
    for (auto v = versionLines.begin(); v != versionLines.end(); ++v) {
        for (const auto &version :
             QString::fromLatin1(*v).tokenize(QStringLiteral(","), Qt::SkipEmptyParts)) {
            bool ok = false;
            (void)version.toUInt(&ok);
            if (!ok) {
                clear();
                return;
            }
            const QWebSocketProtocol::Version ver =
                    QWebSocketProtocol::versionFromString(version.trimmed());
            m_versions << ver;
        }
    }
    //sort in descending order
    std::sort(m_versions.begin(), m_versions.end(), std::greater<QWebSocketProtocol::Version>());

    m_key = QString::fromLatin1(m_parser.firstHeaderField("sec-websocket-key"));
    const QByteArray upgrade = m_parser.firstHeaderField("upgrade");
    QStringList connectionValues;
    appendCommmaSeparatedLineToList(connectionValues, m_parser.firstHeaderField("connection"));

    //optional headers
    m_origin = QString::fromLatin1(m_parser.firstHeaderField("origin"));
    const QList<QByteArray> protocolLines = m_parser.headerFieldValues("sec-websocket-protocol");
    for (const QByteArray &pl : protocolLines)
        appendCommmaSeparatedLineToList(m_protocols, pl);

    const QList<QByteArray> extensionLines = m_parser.headerFieldValues("sec-websocket-extensions");
    for (const QByteArray &el : extensionLines)
        appendCommmaSeparatedLineToList(m_extensions, el);

    //TODO: authentication field

    m_isValid = !(m_requestUrl.host().isEmpty() || resourceName.isEmpty() || m_versions.isEmpty()
                  || m_key.isEmpty() || verb != QStringLiteral("GET") || !conversionOk
                  || httpVersion < 1.1f || upgrade.compare("websocket", Qt::CaseInsensitive) != 0
                  || !connectionValues.contains(QStringLiteral("upgrade"), Qt::CaseInsensitive));
    if (Q_UNLIKELY(!m_isValid))
        clear();
}

QT_END_NAMESPACE
