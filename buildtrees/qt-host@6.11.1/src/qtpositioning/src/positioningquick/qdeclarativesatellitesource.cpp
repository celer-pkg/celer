// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qdeclarativesatellitesource_p.h"

QT_BEGIN_NAMESPACE

/*!
    \qmltype SatelliteSource
    \inqmlmodule QtPositioning
    \since 6.5
    \brief The SatelliteSource class provides the satellite information.

    The SatelliteSource class provides information about satellites in use and
    satellites in view. This class is a QML representation of
    \l QGeoSatelliteInfoSource.

    Like its C++ equivalent, the class supports different plugins. Use the
    \l name property to specify the name of the plugin to be used, and provide
    \l {PluginParameter}s, if required. If the \l name property is not set,
    a default plugin will be used. See \l {Qt Positioning Plugins} for more
    information on the available plugins.

    Use the \l valid property to check the SatelliteSource state.

    Use the \l updateInterval property to indicate how often your application
    wants to receive the satellite information updates. The \l start(),
    \l stop() and \l update() methods can be used to control the operation
    of the SatelliteSource, as well as the \l active property, which when set
    is equivalent to calling \l start() or \l stop().

    When the SatelliteSource is active, satellite information updates can
    be retrieved using the \l satellitesInView and \l satellitesInUse
    properties.

    If an error happens during satellite information updates, use the
    \l sourceError property to get the actual error code.

    \section2 Example Usage

    The following example shows a SatelliteSource which is using the
    \l {Qt Positioning NMEA plugin}{NMEA} plugin to receive satellite
    information updates every second and print the amount of satellites
    in view and satellites in use to the console.

    \qml
    SatelliteSource {
        id: source
        name: "nmea"
        active: true
        updateInterval: 1000
        PluginParameter { name: "nmea.source"; value: "serial:/dev/ttyACM0" }

        onSatellitesInUseChanged: {
            console.log("Satellites in use:", source.satellitesInUse.length)
        }
        onSatellitesInViewChanged: {
            console.log("Satellites in view:", source.satellitesInView.length)
        }
    }
    \endqml

    \sa QGeoSatelliteInfoSource, PluginParameter, geoSatelliteInfo
*/

QDeclarativeSatelliteSource::QDeclarativeSatelliteSource()
    : m_active(0), m_componentComplete(0), m_parametersInitialized(0),
      m_startRequested(0), m_defaultSourceUsed(0), m_regularUpdates(0),
      m_singleUpdate(0), m_singleUpdateRequested(0)
{
}

QDeclarativeSatelliteSource::~QDeclarativeSatelliteSource()
{
    if (m_source)
        m_source->disconnect(this);
}

/*!
    \qmlproperty bool SatelliteSource::active

    This property indicates whether the satellite source is active.
    Setting this property to \c false equals calling \l stop, and
    setting this property to \c true equals calling \l start.

    \sa start, stop, update
*/
bool QDeclarativeSatelliteSource::isActive() const
{
    return m_active;
}

/*!
    \qmlproperty bool SatelliteSource::valid
    \readonly

    This property is \c true if the SatelliteSource object has acquired a valid
    backend plugin to provide data, and \c false otherwise.

    Applications should check this property to determine whether providing
    satellite information is available and enabled on the runtime platform,
    and react accordingly.
*/
bool QDeclarativeSatelliteSource::isValid() const
{
    return m_source != nullptr;
}

/*!
    \qmlproperty int SatelliteSource::updateInterval

    This property holds the desired interval between updates in milliseconds.
*/
int QDeclarativeSatelliteSource::updateInterval() const
{
    return m_source ? m_source->updateInterval() : m_updateInterval;
}

/*!
    \qmlproperty enumeration SatelliteSource::sourceError
    \readonly

    This property holds the error which last occurred with the backend data
    provider.

    \value SatelliteSource.AccessError
        The connection setup to the satellite
        backend failed because the application lacked the required
        privileges.
    \value SatelliteSource.ClosedError
        The satellite backend closed the
        connection, which happens for example in case the user is switching
        location services to off.
    \value SatelliteSource.NoError
        No error has occurred.
    \value SatelliteSource.UnknownSourceError
        An unidentified error occurred.
    \value SatelliteSource.UpdateTimeoutError
        The satellite information could not be retrieved within the specified timeout.
*/
QDeclarativeSatelliteSource::SourceError QDeclarativeSatelliteSource::sourceError() const
{
    return m_error;
}

/*!
    \qmlproperty string SatelliteSource::name

    This property holds the unique internal name for the plugin currently
    providing satellite information.

    Setting the property causes the SatelliteSource to use a particular
    backend plugin. If the SatelliteSource is active at the time that the name
    property is changed, it will become inactive. If the specified backend
    cannot be loaded the satellite source will become invalid.

    Changing the name property may cause the \l updateInterval property
    to change as well.
*/
QString QDeclarativeSatelliteSource::name() const
{
    return m_source ? m_source->sourceName() : m_name;
}

/*!
    \qmlproperty list<PluginParameter> SatelliteSource::parameters
    \readonly
    \qmldefault

    This property holds the list of plugin parameters.

    \sa PluginParameter
*/
QQmlListProperty<QDeclarativePluginParameter> QDeclarativeSatelliteSource::parameters()
{
    return QQmlListProperty<QDeclarativePluginParameter>(this, nullptr,
                                                         parameter_append,
                                                         parameter_count,
                                                         parameter_at,
                                                         parameter_clear);
}

/*!
    \qmlproperty list<geoSatelliteInfo> SatelliteSource::satellitesInUse
    \readonly

    This property holds the list of satellites that are currently in use.
    These are the satellites that are used to get a "fix" - that
    is, those used to determine the current position.
*/
QList<QGeoSatelliteInfo> QDeclarativeSatelliteSource::satellitesInUse() const
{
    return m_satellitesInUse;
}

/*!
    \qmlproperty list<geoSatelliteInfo> SatelliteSource::satellitesInView
    \readonly

    This property holds the list of satellites that are currently in view.
*/
QList<QGeoSatelliteInfo> QDeclarativeSatelliteSource::satellitesInView() const
{
    return m_satellitesInView;
}

void QDeclarativeSatelliteSource::setUpdateInterval(int updateInterval)
{
    if (m_updateInterval == updateInterval)
        return;

    const auto oldInterval = m_updateInterval;

    if (m_source) {
        m_source->setUpdateInterval(updateInterval);
        // The above call can set some other interval, for example if desired
        // updateInterval is less than minimum supported update interval. So
        // we need to read the value back explicitly.
        m_updateInterval = m_source->updateInterval();
    } else {
        m_updateInterval = updateInterval;
    }
    if (oldInterval != m_updateInterval)
        emit updateIntervalChanged();
}

void QDeclarativeSatelliteSource::setActive(bool active)
{
    if (active == m_active)
        return;

    if (active)
        start();
    else
        stop();
}

void QDeclarativeSatelliteSource::setName(const QString &name)
{
    if ((m_name == name) || (name.isEmpty() && m_defaultSourceUsed))
        return;

    if (m_componentComplete && m_parametersInitialized) {
        createSource(name); // it will update name and emit, if needed
    } else {
        m_name = name;
        emit nameChanged();
    }
}

void QDeclarativeSatelliteSource::componentComplete()
{
    m_componentComplete = true;
    m_parametersInitialized = true;
    for (QDeclarativePluginParameter *p: std::as_const(m_parameters)) {
        if (!p->isInitialized()) {
            m_parametersInitialized = false;
            connect(p, &QDeclarativePluginParameter::initialized,
                    this, &QDeclarativeSatelliteSource::onParameterInitialized,
                    Qt::SingleShotConnection);
        }
    }

    if (m_parametersInitialized)
        createSource(m_name);
}

/*!
    \qmlmethod bool SatelliteSource::setBackendProperty(string name, var value)

    Sets the backend-specific property named \a name to \a value.
    Returns true on success, false otherwise, including if called on an
    uninitialized SatelliteSource.
*/
bool QDeclarativeSatelliteSource::setBackendProperty(const QString &name, const QVariant &value)
{
    if (m_source)
        return m_source->setBackendProperty(name, value);
    return false;
}

/*!
    \qmlmethod var SatelliteSource::backendProperty(string name)

    Returns the value of the backend-specific property named \a name, if
    present. Otherwise, including if called on an uninitialized SatelliteSource,
    the return value will be invalid.
*/
QVariant QDeclarativeSatelliteSource::backendProperty(const QString &name) const
{
    return m_source ? m_source->backendProperty(name) : QVariant{};
}

/*!
    \qmlmethod void SatelliteSource::update(int timeout = 0)

    A convenience method to request a single update from the satellite source.
    If there is no source available, this method has no effect.

    If the satellite source is not active, it will be activated for as
    long as it takes to receive an update, or until the request times
    out. The request timeout period is plugin-specific.

    The \a timeout is specified in milliseconds. If the \a timeout is zero
    (the default value), it defaults to a reasonable timeout period as
    appropriate for the source.

    \sa start(), stop(), active
*/
void QDeclarativeSatelliteSource::update(int timeout)
{
    if (m_componentComplete && m_parametersInitialized) {
        executeSingleUpdate(timeout);
    } else {
        m_singleUpdateDesiredTimeout = timeout;
        m_singleUpdateRequested = true;
    }
}

/*!
    \qmlmethod void SatelliteSource::start()

    Requests updates from the satellite source. Uses \l updateInterval if set,
    default interval otherwise. If there is no source available, this method
    has no effect.

    \sa stop(), update(), active
*/
void QDeclarativeSatelliteSource::start()
{
    if (m_componentComplete && m_parametersInitialized)
        executeStart();
    else
        m_startRequested = true;
}

/*!
    \qmlmethod void SatelliteSource::stop()

    Stops updates from the satellite source. If there is no source available or
    it is not active, this method has no effect.

    \sa start(), update(), active
*/
void QDeclarativeSatelliteSource::stop()
{
    if (m_source) {
        m_source->stopUpdates();
        m_regularUpdates = false;

        if (m_active && !m_singleUpdate) {
            m_active = false;
            emit activeChanged();
        }
    } else {
        m_startRequested = false;
    }
}

void QDeclarativeSatelliteSource::sourceErrorReceived(const QGeoSatelliteInfoSource::Error error)
{
    const auto oldError = m_error;
    m_error = static_cast<SourceError>(error);
    if (m_error != oldError)
        emit sourceErrorChanged();

    // if an error occurred during single update, the update is stopped, so we
    // need to change the active state.
    if (m_singleUpdate) {
        m_singleUpdate = false;
        if (m_active && !m_regularUpdates) {
            m_active = false;
            emit activeChanged();
        }
    }
}

void QDeclarativeSatelliteSource::onParameterInitialized()
{
    m_parametersInitialized = true;
    for (QDeclarativePluginParameter *p: std::as_const(m_parameters)) {
        if (!p->isInitialized()) {
            m_parametersInitialized = false;
            break;
        }
    }

    // m_componentComplete == true here
    if (m_parametersInitialized)
        createSource(m_name);
}

void QDeclarativeSatelliteSource::satellitesInViewUpdateReceived(const QList<QGeoSatelliteInfo> &satellites)
{
    m_satellitesInView = satellites;
    emit satellitesInViewChanged();
    handleSingleUpdateReceived();
}

void QDeclarativeSatelliteSource::satellitesInUseUpdateReceived(const QList<QGeoSatelliteInfo> &satellites)
{
    m_satellitesInUse = satellites;
    emit satellitesInUseChanged();
    handleSingleUpdateReceived();
}

QVariantMap QDeclarativeSatelliteSource::parameterMap() const
{
    QVariantMap map;
    for (const auto *parameter : std::as_const(m_parameters))
        map.insert(parameter->name(), parameter->value());
    return map;
}

void QDeclarativeSatelliteSource::createSource(const QString &newName)
{
    if (m_source && m_source->sourceName() == newName)
        return;

    const auto oldName = name();
    const bool oldIsValid = isValid();
    const bool oldActive = isActive();
    const auto oldUpdateInterval = updateInterval();

    if (m_source) {
        m_source->disconnect(this);
        m_source->stopUpdates();
        m_source.reset(nullptr);
        m_active = false;
    }

    if (!newName.isEmpty()) {
        m_source.reset(QGeoSatelliteInfoSource::createSource(newName, parameterMap(), nullptr));
        m_defaultSourceUsed = false;
    } else {
        m_source.reset(QGeoSatelliteInfoSource::createDefaultSource(parameterMap(), nullptr));
        m_defaultSourceUsed = true;
    }

    if (m_source) {
        connect(m_source.get(), &QGeoSatelliteInfoSource::errorOccurred,
                this, &QDeclarativeSatelliteSource::sourceErrorReceived);
        connect(m_source.get(), &QGeoSatelliteInfoSource::satellitesInViewUpdated,
                this, &QDeclarativeSatelliteSource::satellitesInViewUpdateReceived);
        connect(m_source.get(), &QGeoSatelliteInfoSource::satellitesInUseUpdated,
                this, &QDeclarativeSatelliteSource::satellitesInUseUpdateReceived);

        m_name = m_source->sourceName();
        m_source->setUpdateInterval(m_updateInterval);
        m_updateInterval = m_source->updateInterval();
    } else {
        m_name = newName;
        m_defaultSourceUsed = false;
    }

    if (oldName != name())
        emit nameChanged();

    if (oldIsValid != isValid())
        emit validityChanged();

    if (oldActive != isActive())
        emit activeChanged();

    if (oldUpdateInterval != updateInterval())
        emit updateIntervalChanged();

    if (m_startRequested) {
        m_startRequested = false;
        executeStart();
    }
    if (m_singleUpdateRequested) {
        m_singleUpdateRequested = false;
        executeSingleUpdate(m_singleUpdateDesiredTimeout);
    }
}

void QDeclarativeSatelliteSource::handleSingleUpdateReceived()
{
    if (m_singleUpdate) {
        m_singleUpdate = false;
        if (m_active && !m_regularUpdates) {
            m_active = false;
            emit activeChanged();
        }
    }
}

void QDeclarativeSatelliteSource::executeStart()
{
    if (m_source) {
        m_regularUpdates = true;
        if (!m_active) {
            m_active = true;
            emit activeChanged();
        }
        m_source->startUpdates();
    }
}

void QDeclarativeSatelliteSource::executeSingleUpdate(int timeout)
{
    if (m_source) {
        m_singleUpdate = true;
        if (!m_active) {
            m_active = true;
            emit activeChanged();
        }
        m_source->requestUpdate(timeout);
    }
}

void QDeclarativeSatelliteSource::parameter_append(PluginParameterProperty *prop,
                                                   QDeclarativePluginParameter *parameter)
{
    auto *src = static_cast<QDeclarativeSatelliteSource *>(prop->object);
    src->m_parameters.append(parameter);
}

qsizetype QDeclarativeSatelliteSource::parameter_count(PluginParameterProperty *prop)
{
    return static_cast<QDeclarativeSatelliteSource *>(prop->object)->m_parameters.size();
}

QDeclarativePluginParameter *
QDeclarativeSatelliteSource::parameter_at(PluginParameterProperty *prop, qsizetype index)
{
    return static_cast<QDeclarativeSatelliteSource *>(prop->object)->m_parameters[index];
}

void QDeclarativeSatelliteSource::parameter_clear(PluginParameterProperty *prop)
{
    auto *src = static_cast<QDeclarativeSatelliteSource *>(prop->object);
    src->m_parameters.clear();
}

QT_END_NAMESPACE
