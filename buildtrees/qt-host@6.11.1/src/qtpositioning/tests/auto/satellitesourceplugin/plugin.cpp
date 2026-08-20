// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtPositioning/qgeopositioninfosourcefactory.h>
#include <QtPositioning/qgeosatelliteinfosource.h>
#include <QObject>
#include <QtPlugin>
#include <QTimer>

QT_USE_NAMESPACE

using namespace Qt::StringLiterals;

class SatelliteSource : public QGeoSatelliteInfoSource
{
    Q_OBJECT

public:
    SatelliteSource(const QVariantMap &parameters, QObject *parent=0);
    ~SatelliteSource();

    void startUpdates() override;
    void stopUpdates() override;
    void requestUpdate(int timeout = 5000) override;

    void setUpdateInterval(int msec) override;
    int minimumUpdateInterval() const override;
    Error error() const override;

    bool setBackendProperty(const QString &name, const QVariant &value) override;
    QVariant backendProperty(const QString &name) const override;

private slots:
    void doTimeout();
    void updateSatelliteInfo();

private:
    QTimer *timer;
    QTimer *timeoutTimer;
    QTimer *singleUpdateTimer;
    Error lastError = QGeoSatelliteInfoSource::NoError;
    QList<QGeoSatelliteInfo> satellitesInView;
    QList<QGeoSatelliteInfo> satellitesInUse;

    qsizetype desiredInViewCount = 5;
    qsizetype desiredInUseCount = 3;
    bool useElevation = false;
    bool useAzimuth = false;
    QGeoSatelliteInfo::SatelliteSystem desiredSystem = QGeoSatelliteInfo::GPS;
};

SatelliteSource::SatelliteSource(const QVariantMap &parameters, QObject *parent)
    : QGeoSatelliteInfoSource(parent),
      timer(new QTimer(this)),
      timeoutTimer(new QTimer(this)),
      singleUpdateTimer(new QTimer(this))
{
    QVariant systemType = parameters.value("satellitesystem"_L1, QVariant());
    if (systemType.isValid())
        desiredSystem = systemType.value<QGeoSatelliteInfo::SatelliteSystem>();

    timer->setInterval(200);
    connect(timer, &QTimer::timeout, this, &SatelliteSource::updateSatelliteInfo);
    connect(singleUpdateTimer, &QTimer::timeout, this, &SatelliteSource::updateSatelliteInfo);
    connect(timeoutTimer, &QTimer::timeout, this, &SatelliteSource::doTimeout);
}

SatelliteSource::~SatelliteSource()
{
}

QGeoSatelliteInfoSource::Error SatelliteSource::error() const
{
    return lastError;
}

bool SatelliteSource::setBackendProperty(const QString &name, const QVariant &value)
{
    if (name == "desiredInViewCount"_L1)
        desiredInViewCount = value.value<qsizetype>();
    else if (name == "desiredInUseCount"_L1)
        desiredInUseCount = value.value<qsizetype>();
    else if (name == "useElevation"_L1)
        useElevation = value.toBool();
    else if (name == "useAzimuth"_L1)
        useAzimuth = value.toBool();
    else
        return false;

    return true;
}

QVariant SatelliteSource::backendProperty(const QString &name) const
{
    if (name == "desiredInViewCount"_L1)
        return desiredInViewCount;
    else if (name == "desiredInUseCount"_L1)
        return desiredInUseCount;
    else if (name == "useElevation"_L1)
        return useElevation;
    else if (name == "useAzimuth"_L1)
        return useAzimuth;

    return {};
}

void SatelliteSource::setUpdateInterval(int msec)
{
    const int minInterval = minimumUpdateInterval();
    if (msec == 0) {
        timer->setInterval(minInterval);
    } else if (msec < minInterval) {
        msec = minInterval;
        timer->setInterval(msec);
    } else {
        timer->setInterval(msec);
    }

    QGeoSatelliteInfoSource::setUpdateInterval(msec);
}

int SatelliteSource::minimumUpdateInterval() const
{
    return 200;
}

void SatelliteSource::startUpdates()
{
    lastError = QGeoSatelliteInfoSource::NoError;
    timer->start();
}

void SatelliteSource::stopUpdates()
{
    timer->stop();
}

void SatelliteSource::requestUpdate(int timeout)
{
    if (timeoutTimer->isActive())
        return; // already requested an update

    lastError = QGeoSatelliteInfoSource::NoError;
    // Such logic for timeout handling is needed for tst_qgeosatelliteinfosource.
    // It expects an immediate error in case of negative timeout.
    if (timeout == 0)
        timeout = 5000;
    if (timeout < 0)
        timeout = 0;

    timeoutTimer->setInterval(timeout);
    timeoutTimer->start();

    if (timer->isActive()) {
        timer->stop();
        timer->start();
    }
    singleUpdateTimer->start(minimumUpdateInterval());
}

void SatelliteSource::doTimeout()
{
    timeoutTimer->stop();
    singleUpdateTimer->stop();
    lastError = QGeoSatelliteInfoSource::UpdateTimeoutError;
    emit errorOccurred(lastError);
}

void SatelliteSource::updateSatelliteInfo()
{
    timeoutTimer->stop();
    singleUpdateTimer->stop();

    if (desiredInUseCount > desiredInViewCount)
        desiredInUseCount = desiredInViewCount;

    satellitesInView.clear();
    satellitesInUse.clear();
    for (qsizetype i = 0; i < desiredInViewCount; ++i) {
        QGeoSatelliteInfo si;
        si.setSatelliteSystem(desiredSystem);
        si.setSatelliteIdentifier(i + 1);
        si.setSignalStrength(5 * (i + 1));
        if (useElevation)
            si.setAttribute(QGeoSatelliteInfo::Elevation, 3.0 * (i + 1));
        if (useAzimuth)
            si.setAttribute(QGeoSatelliteInfo::Azimuth, 0.5 * (i + 1));

        satellitesInView.push_back(si);
        if (i < desiredInUseCount)
            satellitesInUse.push_back(si);
    }
    emit satellitesInViewUpdated(satellitesInView);
    emit satellitesInUseUpdated(satellitesInUse);
}


class QGeoPositionInfoSourceFactoryTest : public QObject, public QGeoPositionInfoSourceFactory
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.qt.position.sourcefactory/6.0"
                      FILE "plugin.json")
    Q_INTERFACES(QGeoPositionInfoSourceFactory)

public:
    QGeoPositionInfoSource *positionInfoSource(QObject *parent,
                                               const QVariantMap &parameters) override;
    QGeoSatelliteInfoSource *satelliteInfoSource(QObject *parent,
                                                 const QVariantMap &parameters) override;
    QGeoAreaMonitorSource *areaMonitor(QObject *parent, const QVariantMap &parameters) override;
};

QGeoPositionInfoSource *
QGeoPositionInfoSourceFactoryTest::positionInfoSource(QObject *parent, const QVariantMap &parameters)
{
    Q_UNUSED(parent);
    Q_UNUSED(parameters);
    return nullptr;
}

QGeoSatelliteInfoSource *
QGeoPositionInfoSourceFactoryTest::satelliteInfoSource(QObject *parent, const QVariantMap &parameters)
{
    return new SatelliteSource(parameters, parent);
}

QGeoAreaMonitorSource *
QGeoPositionInfoSourceFactoryTest::areaMonitor(QObject *parent, const QVariantMap &parameters)
{
    Q_UNUSED(parent);
    Q_UNUSED(parameters);
    return nullptr;
}

#include "plugin.moc"
