// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qhttpserverconfiguration.h"

QT_BEGIN_NAMESPACE

/*!
    \class QHttpServerConfiguration
    \since 6.9
    \inmodule QtHttpServer
    \brief The QHttpServerConfiguration class controls server parameters.
*/
class QHttpServerConfigurationPrivate : public QSharedData
{
public:
    bool equals(const QHttpServerConfigurationPrivate &other) const noexcept
    {
        return rateLimit == other.rateLimit
            && keepAliveTimeout == other.keepAliveTimeout
            && whitelist == other.whitelist
            && blacklist == other.blacklist
            && maxUrlSize == other.maxUrlSize
            && maxTotalHeaderSize == other.maxTotalHeaderSize
            && maxHeaderFieldSize == other.maxHeaderFieldSize
            && maxNumberOfHeaderFields == other.maxNumberOfHeaderFields
            && maxBodySize == other.maxBodySize;
    }

    quint32 rateLimit = 0;
    std::chrono::seconds keepAliveTimeout = std::chrono::seconds(15);
    QList<std::pair<QHostAddress, int>> whitelist;
    QList<std::pair<QHostAddress, int>> blacklist;
    qint64 maxUrlSize = 64 * 1024;
    qint64 maxTotalHeaderSize = 64 * 1024;
    qint64 maxHeaderFieldSize = 48 * 1024;
    qint64 maxNumberOfHeaderFields = 128;
    qint64 maxBodySize = 32 * 1024 * 1024;
};

QT_DEFINE_QESDP_SPECIALIZATION_DTOR(QHttpServerConfigurationPrivate)

/*!
    Default constructs a QHttpServerConfiguration object.

    Such a configuration has the following values:
     \list
         \li Rate limit is disabled
     \endlist
*/
QHttpServerConfiguration::QHttpServerConfiguration()
    : d(new QHttpServerConfigurationPrivate)
{
}

/*!
    Copy-constructs this QHttpServerConfiguration.
*/
QHttpServerConfiguration::QHttpServerConfiguration(const QHttpServerConfiguration &) = default;

/*!
    \fn QHttpServerConfiguration::QHttpServerConfiguration(QHttpServerConfiguration &&other) noexcept

    Move-constructs this QHttpServerConfiguration from \a other
*/

/*!
    Copy-assigns \a other to this QHttpServerConfiguration.
*/
QHttpServerConfiguration &QHttpServerConfiguration::operator=(const QHttpServerConfiguration &) = default;

/*!
    \fn QHttpServerConfiguration &QHttpServerConfiguration::operator=(QHttpServerConfiguration &&other) noexcept

    Move-assigns \a other to this QHttpServerConfiguration.
*/

/*!
    Destructor.
*/
QHttpServerConfiguration::~QHttpServerConfiguration()
    = default;

/*!
    Sets \a maxRequests as the maximum number of incoming requests
    per second per IP that will be accepted by QHttpServer.
    If the limit is exceeded, QHttpServer will respond with
    QHttpServerResponder::StatusCode::TooManyRequests.

    \sa rateLimitPerSecond(), QHttpServerResponder::StatusCode
*/
void QHttpServerConfiguration::setRateLimitPerSecond(quint32 maxRequests)
{
    d.detach();
    d->rateLimit = maxRequests;
}

/*!
    Returns maximum number of incoming requests per second per IP
    accepted by the server.

    \sa setRateLimitPerSecond()
*/
quint32 QHttpServerConfiguration::rateLimitPerSecond() const
{
    return d->rateLimit;
}

/*!
    \since 6.10

    Sets \a timeout as keep-alive timeout for QHttpServer.

    The keep-alive timeout determines how long an idle connection is kept
    open before being closed.
    By default, the timeout is set to 15 seconds.

    \sa keepAliveTimeout()
*/
void QHttpServerConfiguration::setKeepAliveTimeout(std::chrono::seconds timeout)
{
    d.detach();
    d->keepAliveTimeout = timeout;
}

/*!
    \since 6.10

    Returns the keep-alive timeout used by QHttpServer.

    \sa setKeepAliveTimeout()
*/
std::chrono::seconds QHttpServerConfiguration::keepAliveTimeout() const
{
    return d->keepAliveTimeout;
}

/*!
    \since 6.10

    Sets \a subnetList as the whitelist of allowed subnets.

    When the list is not empty, only IP addresses in this list
    will be allowed by QHttpServer. The whitelist takes priority
    over the blacklist.

    Each subnet is represented as a pair consisting of:
    \list
      \li A base IP address of type QHostAddress.
      \li A CIDR prefix length of type int, which defines the subnet mask.
    \endlist

    To allow only a specific IP address, use a prefix length of 32 for IPv4
    (e.g., \c "192.168.1.100/32") or 128 for IPv6 (e.g., \c "2001:db8::1/128").

    \sa whitelist(), setBlacklist(), QHostAddress::parseSubnet()
*/
void QHttpServerConfiguration::setWhitelist(QSpan<const std::pair<QHostAddress, int>> subnetList)
{
    d.detach();
    d->whitelist.assign(subnetList.begin(), subnetList.end());
}

/*!
    \since 6.10

    Returns the whitelist of subnets allowed by QHttpServer.

    \sa setWhitelist()
*/
QSpan<const std::pair<QHostAddress, int>> QHttpServerConfiguration::whitelist() const
{
    return d->whitelist;
}

/*!
    \since 6.10

    Sets \a subnetList as the blacklist of subnets.

    IP addresses in this list will be denied access by QHttpServer.
    The blacklist is active only when the whitelist is empty.

    \sa blacklist(), setWhitelist(), QHostAddress::parseSubnet()
*/
void QHttpServerConfiguration::setBlacklist(QSpan<const std::pair<QHostAddress, int>> subnetList)
{
    d.detach();
    d->blacklist.assign(subnetList.begin(), subnetList.end());
}

/*!
    \since 6.10

    Returns the blacklist of subnets that are denied access by QHttpServer.

    \sa setBlacklist()
*/
QSpan<const std::pair<QHostAddress, int>> QHttpServerConfiguration::blacklist() const
{
    return d->blacklist;
}

/*!
    \since 6.11

    Sets \a maxSize as the maximum size of the URL
    that the server will accept. If the limit is exceeded,
    QHttpServer will respond with status 414 Request-URI
    Too Long. If \a maxSize is -1 there is no limit.
    The default is 64 KiB.

    \sa maximumUrlSize(), QHttpServerResponder::StatusCode
*/
void QHttpServerConfiguration::setMaximumUrlSize(qint64 maxSize)
{
    d.detach();
    d->maxUrlSize = maxSize;
}

/*!
    \since 6.11

    Returns maximum size of the URL accepted by the
    server.

    \sa setMaximumUrlSize()
*/
qint64 QHttpServerConfiguration::maximumUrlSize() const
{
    return d->maxUrlSize;
}

/*!
    \since 6.11

    Sets \a maxSize as the maximum total size of the headers
    that the server will accept. If the limit is exceeded,
    QHttpServer will respond with status 431 Request Header
    Fields Too Large. If \a maxSize is -1 there is no limit.
    The default is 64 KiB.

    \sa maximumTotalHeaderSize(), QHttpServerResponder::StatusCode
*/
void QHttpServerConfiguration::setMaximumTotalHeaderSize(qint64 maxSize)
{
    d.detach();
    d->maxTotalHeaderSize = maxSize;
}

/*!
    \since 6.11

    Returns maximum size of the total incoming headers accepted
    by the server.

    \sa setMaximumTotalHeaderSize()
*/
qint64 QHttpServerConfiguration::maximumTotalHeaderSize() const
{
    return d->maxTotalHeaderSize;
}

/*!
    \since 6.11

    Sets \a maxSize as the maximum size of a single header
    that the server will accept. If the limit is exceeded,
    QHttpServer will respond with status 431 Request Header
    Fields Too Large. If \a maxSize is -1 there is no limit.
    The default is 48 KiB.

    \sa maximumHeaderFieldSize(), QHttpServerResponder::StatusCode
*/
void QHttpServerConfiguration::setMaximumHeaderFieldSize(qint64 maxSize)
{
    d.detach();
    d->maxHeaderFieldSize = maxSize;
}

/*!
    \since 6.11

    Returns maximum size of the incoming headers accepted
    by the server.

    \sa setMaximumHeaderFieldSize()
*/
qint64 QHttpServerConfiguration::maximumHeaderFieldSize() const
{
    return d->maxHeaderFieldSize;
}

/*!
    \since 6.11

    Sets \a maxSize as the maximum number of header fields
    that the server will accept. If the limit is exceeded,
    QHttpServer will respond with status 431 Request Header
    Fields Too Large. If \a maxSize is -1 there is no limit.
    The default is 128.

    \sa maximumHeaderFieldCount(), QHttpServerResponder::StatusCode
*/
void QHttpServerConfiguration::setMaximumHeaderFieldCount(qint64 maxSize)
{
    d.detach();
    d->maxNumberOfHeaderFields = maxSize;
}

/*!
    \since 6.11

    Returns maximum number of incoming header fields accepted
    by the server.

    \sa setMaximumHeaderFieldCount()
*/
qint64 QHttpServerConfiguration::maximumHeaderFieldCount() const
{
    return d->maxNumberOfHeaderFields;
}

/*!
    \since 6.11

    Sets \a maxSize as the maximum size of the body
    that the server will accept. If the limit is exceeded,
    QHttpServer will respond with status 413 Content Too Large.
    If \a maxSize is -1 there is no limit. The default is 32 MiB.

    \sa maximumBodySize(), QHttpServerResponder::StatusCode
*/
void QHttpServerConfiguration::setMaximumBodySize(qint64 maxSize)
{
    d.detach();
    d->maxBodySize = maxSize;
}

/*!
    \since 6.11

    Returns maximum size of the incoming body accepted
    by the server.

    \sa setMaximumBodySize()
*/
qint64 QHttpServerConfiguration::maximumBodySize() const
{
    return d->maxBodySize;
}

/*!
    \fn void QHttpServerConfiguration::swap(QHttpServerConfiguration &other)
    \memberswap{configuration}
*/

/*!
    \fn bool QHttpServerConfiguration::operator==(const QHttpServerConfiguration &lhs, const QHttpServerConfiguration &rhs) noexcept
    Returns \c true if \a lhs and \a rhs have the same set of configuration
    parameters.
*/

/*!
    \fn bool QHttpServerConfiguration::operator!=(const QHttpServerConfiguration &lhs, const QHttpServerConfiguration &rhs) noexcept
    Returns \c true if \a lhs and \a rhs do not have the same set of configuration
    parameters.
*/

/*!
    \internal
*/
bool comparesEqual(const QHttpServerConfiguration &lhs, const QHttpServerConfiguration &rhs) noexcept
{
    return lhs.d == rhs.d || lhs.d->equals(*rhs.d);
}

QT_END_NAMESPACE
