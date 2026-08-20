// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qhttpserverrequestfilter_p.h"

#include <QtCore/qdatetime.h>

#include <algorithm>

QT_BEGIN_NAMESPACE

const int QHttpServerRequestFilterPrivate::cPeriodDurationMSec = 1000;

// compromise value to remove some garbage without processing the entire array.
static constexpr int cCleanupThreshold = 10;

unsigned int QHttpServerRequestFilter::maxRequestPerPeriod() const
{
    return m_config.rateLimitPerSecond();
}

void QHttpServerRequestFilter::setConfiguration(const QHttpServerConfiguration &config)
{
    m_config = config;
}

bool QHttpServerRequestFilter::isRequestAllowed(const QHostAddress &peerAddress) const
{
    const auto matches = [](const QHostAddress &addr) {
        return [&addr] (const auto &subnet) {
            return addr.isInSubnet(subnet);
        };
    };

    if (const auto whitelist = m_config.whitelist(); !whitelist.empty())
        return std::any_of(whitelist.cbegin(), whitelist.cend(), matches(peerAddress));

    const auto blacklist = m_config.blacklist();
    return std::none_of(blacklist.cbegin(), blacklist.cend(), matches(peerAddress));
}

bool QHttpServerRequestFilter::isRequestWithinRate(const QHostAddress &peerAddress)
{
    return isRequestWithinRate(peerAddress, QDateTime::currentMSecsSinceEpoch());
}

bool QHttpServerRequestFilter::isRequestWithinRate(const QHostAddress &peerAddress,
                                                   qint64 currTimeMSec)
{
    using namespace QHttpServerRequestFilterPrivate;

    if (m_config.rateLimitPerSecond() == 0)
        return true;

    const auto it = ipInfo.tryEmplace(peerAddress, currTimeMSec + cPeriodDurationMSec).iterator;

    bool result = true;
    if (it->isGarbage(currTimeMSec)) {
        // did not make any requests for a whole period? start the new one.
        it->m_thisPeriodEnd = currTimeMSec + cPeriodDurationMSec;
        it->m_nRequests = 1;
    } else if (currTimeMSec > it->m_thisPeriodEnd) {
        // showed up during next period, update info
        it->m_thisPeriodEnd += cPeriodDurationMSec;
        it->m_nRequests = 1;
    } else {
        // check whether we exceeded
        if (++it->m_nRequests > maxRequestPerPeriod())
            result = false;  // too many requests
    }

    // clean more garbage then we create
    cleanIpInfoGarbage(it, currTimeMSec);

    return result;
}

/*!
    \internal
*/
bool QHttpServerRequestFilter::isUrlSizeAllowed(qsizetype urlSize) const
{
    auto limit = m_config.maximumUrlSize();
    return limit < 0 || limit >= qint64(urlSize);
}

/*!
    \internal
*/
bool QHttpServerRequestFilter::isTotalHeaderSizeAllowed(qsizetype headerSize) const
{
    auto limit = m_config.maximumTotalHeaderSize();
    return limit < 0 || limit >= qint64(headerSize);
}

/*!
    \internal
*/
bool QHttpServerRequestFilter::isHeaderFieldSizeAllowed(qsizetype headerSize) const
{
    auto limit = m_config.maximumHeaderFieldSize();
    return limit < 0 || limit >= qint64(headerSize);
}

/*!
    \internal
*/
bool QHttpServerRequestFilter::isNumberOfHeaderFieldsAllowed(qsizetype headerSize) const
{
    auto limit = m_config.maximumHeaderFieldCount();
    return limit < 0 || limit >= qint64(headerSize);
}

/*!
    \internal
*/
bool QHttpServerRequestFilter::isBodySizeAllowed(qint64 bodySize) const
{
    auto limit = m_config.maximumBodySize();
    return limit < 0 || limit >= bodySize;
}

void QHttpServerRequestFilter::cleanIpInfoGarbage(QHash<QHostAddress, IpInfo>::iterator it,
                                                  qint64 currTime)
{
    Q_ASSERT(ipInfo.begin() != ipInfo.end());

    const auto myIp = it.key();
    ++it;
    // check the range after the current ip
    for (int i = 0; i < cCleanupThreshold; ++i) {
        if (it == ipInfo.end())
            it = ipInfo.begin();

        if (it.key() == myIp)
            break;

        if (it->isGarbage(currTime))
            it = ipInfo.erase(it);
        else
            ++it;
    }
}

bool QHttpServerRequestFilter::IpInfo::isGarbage(qint64 currTime) const
{
    // ip info is garbage if we got no requests during next period
    return (currTime >= m_thisPeriodEnd + QHttpServerRequestFilterPrivate::cPeriodDurationMSec);
}

QT_END_NAMESPACE
