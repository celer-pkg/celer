// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

//TESTED_COMPONENT=src/location

#include <QtTest/QtTest>
#include <QSignalSpy>
#include <QtPositioning/qgeopositioninfosource.h>
#include <QtPositioning/qgeosatelliteinfosource.h>
#include <QtPositioning/qgeoareamonitorsource.h>
#include <QtPositioning/qgeocoordinate.h>

QT_USE_NAMESPACE

class tst_PositionPlugin : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();

    void availableSources();
    void create();
    void getUpdates();
};

void tst_PositionPlugin::initTestCase()
{
#if QT_CONFIG(library)
    /*
     * Set custom path since CI doesn't install test plugins
     */
#ifdef Q_OS_WIN
    QCoreApplication::addLibraryPath(QCoreApplication::applicationDirPath() +
                                     QStringLiteral("/../../../../plugins"));
#else
    QCoreApplication::addLibraryPath(QCoreApplication::applicationDirPath()
                                     + QStringLiteral("/../../../plugins"));
#endif
#endif
}

void tst_PositionPlugin::availableSources()
{
    QVERIFY(QGeoPositionInfoSource::availableSources().contains("test.source"));
    QVERIFY(!QGeoSatelliteInfoSource::availableSources().contains("test.source"));
    QVERIFY(!QGeoAreaMonitorSource::availableSources().contains("test.source"));
}

void tst_PositionPlugin::create()
{
    std::unique_ptr<QGeoPositionInfoSource> src = nullptr;
    src.reset(QGeoPositionInfoSource::createSource("test.source", 0));
    QVERIFY(src != nullptr);

    QVERIFY(src->minimumUpdateInterval() == 200);

    src.reset(QGeoPositionInfoSource::createSource("invalid source that will never exist", 0));
    QVERIFY(src == 0);

    std::unique_ptr<QGeoSatelliteInfoSource> ssrc = nullptr;
    ssrc.reset(QGeoSatelliteInfoSource::createSource("test.source", 0));
    QVERIFY(ssrc == nullptr);
}

void tst_PositionPlugin::getUpdates()
{
    std::unique_ptr<QGeoPositionInfoSource> src(QGeoPositionInfoSource::createSource("test.source", 0));
    src->setUpdateInterval(200);

    QSignalSpy spy(src.get(), SIGNAL(positionUpdated(QGeoPositionInfo)));
    src->startUpdates();
    QTRY_COMPARE_WITH_TIMEOUT(spy.size(), 1, 5000);
    QCOMPARE(spy[0].size(), 1);

    QGeoPositionInfo info = qvariant_cast<QGeoPositionInfo>(spy[0][0]);
    QCOMPARE(info.coordinate().latitude(), 0.1);
    QCOMPARE(info.coordinate().longitude(), 0.1);
}



QTEST_GUILESS_MAIN(tst_PositionPlugin)
#include "tst_positionplugin.moc"
