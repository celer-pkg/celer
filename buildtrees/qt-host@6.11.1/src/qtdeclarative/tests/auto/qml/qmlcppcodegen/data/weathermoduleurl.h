// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef WEATHERMODELURL_H
#define WEATHERMODELURL_H

#include <QtQml/qqmlregistration.h>
#include <QtCore/qobject.h>

class WeatherModelUrl
{
    Q_GADGET
    QML_STRUCTURED_VALUE
    QML_VALUE_TYPE(weatherModelUrl)
    Q_PROPERTY(qsizetype timeIndex READ timeIndex CONSTANT)
    Q_PROPERTY(QStringList strings READ strings WRITE setStrings)

public:
    WeatherModelUrl() : m_timeIndex(-1) {}
    WeatherModelUrl(qsizetype timeIdx) : m_timeIndex(timeIdx) {}
    Q_INVOKABLE WeatherModelUrl(const WeatherModelUrl &) = default;

    ~WeatherModelUrl() = default;
    WeatherModelUrl(WeatherModelUrl &&) = default;
    WeatherModelUrl &operator=(const WeatherModelUrl &) = default;
    WeatherModelUrl &operator=(WeatherModelUrl &&) = default;

    qsizetype timeIndex() const { return m_timeIndex; }

    QStringList strings() const { return m_strings; }
    void setStrings(const QStringList &newStrings)
    {
        if (m_strings != newStrings)
            m_strings = newStrings;
    }

private:
    friend bool operator==(const WeatherModelUrl &a, const WeatherModelUrl &b)
    {
        return a.m_timeIndex == b.m_timeIndex;
    }

    friend bool operator!=(const WeatherModelUrl &a, const WeatherModelUrl &b)
    {
        return !(a == b);
    }

    qsizetype m_timeIndex;
    QStringList m_strings;
};

class WeatherModelUrlDerived : public WeatherModelUrl
{
    Q_GADGET
    QML_STRUCTURED_VALUE
    QML_VALUE_TYPE(weatherModelUrlDerived)
};

class WeatherModelUrlExtended : public QObject
{
    Q_OBJECT
public:
    WeatherModelUrlExtended(QObject *parent = nullptr) : QObject(parent) {}
};

class WeatherModelUrlUtils : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    // Hack: Add an extension to mark the singleton as not shadowable
    // TODO: Remove this when we either can assign QVariant to WeatherModuleUrl or
    //       when we can mark the url() method as final or when moc can recognize
    //       a whole class as final and we can draw conclusions from that.
    QML_EXTENDED(WeatherModelUrlExtended)

public:
    WeatherModelUrlUtils(QObject *parent = nullptr) : QObject(parent) {}

    Q_INVOKABLE static WeatherModelUrl url(int timeIdx)
    {
        return WeatherModelUrl(timeIdx);
    }
};

#endif // WEATHERMODELURL_H
