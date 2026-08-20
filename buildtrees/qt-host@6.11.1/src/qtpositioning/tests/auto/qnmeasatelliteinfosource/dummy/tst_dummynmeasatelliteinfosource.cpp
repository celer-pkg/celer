// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QTest>
#include <QSignalSpy>
#include <QtPositioning/QNmeaSatelliteInfoSource>
#include "../../utils/qnmeaproxyfactory.h"

#include <memory>

class DummyNmeaSatelliteInfoSource : public QNmeaSatelliteInfoSource
{
    Q_OBJECT

public:
    DummyNmeaSatelliteInfoSource(QObject *parent = 0);

protected:
#if QT_VERSION < QT_VERSION_CHECK(7, 0, 0)
    QGeoSatelliteInfo::SatelliteSystem parseSatellitesInUseFromNmea(const char *data, int size,
                                                                    QList<int> &pnrsInUse)
                                                                    QT6_ONLY(override)
    {
        return parseSatellitesInUseFromNmeaImpl(QByteArrayView{data, size}, pnrsInUse);
    }

    SatelliteInfoParseStatus parseSatelliteInfoFromNmea(const char *data, int size,
                                                        QList<QGeoSatelliteInfo> &infos,
                                                        QGeoSatelliteInfo::SatelliteSystem &system)
                                                        QT6_ONLY(override)
    {
        return parseSatelliteInfoImpl(QByteArrayView{data, size}, infos, system);
    }
#else
    QGeoSatelliteInfo::SatelliteSystem parseSatellitesInUseFromNmea(QByteArrayView data,
                                                                    QList<int> &pnrsInUse)
                                                                    QT7_ONLY(override)
    {
        return parseSatellitesInUseFromNmeaImpl(data, pnrsInUse);
    }
    SatelliteInfoParseStatus parseSatelliteInfoFromNmea(QByteArrayView data,
                                                        QList<QGeoSatelliteInfo> &infos,
                                                        QGeoSatelliteInfo::SatelliteSystem &system)
                                                        QT7_ONLY(override)
    {
        return parseSatelliteInfoImpl(data, infos, system);
    }
#endif
private:
    QGeoSatelliteInfo::SatelliteSystem parseSatellitesInUseFromNmeaImpl(QByteArrayView data,
                                                                        QList<int> &pnrsInUse);
    SatelliteInfoParseStatus parseSatelliteInfoImpl(QByteArrayView data,
                                                    QList<QGeoSatelliteInfo> &infos,
                                                    QGeoSatelliteInfo::SatelliteSystem &system);
};

DummyNmeaSatelliteInfoSource::DummyNmeaSatelliteInfoSource(QObject *parent)
    : QNmeaSatelliteInfoSource(QNmeaSatelliteInfoSource::UpdateMode::RealTimeMode, parent)
{
}

QGeoSatelliteInfo::SatelliteSystem
DummyNmeaSatelliteInfoSource::parseSatellitesInUseFromNmeaImpl(QByteArrayView data,
                                                               QList<int> &pnrsInUse)
{
    // expected format: "USE:num1;num2;num3\n"
    // example: "USE:1;3;4;7\n"
    if (data.isEmpty())
        return QGeoSatelliteInfo::Undefined;

    QString str = QLatin1String(data).toString();
    if (!str.startsWith("USE:"))
        return QGeoSatelliteInfo::Undefined;

    const QStringList sl = str.mid(4).split(";", Qt::SkipEmptyParts);

    if (sl.empty())
        return QGeoSatelliteInfo::Undefined;

    for (const auto &str : sl) {
        bool ok = false;
        int value = str.toInt(&ok);
        if (ok) {
            pnrsInUse.push_back(value);
        }
    }
    return QGeoSatelliteInfo::GPS;
}

QNmeaSatelliteInfoSource::SatelliteInfoParseStatus
DummyNmeaSatelliteInfoSource::parseSatelliteInfoImpl(QByteArrayView data,
                                                     QList<QGeoSatelliteInfo> &infos,
                                                     QGeoSatelliteInfo::SatelliteSystem &system)
{
    // expected format: "INFO:system,identifier;system,identifier;system,identifier\n"
    // example: "INFO:1,5;1,7;1,15\n"
    if (data.isEmpty())
        return NotParsed;

    QString str = QLatin1String(data).toString();
    if (!str.startsWith("INFO:"))
        return NotParsed;

    QStringList sat_infos = str.mid(5).split(";", Qt::SkipEmptyParts);

    if (sat_infos.empty())
        return NotParsed;

    for (const auto &sat_info : sat_infos) {
        QStringList parameters = sat_info.split(",", Qt::SkipEmptyParts);
        if (parameters.size() == 2) {
            QGeoSatelliteInfo info;
            info.setSatelliteSystem(
                    static_cast<QGeoSatelliteInfo::SatelliteSystem>(parameters[0].toInt()));
            info.setSatelliteIdentifier(parameters[1].toInt());
            infos.push_back(info);
        }
    }

    system = infos.isEmpty() ? QGeoSatelliteInfo::Undefined : infos.front().satelliteSystem();

    return FullyParsed;
}

class tst_DummyNmeaSatelliteInfoSource : public QObject
{
    Q_OBJECT

private slots:
    void testOverloadedParseFunction();
};

void tst_DummyNmeaSatelliteInfoSource::testOverloadedParseFunction()
{
    DummyNmeaSatelliteInfoSource source;
    QNmeaProxyFactory factory;
    std::unique_ptr<QNmeaSatelliteInfoSourceProxy> proxy(
            factory.createSatelliteInfoSourceProxy(&source));

    QSignalSpy inUseSpy(proxy->source(), &QNmeaSatelliteInfoSource::satellitesInUseUpdated);
    QSignalSpy inViewSpy(proxy->source(), &QNmeaSatelliteInfoSource::satellitesInViewUpdated);

    proxy->source()->startUpdates();

    // first we need to send all satellites
    proxy->feedBytes("INFO:1,5;1,7;1,15\n");
    // then - used ones
    proxy->feedBytes("USE:5;15\n");

    QTRY_VERIFY_WITH_TIMEOUT(inUseSpy.size() == 1, 10000);
    QTRY_VERIFY_WITH_TIMEOUT(inViewSpy.size() == 1, 10000);

    QGeoSatelliteInfo info_1_5;
    info_1_5.setSatelliteSystem(QGeoSatelliteInfo::GPS);
    info_1_5.setSatelliteIdentifier(5);

    QGeoSatelliteInfo info_1_7;
    info_1_7.setSatelliteSystem(QGeoSatelliteInfo::GPS);
    info_1_7.setSatelliteIdentifier(7);

    QGeoSatelliteInfo info_1_15;
    info_1_15.setSatelliteSystem(QGeoSatelliteInfo::GPS);
    info_1_15.setSatelliteIdentifier(15);

    const QList<QGeoSatelliteInfo> desiredInView = { info_1_5, info_1_7, info_1_15 };
    const QList<QGeoSatelliteInfo> desiredInUse = { info_1_5, info_1_15 };

    const QList<QGeoSatelliteInfo> inViewList =
            inViewSpy.at(0).at(0).value<QList<QGeoSatelliteInfo>>();
    const QList<QGeoSatelliteInfo> inUseList =
            inUseSpy.at(0).at(0).value<QList<QGeoSatelliteInfo>>();

    QCOMPARE(inViewList, desiredInView);
    QCOMPARE(inUseList, desiredInUse);
}

#include "tst_dummynmeasatelliteinfosource.moc"

QTEST_GUILESS_MAIN(tst_DummyNmeaSatelliteInfoSource);
