// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef WEATHERAPIBACKEND_H
#define WEATHERAPIBACKEND_H

#include "providerbackend.h"

QT_BEGIN_NAMESPACE
class QNetworkAccessManager;
class QNetworkReply;
QT_END_NAMESPACE

class WeatherApiBackend : public ProviderBackend
{
    Q_OBJECT
public:
    explicit WeatherApiBackend(QObject *parent = nullptr);

    void requestWeatherInfo(const QString &city) override;
    void requestWeatherInfo(const QGeoCoordinate &coordinate) override;

private slots:
    void handleWeatherForecastReply(QNetworkReply *reply, const QGeoCoordinate &coordinate);

private:
    void generateWeatherRequest(const QString &locationString, const QGeoCoordinate &coordinate);

    QNetworkAccessManager *m_networkManager;
    const QString m_apiKey;
};

#endif // WEATHERAPIBACKEND_H
