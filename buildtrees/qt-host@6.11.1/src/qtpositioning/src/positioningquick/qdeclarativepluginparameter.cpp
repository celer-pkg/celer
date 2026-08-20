// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qdeclarativepluginparameter_p.h"

QT_BEGIN_NAMESPACE

/*!
    \qmltype PluginParameter
    \inqmlmodule QtPositioning
    \ingroup qml-QtPositioning5-common
    \since QtPositioning 5.14

    \brief The PluginParameter type describes a parameter for a
    \omit
    plugin, either geoservice \l Plugin, or
    \endomit
    position plugin.

    The PluginParameter object is used to provide a parameter of some kind
    to a plugin. Typically, these parameters contain details like an application
    token for access to a service, or a proxy server to use for network access,
    or the serial port to which a serial GPS receiver is connected.

    To set such a parameter, declare a PluginParameter inside an element that
    accepts plugin parameters as configuration objects, such as a
    \omit
    \l Plugin object, or a
    \endomit
    \l PositionSource object, and set values for its \l{name} and \l{value}
    properties. A list of valid parameter names for each plugin is available
    from the
    \omit
    \l {Qt Location#Plugin References and Parameters}{plugin reference pages}
    for geoservice plugins, and
    \endomit
    \l {Qt Positioning plugins#Default plugins}{default plugins page} for
    position plugins.

    \section2 Example Usage

    The following example shows the instantiation of the
    \l {Qt Positioning NMEA plugin}{NMEA} plugin with the \e nmea.source
    parameter that specifies the data source.

    \code
    PositionSource {
        name: "nmea"
        PluginParameter { name: "nmea.source"; value: "serial:/dev/ttyACM0" }
    }
    \endcode
*/

/*!
    \qmlproperty string PluginParameter::name

    This property holds the name of the plugin parameter as a single formatted string.
    This property is a write-once property.
*/

/*!
    \qmlproperty QVariant PluginParameter::value

    This property holds the value of the plugin parameter which support different types of values (variant).
    This property is a write-once property.
*/

QDeclarativePluginParameter::QDeclarativePluginParameter(QObject *parent)
    : QObject(parent) {}

QDeclarativePluginParameter::~QDeclarativePluginParameter() {}

void QDeclarativePluginParameter::setName(const QString &name)
{
    if (!name_.isEmpty() || name.isEmpty())
        return;

    name_ = name;

    emit nameChanged(name_);
    if (value_.isValid())
        emit initialized();
}

QString QDeclarativePluginParameter::name() const
{
    return name_;
}

void QDeclarativePluginParameter::setValue(const QVariant &value)
{
    if (value_.isValid() || !value.isValid() || value.isNull())
        return;

    value_ = value;

    emit valueChanged(value_);
    if (!name_.isEmpty())
        emit initialized();
}

QVariant QDeclarativePluginParameter::value() const
{
    return value_;
}

bool QDeclarativePluginParameter::isInitialized() const
{
    return !name_.isEmpty() && value_.isValid();
}

QT_END_NAMESPACE

#include "moc_qdeclarativepluginparameter_p.cpp"
