// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QTest>
#include <QTimer>
#include <QtPositioning/QNmeaSatelliteInfoSource>
#include "../../qgeosatelliteinfosource/testqgeosatelliteinfosource_p.h"
#include "../../utils/qnmeaproxyfactory.h"
#include "../../utils/qlocationtestutils_p.h"

class Feeder : public QObject
{
    Q_OBJECT
public:
    Feeder(QObject *parent) : QObject(parent)
    {
    }

    void start(QNmeaSatelliteInfoSourceProxy *proxy)
    {
        m_proxy = proxy;
        QTimer *timer = new QTimer(this);
        QObject::connect(timer, &QTimer::timeout, this, &Feeder::timeout);
        timer->setInterval(proxy->source()->minimumUpdateInterval() * 2);
        timer->start();
    }

public slots:
    void timeout()
    {
        // Here we need to provide different chunks of data, because the signals
        // are emitted only when data changes.
        if (has_data) {
            m_proxy->feedBytes(QLocationTestUtils::createGsvLongSentence().toLatin1());
            m_proxy->feedBytes(QLocationTestUtils::createGsaLongSentence().toLatin1());
        } else {
            m_proxy->feedBytes(QLocationTestUtils::createGsvSentence().toLatin1());
            m_proxy->feedBytes(QLocationTestUtils::createGsaSentence().toLatin1());
        }
        has_data = !has_data;
    }

private:
    QNmeaSatelliteInfoSourceProxy *m_proxy;
    bool has_data = true;
};

class tst_QNmeaSatelliteInfoSource_Generic_Realtime : public TestQGeoSatelliteInfoSource
{
    Q_OBJECT
protected:
    QGeoSatelliteInfoSource *createTestSource() override
    {
        QNmeaSatelliteInfoSource *source =
                new QNmeaSatelliteInfoSource(QNmeaSatelliteInfoSource::UpdateMode::RealTimeMode);
        QNmeaSatelliteInfoSourceProxy *proxy = m_factory.createSatelliteInfoSourceProxy(source);
        Feeder *feeder = new Feeder(source);
        feeder->start(proxy);
        return source;
    }

private:
    QNmeaProxyFactory m_factory;
};

#include "tst_nmeasatelliteinfosource_generic_rt.moc"

QTEST_GUILESS_MAIN(tst_QNmeaSatelliteInfoSource_Generic_Realtime)
