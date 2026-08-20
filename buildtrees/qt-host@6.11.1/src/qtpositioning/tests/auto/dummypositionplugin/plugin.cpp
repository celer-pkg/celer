// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtPositioning/qgeopositioninfosource.h>
#include <QtPositioning/qgeopositioninfosourcefactory.h>
#include <QtPositioning/qgeosatelliteinfosource.h>
#include <QObject>
#include <QtPlugin>

QT_USE_NAMESPACE

// This is a dummy plugin that is created mostly to test some features of
// QDeclarativePositionSource. It does not provide any position updates.
// Use plugin from positionplugin subdirectory, if you need to simulate some
// positioning updates.

class DummyPluginSource : public QGeoPositionInfoSource
{
    Q_OBJECT

public:
    DummyPluginSource(const QVariantMap &parameters, QObject *parent = 0);
    ~DummyPluginSource();

    void startUpdates() override;
    void stopUpdates() override;
    void requestUpdate(int timeout = 5000) override;

    QGeoPositionInfo lastKnownPosition(bool fromSatellitePositioningMethodsOnly) const override;
    PositioningMethods supportedPositioningMethods() const override;

    void setUpdateInterval(int msec) override;
    int minimumUpdateInterval() const override;
    Error error() const override;

private:
    Error lastError = QGeoPositionInfoSource::NoError;
};

DummyPluginSource::DummyPluginSource(const QVariantMap &parameters, QObject *parent)
    : QGeoPositionInfoSource(parent)
{
    Q_UNUSED(parameters)
}

QGeoPositionInfoSource::Error DummyPluginSource::error() const
{
    return lastError;
}

void DummyPluginSource::setUpdateInterval(int msec)
{
    if (msec < minimumUpdateInterval())
        msec = minimumUpdateInterval();

    QGeoPositionInfoSource::setUpdateInterval(msec);
}

int DummyPluginSource::minimumUpdateInterval() const
{
    return 100;
}

QGeoPositionInfo
DummyPluginSource::lastKnownPosition(bool fromSatellitePositioningMethodsOnly) const
{
    Q_UNUSED(fromSatellitePositioningMethodsOnly);
    return QGeoPositionInfo();
}

QGeoPositionInfoSource::PositioningMethods DummyPluginSource::supportedPositioningMethods() const
{
    return QGeoPositionInfoSource::NonSatellitePositioningMethods;
}

void DummyPluginSource::startUpdates()
{
    lastError = QGeoPositionInfoSource::NoError;
}

void DummyPluginSource::stopUpdates() { }

void DummyPluginSource::requestUpdate(int timeout)
{
    lastError = QGeoPositionInfoSource::NoError;
    if (timeout < minimumUpdateInterval()) {
        lastError = QGeoPositionInfoSource::UpdateTimeoutError;
        emit QGeoPositionInfoSource::errorOccurred(lastError);
    }
}

DummyPluginSource::~DummyPluginSource() { }

class DummySatelliteSourcePlugin : public QGeoSatelliteInfoSource
{
    Q_OBJECT
public:
    DummySatelliteSourcePlugin(const QVariantMap &parameters, QObject *parent = 0);
    ~DummySatelliteSourcePlugin();

    int minimumUpdateInterval() const override;
    Error error() const override;

public slots:
    void startUpdates() override;
    void stopUpdates() override;
    void requestUpdate(int timeout = 0) override;

private:
    Error lastError = QGeoSatelliteInfoSource::NoError;
};

DummySatelliteSourcePlugin::DummySatelliteSourcePlugin(const QVariantMap &parameters,
                                                       QObject *parent)
    : QGeoSatelliteInfoSource(parent)
{
    Q_UNUSED(parameters)
}

DummySatelliteSourcePlugin::~DummySatelliteSourcePlugin()
{
}

int DummySatelliteSourcePlugin::minimumUpdateInterval() const
{
    return 100;
}

QGeoSatelliteInfoSource::Error DummySatelliteSourcePlugin::error() const
{
    return lastError;
}

void DummySatelliteSourcePlugin::startUpdates()
{
    lastError = QGeoSatelliteInfoSource::NoError;
}

void DummySatelliteSourcePlugin::stopUpdates()
{
    lastError = QGeoSatelliteInfoSource::NoError;
}

void DummySatelliteSourcePlugin::requestUpdate(int timeout)
{
    lastError = QGeoSatelliteInfoSource::NoError;
    if (timeout < minimumUpdateInterval()) {
        lastError = QGeoSatelliteInfoSource::UpdateTimeoutError;
        emit errorOccurred(lastError);
    }
}


class DummyPluginForTestsFactory : public QObject, public QGeoPositionInfoSourceFactory
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.qt.position.sourcefactory/6.0" FILE "plugin.json")
    Q_INTERFACES(QGeoPositionInfoSourceFactory)

public:
    QGeoPositionInfoSource *positionInfoSource(QObject *parent,
                                               const QVariantMap &parameters) override;
    QGeoSatelliteInfoSource *satelliteInfoSource(QObject *parent,
                                                 const QVariantMap &parameters) override;
    QGeoAreaMonitorSource *areaMonitor(QObject *parent, const QVariantMap &parameters) override;
};

QGeoPositionInfoSource *
DummyPluginForTestsFactory::positionInfoSource(QObject *parent, const QVariantMap &parameters)
{
    return new DummyPluginSource(parameters, parent);
}

QGeoSatelliteInfoSource *
DummyPluginForTestsFactory::satelliteInfoSource(QObject *parent, const QVariantMap &parameters)
{
    return new DummySatelliteSourcePlugin(parameters, parent);
}

QGeoAreaMonitorSource *DummyPluginForTestsFactory::areaMonitor(QObject *parent,
                                                               const QVariantMap &parameters)
{
    Q_UNUSED(parent);
    Q_UNUSED(parameters);
    return nullptr;
}

#include "plugin.moc"
