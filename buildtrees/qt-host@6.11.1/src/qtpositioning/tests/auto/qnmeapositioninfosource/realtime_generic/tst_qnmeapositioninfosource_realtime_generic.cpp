// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

//TESTED_COMPONENT=src/location

#include "tst_qnmeapositioninfosource.h"

class tst_QNmeaPositionInfoSource_RealTime_Generic : public TestQGeoPositionInfoSource
{
    Q_OBJECT

public:
    tst_QNmeaPositionInfoSource_RealTime_Generic()
    {
        m_factory = new QNmeaProxyFactory;
#if QT_CONFIG(library)
        /*
         * Set custom path since CI doesn't install test plugins
         */
#ifdef Q_OS_WIN
        QCoreApplication::addLibraryPath(QCoreApplication::applicationDirPath() +
                                     QStringLiteral("/../../../../plugins"));
#else
        QCoreApplication::addLibraryPath(QCoreApplication::applicationDirPath()
                                         + QStringLiteral("/../../../../plugins"));
#endif
#endif
    }

    ~tst_QNmeaPositionInfoSource_RealTime_Generic()
    {
        delete m_factory;
    }

protected:
    QGeoPositionInfoSource *createTestSource() override
    {
        QNmeaPositionInfoSource *source = new QNmeaPositionInfoSource(QNmeaPositionInfoSource::RealTimeMode);
        QNmeaPositionInfoSourceProxy *proxy = static_cast<QNmeaPositionInfoSourceProxy *>(
                m_factory->createPositionInfoSourceProxy(source));
        Feeder *feeder = new Feeder(source);
        feeder->start(proxy);
        return source;
    }

private:
    QNmeaProxyFactory *m_factory;
};

#include "tst_qnmeapositioninfosource_realtime_generic.moc"

QTEST_GUILESS_MAIN(tst_QNmeaPositionInfoSource_RealTime_Generic);
