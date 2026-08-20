// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtPositioning/qgeopositioninfosource.h>
#include <QtPositioning/qgeopositioninfosourcefactory.h>
#include <QObject>
#include <QtPlugin>
#include <QTimer>

QT_USE_NAMESPACE

class DummySourcePrivate;
class DummySource : public QGeoPositionInfoSource
{
    Q_OBJECT

public:
    DummySource(const QVariantMap &parameters, QObject *parent=0);
    ~DummySource();

    void startUpdates() override;
    void stopUpdates() override;
    void requestUpdate(int timeout = 5000) override;

    QGeoPositionInfo lastKnownPosition(bool fromSatellitePositioningMethodsOnly) const override;
    PositioningMethods supportedPositioningMethods() const override;

    void setUpdateInterval(int msec) override;
    int minimumUpdateInterval() const override;
    Error error() const override;

    bool setBackendProperty(const QString &name, const QVariant &value) override;
    QVariant backendProperty(const QString &name) const override;

private:
    QTimer *timer;
    QTimer *timeoutTimer;
    QTimer *singleTimer;
    QGeoPositionInfo lastPosition;
    QDateTime lastUpdateTime;
    Error lastError = QGeoPositionInfoSource::NoError;
    qreal altitude = 0.0;

private slots:
    void updatePosition();
    void doTimeout();
};

DummySource::DummySource(const QVariantMap &parameters, QObject *parent) :
    QGeoPositionInfoSource(parent),
    timer(new QTimer(this)),
    timeoutTimer(new QTimer(this)),
    singleTimer(new QTimer(this)),
    lastPosition(QGeoCoordinate(0,0), QDateTime::currentDateTime())
{
    if (parameters.contains(QStringLiteral("test.source.altitude"))) {
        const qreal alti = parameters.value(QStringLiteral("test.source.altitude")).toReal();
        altitude = alti;
        QGeoCoordinate crd = lastPosition.coordinate();
        crd.setAltitude(alti);
        lastPosition.setCoordinate(crd);
    }
    timer->setInterval(200);
    connect(timer, SIGNAL(timeout()),
            this, SLOT(updatePosition()));
    connect(singleTimer, SIGNAL(timeout()),
            this, SLOT(updatePosition()));
    connect(timeoutTimer, SIGNAL(timeout()),
            this, SLOT(doTimeout()));
}

QGeoPositionInfoSource::Error DummySource::error() const
{
    return lastError;
}

bool DummySource::setBackendProperty(const QString &name, const QVariant &value)
{
    if (name == QStringLiteral("altitude")) {
        altitude = value.toReal();
        return true;
    }
    return false;
}

QVariant DummySource::backendProperty(const QString &name) const
{
    if (name == QStringLiteral("altitude"))
        return altitude;
    return QVariant();
}


void DummySource::setUpdateInterval(int msec)
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

    QGeoPositionInfoSource::setUpdateInterval(msec);
}

int DummySource::minimumUpdateInterval() const
{
    return 200;
}

QGeoPositionInfo DummySource::lastKnownPosition(bool fromSatellitePositioningMethodsOnly) const
{
    Q_UNUSED(fromSatellitePositioningMethodsOnly);
    return lastPosition;
}

QGeoPositionInfoSource::PositioningMethods DummySource::supportedPositioningMethods() const
{
    return QGeoPositionInfoSource::AllPositioningMethods;
}

void DummySource::startUpdates()
{
    lastError = QGeoPositionInfoSource::NoError;
    timer->start();
}

void DummySource::stopUpdates()
{
    timer->stop();
}

void DummySource::requestUpdate(int timeout)
{
    lastError = QGeoPositionInfoSource::NoError;
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

    singleTimer->setInterval(minimumUpdateInterval());
    singleTimer->start();
}

DummySource::~DummySource()
{}

void DummySource::updatePosition()
{
    timeoutTimer->stop();
    singleTimer->stop();

    const QDateTime now = QDateTime::currentDateTime();

    QGeoCoordinate coord(lastPosition.coordinate().latitude() + 0.1,
                         lastPosition.coordinate().longitude() + 0.1,
                         altitude);

    QGeoPositionInfo info(coord, now);
    info.setAttribute(QGeoPositionInfo::Direction, lastPosition.coordinate().azimuthTo(coord));
    if (lastUpdateTime.isValid()) {
        double speed = lastPosition.coordinate().distanceTo(coord) / lastUpdateTime.msecsTo(now);
        info.setAttribute(QGeoPositionInfo::GroundSpeed, 1000 * speed);
    }

    lastUpdateTime = now;
    lastPosition = info;
    emit positionUpdated(info);
}

void DummySource::doTimeout()
{
    timeoutTimer->stop();
    singleTimer->stop();
    lastError = QGeoPositionInfoSource::UpdateTimeoutError;
    emit QGeoPositionInfoSource::errorOccurred(lastError);
}


class QGeoPositionInfoSourceFactoryTest : public QObject, public QGeoPositionInfoSourceFactory
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.qt.position.sourcefactory/6.0"
                      FILE "plugin.json")
    Q_INTERFACES(QGeoPositionInfoSourceFactory)

public:
    QGeoPositionInfoSource *positionInfoSource(QObject *parent, const QVariantMap &parameters) override;
    QGeoSatelliteInfoSource *satelliteInfoSource(QObject *parent, const QVariantMap &parameters) override;
    QGeoAreaMonitorSource *areaMonitor(QObject *parent, const QVariantMap &parameters) override;
};

QGeoPositionInfoSource *QGeoPositionInfoSourceFactoryTest::positionInfoSource(QObject *parent, const QVariantMap &parameters)
{
    return new DummySource(parameters, parent);
}

QGeoSatelliteInfoSource *QGeoPositionInfoSourceFactoryTest::satelliteInfoSource(QObject *parent, const QVariantMap &parameters)
{
    Q_UNUSED(parent);
    Q_UNUSED(parameters);
    return nullptr;
}

QGeoAreaMonitorSource *QGeoPositionInfoSourceFactoryTest::areaMonitor(QObject *parent, const QVariantMap &parameters)
{
    Q_UNUSED(parent);
    Q_UNUSED(parameters);
    return nullptr;
}

#include "plugin.moc"
