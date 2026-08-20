// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef PROVIDERBACKEND_H
#define PROVIDERBACKEND_H

#include <QtCore/qobject.h>
#include <QtPositioning/qgeocoordinate.h>

struct WeatherInfo
{
    QString m_dayOfWeek;
    QString m_weatherIconId;
    QString m_weatherDescription;
    QString m_temperature;
};

struct LocationInfo
{
    QString m_name;
    QGeoCoordinate m_coordinate;
};

class ProviderBackend : public QObject
{
    Q_OBJECT
public:
    explicit ProviderBackend(QObject *parent = nullptr);

    virtual void requestWeatherInfo(const QString &city) = 0;
    virtual void requestWeatherInfo(const QGeoCoordinate &coordinate) = 0;

signals:
    // The first element in weatherDetails represents current weather.
    // Next are the weather forecast, including the current day.
    // The LocationInfo object should contain valid coordinate only when it was
    // initially used to request the weather. If the city name was used, an
    // empty coordinate is expected to be transferred.
    void weatherInformation(const LocationInfo &location, const QList<WeatherInfo> &weatherDetails);
    void errorOccurred();
};

#endif // PROVIDERBACKEND_H
