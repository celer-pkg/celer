// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "openmeteobackend.h"

#include <QtCore/qjsonarray.h>
#include <QtCore/qjsondocument.h>
#include <QtCore/qjsonobject.h>
#include <QtCore/qjsonvalue.h>
#include <QtCore/qloggingcategory.h>
#include <QtCore/qstring.h>
#include <QtCore/qtimezone.h>
#include <QtCore/qurl.h>
#include <QtCore/qurlquery.h>
#include <QtNetwork/qnetworkaccessmanager.h>
#include <QtNetwork/qnetworkreply.h>

Q_DECLARE_LOGGING_CATEGORY(requestsLog)

using namespace Qt::StringLiterals;

static constexpr qsizetype kMaxEntries = 4;

static QString niceTemperatureString(double t)
{
    return QString::number(qRound(t)) + QChar(0xB0);
}

/*
    Weather codes are taken from the bottom of this page:
    https://open-meteo.com/en/docs

    The possible strings are based on the icon names. The icon name is built up
    as follows:
        weather-[mystring].png
*/
static QString weatherCodeToIcon(int code)
{
    switch (code) {
    case 0:
        return "sunny"_L1;
    case 1:
        return "sunny-very-few-clouds"_L1;
    case 2:
        return "few-clouds"_L1;
    case 3:
        return "overcast"_L1;
    case 45:
    case 48:
        return "fog"_L1;
    case 51:
    case 53:
    case 55:
    case 56:
    case 57:
        return "showers-scattered"_L1;
    case 61:
    case 63:
    case 65:
    case 80:
    case 81:
    case 82:
        return "showers"_L1;
    case 66:
    case 67:
        return "sleet"_L1;
    case 71:
    case 73:
    case 75:
    case 77:
    case 85:
    case 86:
        return "snow"_L1;
    case 95:
    case 96:
    case 99:
        return "thundershower"_L1;
    }
    return "sunny"_L1; // default
}

/*
    Weather codes and descriptions are taken from the bottom of this page:
    https://open-meteo.com/en/docs
*/
static QString weatherCodeToDescription(int code)
{
    switch (code) {
    case 0:
        return "clear sky"_L1;
    case 1:
        return "mainly clear"_L1;
    case 2:
        return "partly cloudy"_L1;
    case 3:
        return "overcast"_L1;
    case 45:
    case 48:
        return "fog"_L1;
    case 51:
    case 53:
    case 55:
    case 56:
    case 57:
        return "drizzle"_L1;
    case 61:
    case 63:
    case 65:
        return "rain"_L1;
    case 66:
    case 67:
        return "freezing rain"_L1;
    case 71:
    case 73:
    case 75:
    case 77:
    case 85:
    case 86:
        return "snow"_L1;
    case 80:
    case 81:
    case 82:
        return "pouring rain"_L1;
    case 95:
    case 96:
    case 99:
        return "thunderstorm"_L1;
    }
    return QString();
}

// The supported cities are taken from WeatherInfo.qml.
// Need to keep in sync with it.
static QGeoCoordinate coordinateForCity(const QString &city)
{
    if (city == "Brisbane"_L1)
        return QGeoCoordinate(-27.3818, 152.8531);
    else if (city == "Oslo"_L1)
        return QGeoCoordinate(59.8939, 10.7151);
    else if (city == "Helsinki"_L1)
        return QGeoCoordinate(60.1103, 24.8786);
    else if (city == "New York"_L1)
        return QGeoCoordinate(40.6977, -74.1198);

    return QGeoCoordinate();
}

OpenMeteoBackend::OpenMeteoBackend(QObject *parent)
    : ProviderBackend{parent},
      m_networkManager{new QNetworkAccessManager(this)}
{
}

void OpenMeteoBackend::requestWeatherInfo(const QString &city)
{
    const auto coordinate = coordinateForCity(city);
    if (!coordinate.isValid()) {
        qCDebug(requestsLog) << "Weather request for unknown city:" << city;
        emit errorOccurred();
        return;
    }
    generateWeatherRequest(city, coordinate);
}

void OpenMeteoBackend::requestWeatherInfo(const QGeoCoordinate &coordinate)
{
    generateWeatherRequest(QString(), coordinate);
}

void OpenMeteoBackend::handleWeatherForecastReply(QNetworkReply *reply,
                                                  const LocationInfo &location)
{
    Q_UNUSED(reply);
    Q_UNUSED(location);
    if (!reply) {
        emit errorOccurred();
        return;
    }

    bool parsed = false;
    // first item is current weather, then forecast data
    QList<WeatherInfo> weatherData;
    if (!reply->error()) {
        const QJsonDocument document = QJsonDocument::fromJson(reply->readAll());
        const QJsonObject documentObject = document.object();

        // utc offset
        const QJsonValue offsetVal = documentObject.value("utc_offset_seconds"_L1);
        const qint64 utcOffset = !offsetVal.isUndefined() ? offsetVal.toInteger() : -1;

        // current weather
        WeatherInfo currentWeather;
        const QJsonObject currWeatherObj = documentObject.value("current_weather"_L1).toObject();
        const QJsonValue currTemp = currWeatherObj.value("temperature"_L1);
        if (currTemp.isDouble())
            currentWeather.m_temperature = niceTemperatureString(currTemp.toDouble());
        const QJsonValue weatherCode = currWeatherObj.value("weathercode"_L1);
        if (weatherCode.isDouble()) {
            const int code = weatherCode.toInt(-1);
            if (code >= 0) {
                currentWeather.m_weatherIconId = weatherCodeToIcon(code);
                currentWeather.m_weatherDescription = weatherCodeToDescription(code);
            }
        }
        if (!currentWeather.m_temperature.isEmpty()
                && !currentWeather.m_weatherIconId.isEmpty()
                && !currentWeather.m_weatherDescription.isEmpty()) {
            weatherData.append(currentWeather);
        } else {
            qCDebug(requestsLog) << "Failed to extract current weather";
        }

        // daily weather
        const QJsonObject dailyData = documentObject.value("daily"_L1).toObject();
        const QJsonArray days = dailyData.value("time"_L1).toArray();
        const QJsonArray weatherCodes = dailyData.value("weathercode"_L1).toArray();
        const QJsonArray maxTemperatures = dailyData.value("temperature_2m_max"_L1).toArray();
        const QJsonArray minTemperatures = dailyData.value("temperature_2m_min"_L1).toArray();
        if (days.size() >= kMaxEntries && weatherCodes.size() >= kMaxEntries
                && maxTemperatures.size() >= kMaxEntries
                && minTemperatures.size() >= kMaxEntries) {
            for (qsizetype i = 0; i < kMaxEntries; ++i) {
                WeatherInfo info;
                const qint64 unixTime = days.at(i).toInteger(-1);
                if (unixTime > 0) {
                    const QDateTime date = QDateTime::fromSecsSinceEpoch(unixTime + utcOffset);
                    info.m_dayOfWeek = date.toString("ddd"_L1);
                }
                const int code = weatherCodes.at(i).toInt(-1);
                if (code >= 0)
                    info.m_weatherIconId = weatherCodeToIcon(code);
                const double minTemp = minTemperatures.at(i).toDouble();
                const double maxTemp = maxTemperatures.at(i).toDouble();
                info.m_temperature = niceTemperatureString(minTemp) + u'/'
                        + niceTemperatureString(maxTemp);

                if (!info.m_dayOfWeek.isEmpty() && !info.m_temperature.isEmpty()
                        && !info.m_weatherIconId.isEmpty()) {
                    weatherData.append(info);
                } else {
                    qCDebug(requestsLog) << "Failed to extract weather forecast";
                    break;
                }
            }
        }

        parsed = (utcOffset != -1) && (weatherData.size() == (kMaxEntries + 1));
    }

    if (parsed) {
        emit weatherInformation(location, weatherData);
    } else {
        emit errorOccurred();
        if (reply->error())
            qCDebug(requestsLog) << reply->errorString();
        else
            qCDebug(requestsLog, "Failed to parse weather JSON.");
    }

    reply->deleteLater();
}

void OpenMeteoBackend::generateWeatherRequest(const QString &city,
                                              const QGeoCoordinate &coordinate)
{
    QUrl url("https://api.open-meteo.com/v1/forecast"_L1);

    QUrlQuery query;
    query.addQueryItem("latitude"_L1, QString::number(coordinate.latitude()));
    query.addQueryItem("longitude"_L1, QString::number(coordinate.longitude()));
    query.addQueryItem("daily"_L1, "weathercode,temperature_2m_max,temperature_2m_min"_L1);
    query.addQueryItem("temperature_unit"_L1, "celsius"_L1);
    query.addQueryItem("current_weather"_L1, "true"_L1);
    query.addQueryItem("timezone"_L1, QString::fromLatin1(QTimeZone::systemTimeZoneId()));
    query.addQueryItem("timeformat"_L1, "unixtime"_L1);

    url.setQuery(query);

    // We always need to have a non-empty "city" name to display to the user.
    // Other backends provide this information, but here we just use the string
    // representation of QGeoCoordinate if the city is unknown.
    // If the city is known, we pass an empty QGeoCoordinate object, so that
    // the WeatherDataCache is populated correctly.
    const LocationInfo info = city.isEmpty()
            ? LocationInfo{coordinate.toString(QGeoCoordinate::DegreesMinutesWithHemisphere),
                           coordinate}
            : LocationInfo{city, QGeoCoordinate()};

    QNetworkReply *reply = m_networkManager->get(QNetworkRequest(url));
    connect(reply, &QNetworkReply::finished, this, [this, reply, info]() {
        handleWeatherForecastReply(reply, info);
    });
}
