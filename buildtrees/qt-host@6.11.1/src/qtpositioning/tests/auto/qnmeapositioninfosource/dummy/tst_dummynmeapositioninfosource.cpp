// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

//TESTED_COMPONENT=src/location

#include "../utils/qnmeaproxyfactory.h"
#include "../utils/qlocationtestutils_p.h"

#include <QtPositioning/qnmeapositioninfosource.h>
#include <QSignalSpy>
#include <QTest>

Q_DECLARE_METATYPE(QNmeaPositionInfoSource::UpdateMode)

class DummyNmeaPositionInfoSource : public QNmeaPositionInfoSource
{
    Q_OBJECT

public:
    DummyNmeaPositionInfoSource(QNmeaPositionInfoSource::UpdateMode mode, QObject *parent = 0);

protected:
#if QT_VERSION < QT_VERSION_CHECK(7, 0, 0)
    bool parsePosInfoFromNmeaData(const char *data,
                                  int size,
                                  QGeoPositionInfo *posInfo,
                                  bool *hasFix)
                                  QT6_ONLY(override)
    {
        return parsePosInfoFromNmeaDataImpl(QByteArrayView{data, size}, posInfo, hasFix);
    }
#else
    bool parsePosInfoFromNmeaData(QByteArrayView data,
                                  QGeoPositionInfo *posInfo,
                                  bool *hasFix)
                                  QT7_ONLY(override)
    {
        return parsePosInfoFromNmeaDataImpl(data, posInfo, hasFix);
    }
#endif

private:
    int callCount;

    bool parsePosInfoFromNmeaDataImpl(QByteArrayView data,
                                      QGeoPositionInfo *posInfo,
                                      bool *hasFix);
};

DummyNmeaPositionInfoSource::DummyNmeaPositionInfoSource(QNmeaPositionInfoSource::UpdateMode mode, QObject *parent) :
        QNmeaPositionInfoSource(mode, parent),
        callCount(0)
{
}

bool DummyNmeaPositionInfoSource::parsePosInfoFromNmeaDataImpl(QByteArrayView data,
                                                               QGeoPositionInfo *posInfo,
                                                               bool *hasFix)
{
    Q_UNUSED(data);
    posInfo->setCoordinate(QGeoCoordinate(callCount * 1.0, callCount * 1.0, callCount * 1.0));
    posInfo->setTimestamp(QDateTime::currentDateTimeUtc());
    *hasFix = true;
    ++callCount;

    return true;
}

class tst_DummyNmeaPositionInfoSource : public QObject
{
    Q_OBJECT

public:
    tst_DummyNmeaPositionInfoSource();

private slots:
    void initTestCase();
    void testOverloadedParseFunction();
};


tst_DummyNmeaPositionInfoSource::tst_DummyNmeaPositionInfoSource() {}

void tst_DummyNmeaPositionInfoSource::initTestCase()
{

}

void tst_DummyNmeaPositionInfoSource::testOverloadedParseFunction()
{
    DummyNmeaPositionInfoSource source(QNmeaPositionInfoSource::RealTimeMode);
    QNmeaProxyFactory factory;
    // proxy is deleted by the source
    QNmeaPositionInfoSourceProxy *proxy = static_cast<QNmeaPositionInfoSourceProxy *>(
            factory.createPositionInfoSourceProxy(&source));

    QSignalSpy spy(proxy->source(), SIGNAL(positionUpdated(QGeoPositionInfo)));

    QGeoPositionInfo pos;

    proxy->source()->startUpdates();

    proxy->feedBytes(QString("The parser converts\n").toLatin1());

    QTRY_VERIFY_WITH_TIMEOUT((spy.size() == 1), 10000);
    pos = spy.at(0).at(0).value<QGeoPositionInfo>();

    QVERIFY((pos.coordinate().latitude() == 0.0)
        && (pos.coordinate().longitude() == 0.0)
        && (pos.coordinate().altitude() == 0.0));

    spy.clear();

    proxy->feedBytes(QString("any data it receives\n").toLatin1());

    QTRY_VERIFY_WITH_TIMEOUT((spy.size() == 1), 10000);
    pos = spy.at(0).at(0).value<QGeoPositionInfo>();

    QVERIFY((pos.coordinate().latitude() == 1.0)
        && (pos.coordinate().longitude() == 1.0)
        && (pos.coordinate().altitude() == 1.0));

    spy.clear();

    proxy->feedBytes(QString("into positions\n").toLatin1());

    QTRY_VERIFY_WITH_TIMEOUT((spy.size() == 1), 10000);
    pos = spy.at(0).at(0).value<QGeoPositionInfo>();

    QVERIFY((pos.coordinate().latitude() == 2.0)
        && (pos.coordinate().longitude() == 2.0)
        && (pos.coordinate().altitude() == 2.0));

    spy.clear();
}

#include "tst_dummynmeapositioninfosource.moc"

QTEST_GUILESS_MAIN(tst_DummyNmeaPositionInfoSource);
