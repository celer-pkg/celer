// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtHttpServer/private/qhttpserverrequestfilter_p.h>
#include <QtHttpServer/qhttpserverconfiguration.h>

#include <QtTest/qtest.h>

QT_BEGIN_NAMESPACE

class tst_QHttpServerRequestFilter : public QObject
{
    Q_OBJECT

private slots:
    void testIsRequestWithinRate();
    void testIsRequestAllowed();
    void testIsUrlSizeAllowed();
    void testIsTotalHeaderSizeAllowed();
    void testIsHeaderFieldSizeAllowed();
    void testIsNumberOfHeaderFieldsAllowed();
    void testIsBodySizeAllowed();
};

void tst_QHttpServerRequestFilter::testIsRequestWithinRate()
{
    using namespace QHttpServerRequestFilterPrivate;

    QHttpServerRequestFilter filter1;
    QHostAddress address("127.0.0.1");
    QCOMPARE(filter1.isRequestWithinRate(address), true);

    QHttpServerConfiguration config;
    config.setRateLimitPerSecond(0);
    filter1.setConfiguration(config);
    QCOMPARE(filter1.isRequestWithinRate(address), true);

    QHttpServerRequestFilter filter2;
    config.setRateLimitPerSecond(1);
    filter2.setConfiguration(config);
    qint64 currTimeMSec = QDateTime::currentMSecsSinceEpoch();
    QCOMPARE(filter2.isRequestWithinRate(address, currTimeMSec), true);
    QCOMPARE(filter2.isRequestWithinRate(address, currTimeMSec), false);
    // check next period
    currTimeMSec += cPeriodDurationMSec + 1;
    QCOMPARE(filter2.isRequestWithinRate(address, currTimeMSec), true);
    QCOMPARE(filter2.isRequestWithinRate(address, currTimeMSec), false);
    // check after 2 periods
    currTimeMSec += 2 * cPeriodDurationMSec + 1;
    QCOMPARE(filter2.isRequestWithinRate(address, currTimeMSec), true);
    QCOMPARE(filter2.isRequestWithinRate(address, currTimeMSec), false);
    // check after previous ip info becomes garbage
    QHostAddress address2("127.0.0.2");
    currTimeMSec += 3 * cPeriodDurationMSec;
    QCOMPARE(filter2.isRequestWithinRate(address2, currTimeMSec), true);
    QCOMPARE(filter2.isRequestWithinRate(address2, currTimeMSec), false);
    // check after this ip becomes garbage
    currTimeMSec += 3 * cPeriodDurationMSec;
    QCOMPARE(filter2.isRequestWithinRate(address2, currTimeMSec), true);
    QCOMPARE(filter2.isRequestWithinRate(address2, currTimeMSec), false);

    // higher rate limit
    QHttpServerRequestFilter filter3;
    const int nRequests = 10;
    config.setRateLimitPerSecond(nRequests);
    filter3.setConfiguration(config);
    currTimeMSec = QDateTime::currentMSecsSinceEpoch();
    QCOMPARE(filter3.isRequestWithinRate(address2, currTimeMSec), true);
    currTimeMSec += cPeriodDurationMSec;
    for (int i = 0; i < nRequests - 1; ++i)
        QCOMPARE(filter3.isRequestWithinRate(address2, currTimeMSec), true);
    QCOMPARE(filter3.isRequestWithinRate(address2, currTimeMSec), false);
    // check next period
    currTimeMSec += 1;
    QCOMPARE(filter3.isRequestWithinRate(address2, currTimeMSec), true);
    for (int i = 0; i < nRequests - 1; ++i)
        QCOMPARE(filter3.isRequestWithinRate(address2, currTimeMSec), true);
    QCOMPARE(filter3.isRequestWithinRate(address2, currTimeMSec), false);

    // more hosts
    QHttpServerRequestFilter filter4;
    config.setRateLimitPerSecond(1);
    filter4.setConfiguration(config);
    currTimeMSec = QDateTime::currentMSecsSinceEpoch();
    for (int i = 0; i < 10; ++i) {
        const auto hostAddress = QHostAddress(QString("127.0.0.%1").arg(i));
        QCOMPARE(filter4.isRequestWithinRate(hostAddress, currTimeMSec), true);
        QCOMPARE(filter4.isRequestWithinRate(hostAddress, currTimeMSec), false);
    }
    // check after all the hosts become garbage
    currTimeMSec += 3 * cPeriodDurationMSec;
    QCOMPARE(filter4.isRequestWithinRate(QHostAddress("168.0.0.1"), currTimeMSec), true);
}

void tst_QHttpServerRequestFilter::testIsRequestAllowed()
{
    using namespace QHttpServerRequestFilterPrivate;

    QHttpServerRequestFilter filter;
    QHttpServerConfiguration config;

    // no blacklist or whitelist set
    filter.setConfiguration(config);
    QCOMPARE(filter.isRequestAllowed(QHostAddress("127.0.0.1")), true);

    // whitelist only
    QList<std::pair<QHostAddress, int>> whiteList = {
        { QHostAddress("192.168.1.100"), 32 },
        { QHostAddress("10.0.0.0"), 8 }
    };
    config.setWhitelist(whiteList);
    filter.setConfiguration(config);

    QCOMPARE(filter.isRequestAllowed(QHostAddress("192.168.1.100")), true);
    QCOMPARE(filter.isRequestAllowed(QHostAddress("192.168.1.101")), false);
    QCOMPARE(filter.isRequestAllowed(QHostAddress("10.0.0.50")), true);
    QCOMPARE(filter.isRequestAllowed(QHostAddress("11.0.0.1")), false);

    // blacklist only
    QList<std::pair<QHostAddress, int>> blackList = {
        { QHostAddress("192.168.1.200"), 32 },
        { QHostAddress("172.16.0.0"), 12 }
    };
    config.setBlacklist(blackList);
    config.setWhitelist({});
    filter.setConfiguration(config);

    QCOMPARE(filter.isRequestAllowed(QHostAddress("192.168.1.200")), false);
    QCOMPARE(filter.isRequestAllowed(QHostAddress("192.168.1.201")), true);
    QCOMPARE(filter.isRequestAllowed(QHostAddress("172.16.5.10")), false);
    QCOMPARE(filter.isRequestAllowed(QHostAddress("172.32.0.1")), true);

    // both whitelist and blacklist (whitelist should take priority)
    whiteList = {
        { QHostAddress("10.0.0.1"), 32 }
    };
    blackList = {
        { QHostAddress("10.0.0.0"), 8 }
    };
    config.setWhitelist(whiteList);
    config.setBlacklist(blackList);
    filter.setConfiguration(config);

    QCOMPARE(filter.isRequestAllowed(QHostAddress("10.0.0.1")), true);
    QCOMPARE(filter.isRequestAllowed(QHostAddress("10.0.0.2")), false);
    QCOMPARE(filter.isRequestAllowed(QHostAddress("192.168.0.1")), false);
}

void tst_QHttpServerRequestFilter::testIsUrlSizeAllowed()
{
    using namespace QHttpServerRequestFilterPrivate;

    QHttpServerRequestFilter filter;
    QHttpServerConfiguration config;

    filter.setConfiguration(config);
    QCOMPARE(filter.isUrlSizeAllowed(1000), true);

    config.setMaximumUrlSize(100);
    filter.setConfiguration(config);
    QCOMPARE(filter.isUrlSizeAllowed(100), true);
    QCOMPARE(filter.isUrlSizeAllowed(101), false);
    QCOMPARE(filter.isUrlSizeAllowed(1024 * 1024), false);
}

void tst_QHttpServerRequestFilter::testIsTotalHeaderSizeAllowed()
{
    using namespace QHttpServerRequestFilterPrivate;

    QHttpServerRequestFilter filter;
    QHttpServerConfiguration config;

    filter.setConfiguration(config);
    QCOMPARE(filter.isTotalHeaderSizeAllowed(1000), true);

    config.setMaximumTotalHeaderSize(100);
    filter.setConfiguration(config);
    QCOMPARE(filter.isTotalHeaderSizeAllowed(100), true);
    QCOMPARE(filter.isTotalHeaderSizeAllowed(101), false);
    QCOMPARE(filter.isTotalHeaderSizeAllowed(1024 * 1024), false);

    config.setMaximumTotalHeaderSize(-1);
    filter.setConfiguration(config);
    QCOMPARE(filter.isTotalHeaderSizeAllowed(100), true);
    QCOMPARE(filter.isTotalHeaderSizeAllowed(101), true);
    QCOMPARE(filter.isTotalHeaderSizeAllowed(1024 * 1024 * 1024), true);
}

void tst_QHttpServerRequestFilter::testIsHeaderFieldSizeAllowed()
{
    using namespace QHttpServerRequestFilterPrivate;

    QHttpServerRequestFilter filter;
    QHttpServerConfiguration config;

    filter.setConfiguration(config);
    QCOMPARE(filter.isHeaderFieldSizeAllowed(1000), true);

    config.setMaximumHeaderFieldSize(100);
    filter.setConfiguration(config);
    QCOMPARE(filter.isHeaderFieldSizeAllowed(100), true);
    QCOMPARE(filter.isHeaderFieldSizeAllowed(101), false);
    QCOMPARE(filter.isHeaderFieldSizeAllowed(1024 * 1024), false);

    config.setMaximumHeaderFieldSize(-1);
    filter.setConfiguration(config);
    QCOMPARE(filter.isHeaderFieldSizeAllowed(100), true);
    QCOMPARE(filter.isHeaderFieldSizeAllowed(101), true);
    QCOMPARE(filter.isHeaderFieldSizeAllowed(1024 * 1024 * 1024), true);
}

void tst_QHttpServerRequestFilter::testIsNumberOfHeaderFieldsAllowed()
{
    using namespace QHttpServerRequestFilterPrivate;

    QHttpServerRequestFilter filter;
    QHttpServerConfiguration config;

    filter.setConfiguration(config);
    QCOMPARE(filter.isNumberOfHeaderFieldsAllowed(100), true);

    config.setMaximumHeaderFieldCount(100);
    filter.setConfiguration(config);
    QCOMPARE(filter.isNumberOfHeaderFieldsAllowed(100), true);
    QCOMPARE(filter.isNumberOfHeaderFieldsAllowed(101), false);
    QCOMPARE(filter.isNumberOfHeaderFieldsAllowed(1024 * 1024), false);

    config.setMaximumHeaderFieldCount(-1);
    filter.setConfiguration(config);
    QCOMPARE(filter.isNumberOfHeaderFieldsAllowed(100), true);
    QCOMPARE(filter.isNumberOfHeaderFieldsAllowed(101), true);
    QCOMPARE(filter.isNumberOfHeaderFieldsAllowed(1024 * 1024), true);
}

void tst_QHttpServerRequestFilter::testIsBodySizeAllowed()
{
    using namespace QHttpServerRequestFilterPrivate;

    QHttpServerRequestFilter filter;
    QHttpServerConfiguration config;

    filter.setConfiguration(config);
    QCOMPARE(filter.isBodySizeAllowed(1000), true);

    config.setMaximumBodySize(100);
    filter.setConfiguration(config);
    QCOMPARE(filter.isBodySizeAllowed(100), true);
    QCOMPARE(filter.isBodySizeAllowed(101), false);
    QCOMPARE(filter.isBodySizeAllowed(1024 * 1024), false);
}

QT_END_NAMESPACE

QTEST_MAIN(tst_QHttpServerRequestFilter)

#include "tst_qhttpserverrequestfilter.moc"
