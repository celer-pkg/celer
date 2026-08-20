// Copyright (C) 2017 Ford Motor Company
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef TEMPERATURE_H
#define TEMPERATURE_H

#include <QSharedPointer>
#include <QString>

class Temperature
{
public:
    typedef QSharedPointer<Temperature> Ptr;

    Temperature() : _value(0) {}
    Temperature(double value, const QString &unit) : _value(value), _unit(unit) {}

    void setValue(double arg) { _value = arg; }
    double value() const { return _value; }

    void setUnit(const QString &arg) { _unit = arg; }
    QString unit() const { return _unit; }

private:
    double _value;
    QString _unit;
};

inline bool operator==(const Temperature &lhs, const Temperature &rhs)
{
    return lhs.unit() == rhs.unit() &&
            lhs.value() == rhs.value();
}

inline bool operator!=(const Temperature &lhs, const Temperature &rhs)
{
    return !(lhs == rhs);
}

inline QDataStream &operator<<(QDataStream &out, const Temperature &temperature)
{
    out << temperature.value();
    out << temperature.unit();
    return out;
}

inline QDataStream &operator>>(QDataStream &in, Temperature &temperature)
{
    double value;
    in >> value;
    temperature.setValue(value);

    QString unit;
    in >> unit;
    temperature.setUnit(unit);
    return in;
}

inline QDataStream &operator>>(QDataStream &out, const Temperature::Ptr& temperaturePtr)
{
    out << *temperaturePtr;
    return out;
}

inline QDataStream &operator>>(QDataStream &in, Temperature::Ptr& temperaturePtr)
{
    Temperature *temperature = new Temperature;
    in >> *temperature;
    temperaturePtr.reset(temperature);
    return in;
}

Q_DECLARE_METATYPE(Temperature);
Q_DECLARE_METATYPE(Temperature::Ptr);

#endif
