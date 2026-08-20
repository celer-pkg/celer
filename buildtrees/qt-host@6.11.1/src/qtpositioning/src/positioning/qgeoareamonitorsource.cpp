// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include <QtCore/private/qobject_p.h>
#include <QGeoAreaMonitorSource>
#include "qgeopositioninfosourcefactory.h"
#include "qgeopositioninfosource_p.h"

/*!
    \class QGeoAreaMonitorSource
    \inmodule QtPositioning
    \ingroup QtPositioning-positioning
    \since 5.2

    \brief The QGeoAreaMonitorSource class enables the detection of proximity
    changes for a specified set of coordinates.

    A QGeoAreaMonitorSource emits signals when the current position is in
    range, or has moved out of range, of a specified area.
    Each area is specified by a \l QGeoAreaMonitorInfo object.
    For example:

    \snippet cpp/cppqml.cpp BigBen

    \c QGeoAreaMonitorSource follows a singleton pattern. Each instance of
    the class with the same \l sourceName() shares the same area monitoring backend.
    If a new \l QGeoAreaMonitorInfo object is added via \l startMonitoring()
    or \l requestUpdate() it can be retrieved by another instance of this class
    (provided that they are sourced from the same area monitor provider plug-in).
    The same singleton pattern applies to the \l QGeoPositionInfoSource instance
    used by this class. The following code snippet emphasizes the behavior:

    \code
        QGeoAreaMonitorSource *s1 = QGeoAreaMonitorSource::createSource("blah", this);
        QGeoAreaMonitorSource *s2 = QGeoAreaMonitorSource::createSource("blah", this);
        QVERIFY(s1->positionInfoSource() == s2->positionInfoSource);
    \endcode
*/

QT_BEGIN_NAMESPACE



class QGeoAreaMonitorSourcePrivate : public QObjectPrivate
{
public:
    QGeoPositionInfoSource *source;
    QString providerName;
};

/*!
    \enum QGeoAreaMonitorSource::Error
    Defines the types of positioning methods.

    The Error enumeration represents the errors which can occur.

    \value AccessError The connection setup to the remote area monitoring backend failed because the
        application lacked the required privileges.
    \value InsufficientPositionInfo The area monitoring source could not retrieve a location fix or
        the accuracy of the fix is not high enough to provide an effective area monitoring.
    \value NoError No error has occurred.
    \value UnknownSourceError An unidentified error occurred.
*/

/*!
    \enum QGeoAreaMonitorSource::AreaMonitorFeature
    Defines the types of area monitoring capabilities.

    \value PersistentAreaMonitorFeature QGeoAreaMonitorInfo instances can be made persistent.
    A persistent monitor continues to be active even when the application managing the monitor is
    not running.
    \value AnyAreaMonitorFeature Matches all possible area monitoring features.
*/

/*!
    \fn virtual AreaMonitoringFeatures QGeoAreaMonitorSource::supportedAreaMonitorFeatures() const = 0;

    Returns the area monitoring features available to this source.
*/

/*!
     \fn virtual QGeoAreaMonitorSource::Error QGeoAreaMonitorSource::error() const

     Returns the type of error that last occurred.

    \note Since Qt6 the last error is always reset when calling
    startMonitoring() or requestUpdate().
*/

/*!
    Creates a monitor source with the given \a parent.
*/
QGeoAreaMonitorSource::QGeoAreaMonitorSource(QObject *parent)
        : QObject(*new QGeoAreaMonitorSourcePrivate, parent)
{
    Q_D(QGeoAreaMonitorSource);
    d->source = nullptr;
}

/*!
    Destroys the monitor source.
*/
QGeoAreaMonitorSource::~QGeoAreaMonitorSource()
{
}

/*!
    Creates and returns a monitor source with the given \a parent that
    monitors areas using resources on the underlying system.

    Returns \c nullptr if the system has no support for position monitoring.
*/
QGeoAreaMonitorSource *QGeoAreaMonitorSource::createDefaultSource(QObject *parent)
{
    const QList<QCborMap> plugins = QGeoPositionInfoSourcePrivate::pluginsSorted();
    for (const QCborMap &obj : plugins) {
        if (obj.value(QStringLiteral("Monitor")).isBool()
                && obj.value(QStringLiteral("Monitor")).toBool())
        {
            QGeoAreaMonitorSource *s = nullptr;
            auto factory = QGeoPositionInfoSourcePrivate::loadFactory(obj);
            if (factory)
                s = factory->areaMonitor(parent, QVariantMap());
            if (s)
                s->d_func()->providerName = obj.value(QStringLiteral("Provider")).toString();
            return s;
        }
    }
    return nullptr;
}

/*!
    Creates and returns a monitor source with the given \a parent,
    by loading the plugin named \a sourceName.

    Returns \c nullptr if the plugin cannot be found.
*/
QGeoAreaMonitorSource *QGeoAreaMonitorSource::createSource(const QString &sourceName, QObject *parent)
{
    auto plugins = QGeoPositionInfoSourcePrivate::plugins();
    if (plugins.contains(sourceName)) {
        const auto metaData = plugins.value(sourceName);
        QGeoAreaMonitorSource *s = nullptr;
        auto factory = QGeoPositionInfoSourcePrivate::loadFactory(metaData);
        if (factory)
            s = factory->areaMonitor(parent, QVariantMap());
        if (s)
            s->d_func()->providerName = metaData.value(QStringLiteral("Provider")).toString();
        return s;
    }

    return nullptr;
}

/*!
    Returns a list of available monitor plugins, including the default system
    backend if one is available.
*/
QStringList QGeoAreaMonitorSource::availableSources()
{
    QStringList plugins;
    const auto meta = QGeoPositionInfoSourcePrivate::plugins();
    for (auto it = meta.cbegin(), end = meta.cend(); it != end; ++it) {
        if (it.value().value(QStringLiteral("Monitor")).isBool()
                && it.value().value(QStringLiteral("Monitor")).toBool()) {
            plugins << it.key();
        }
    }

    return plugins;
}

/*!
    Returns the unique name of the area monitor source implementation in use.

    This is the same name that can be passed to createSource() in order to
    create a new instance of a particular area monitor source implementation.
*/
QString QGeoAreaMonitorSource::sourceName() const
{
    Q_D(const QGeoAreaMonitorSource);
    return d->providerName;
}

/*!
    \since 6.2

    Sets the backend-specific property named \a name to \a value.
    Returns \c true on success, otherwise returns \c false.
    Backend-specific properties can be used to configure the area monitoring
    subsystem behavior at runtime.

    \sa backendProperty()
*/
bool QGeoAreaMonitorSource::setBackendProperty(const QString &name, const QVariant &value)
{
    Q_UNUSED(name);
    Q_UNUSED(value);
    return false;
}

/*!
    \since 6.2

    Returns the value of the backend-specific property named \a name,
    if present. Otherwise the returned value will be invalid.

    \sa setBackendProperty()
*/
QVariant QGeoAreaMonitorSource::backendProperty(const QString &name) const
{
    Q_UNUSED(name);
    return QVariant();
}

/*!
    Returns the current QGeoPositionInfoSource used by this QGeoAreaMonitorSource
    object. The function will return \l QGeoPositionInfoSource::createDefaultSource()
    if no other object has been set.

    The function returns \c nullptr if not even a default QGeoPositionInfoSource
    exists.

    Any usage of the returned \l QGeoPositionInfoSource instance should account
    for the fact that it may reside in a different thread.

    \sa QGeoPositionInfoSource, setPositionInfoSource()
*/
QGeoPositionInfoSource* QGeoAreaMonitorSource::positionInfoSource() const
{
    Q_D(const QGeoAreaMonitorSource);
    return d->source;
}

/*!
    Sets the new \l QGeoPositionInfoSource to be used by this QGeoAreaMonitorSource object.
    The area monitoring backend becomes the new QObject parent for \a newSource.
    The previous \l QGeoPositionInfoSource object will be deleted. All QGeoAreaMonitorSource
    instances based on the same \l sourceName() share the same QGeoPositionInfoSource
    instance.

    This may be useful when it is desirable to manipulate the positioning system
    used by the area monitoring engine.

    Note that ownership must be taken care of by subclasses of QGeoAreaMonitorSource.
    Due to the singleton pattern behind this class \a newSource may be moved to a
    new thread.

    \sa positionInfoSource()
 */
void QGeoAreaMonitorSource::setPositionInfoSource(QGeoPositionInfoSource *newSource)
{
    Q_D(QGeoAreaMonitorSource);
    d->source = newSource;
}


/*!
    \fn virtual bool QGeoAreaMonitorSource::startMonitoring(const QGeoAreaMonitorInfo &monitor)

    Returns \c true if the monitoring of \a monitor could be successfully started; otherwise
    returns \c false. A reason for not being able to start monitoring could be the unavailability
    of an appropriate default position info source while no alternative QGeoPositionInfoSource
    has been set via \l setPositionInfoSource().

    If \a monitor is already active, the existing monitor object will be replaced by the new \a monitor reference.
    The identification of QGeoAreaMonitorInfo instances happens via \l QGeoAreaMonitorInfo::identifier().
    Therefore this function can also be used to update active monitors.

    If \a monitor has an expiry date that has been passed this function returns false. Calling
    this function for an already via \l requestUpdate() registered single shot monitor
    switches the monitor to a permanent monitoring mode.

    Requesting persistent monitoring on a QGeoAreaMonitorSource instance fails if the area monitoring
    backend doesn't support \l QGeoAreaMonitorSource::PersistentAreaMonitorFeature.

    \note Since Qt6 this method always resets the last error to
    \l {QGeoAreaMonitorSource::}{NoError} before starting monitoring.

    \sa stopMonitoring()
*/

/*!
    \fn virtual bool QGeoAreaMonitorSource::requestUpdate(const QGeoAreaMonitorInfo &monitor, const char *signal)

    Enables single shot area monitoring. Area monitoring for \a monitor will be performed
    until this QGeoAreaMonitorSource instance emits \a signal for the first time. Once
    the signal was emitted, \a monitor is automatically removed from the list of
    \l activeMonitors(). If \a monitor is invalid or has an expiry date that has
    been passed, this function returns \c false.

    \code
        QGeoAreaMonitor singleShotMonitor;
        QGeoAreaMonitorSource * source = QGeoAreaMonitorSource::createDefaultSource(this);
        //...
        bool ret = source->requestUpdate(singleShotMonitor,
                              SIGNAL(areaExited(QGeoAreaMonitor,QGeoPositionInfo)));
    \endcode

    The above \c singleShotMonitor object will cease to send updates once the \l areaExited() signal
    was emitted for the first time. Until this point in time any other signal may be emitted
    zero or more times depending on the area context.

    It is not possible to simultanously request updates for more than one signal of the same monitor object.
    The last call to this function determines the signal upon which the updates cease to continue.
    At this stage only the \l areaEntered() and \l areaExited() signals can be used to
    terminate the monitoring process.

    Requesting persistent monitoring on a QGeoAreaMonitorSource instance fails if the area monitoring
    backend doesn't support \l QGeoAreaMonitorSource::PersistentAreaMonitorFeature.

    If \a monitor was already registered via \l startMonitoring() it is converted to a single
    shot behavior.

    \note Since Qt6 this method always resets the last error to
    \l {QGeoAreaMonitorSource::}{NoError} before starting monitoring.

    \sa startMonitoring(), stopMonitoring()
 */

/*!
    \fn virtual bool QGeoAreaMonitorSource::stopMonitoring(const QGeoAreaMonitorInfo &monitor)

    Returns true if \a monitor was successfully removed from the list of \l activeMonitors();
    otherwise returns false. This behavior is independent on whether \a monitor was registered
    via \l startMonitoring() or \l requestUpdate().
*/

/*!
    \fn virtual QList<QGeoAreaMonitorInfo> QGeoAreaMonitorSource::activeMonitors() const

    Returns the list of all active monitors known to the QGeoAreaMonitorSource object.

    An active monitor was started via startMonitoring(). For every active
    monitor the source object will emit the required signals, such as
    areaEntered() or areaExited(). Multiple \l QGeoAreaMonitorSource instances
    within the same application share the same active monitor objects.

    Unless an active QGeoAreaMonitorInfo \l {QGeoAreaMonitorInfo::isPersistent()}{isPersistent()} an active QGeoAreaMonitorInfo
    will be stopped once the current application terminates.
*/

/*!
    \fn virtual QList<QGeoAreaMonitorInfo> QGeoAreaMonitorSource::activeMonitors(const QGeoShape &lookupArea) const

    Returns the list of all active monitors known to the QGeoAreaMonitorSource object whose
    center lies within \a lookupArea. If \a lookupArea is empty the returned list will be empty.

    An active monitor was started via startMonitoring(). For every active
    monitor the source object will emit the required signals, such as
    areaEntered() or areaExited(). Multiple \l QGeoAreaMonitorSource instances
    within the same application share the same active monitor objects.

    Unless an active QGeoAreaMonitorInfo \l {QGeoAreaMonitorInfo::isPersistent()}{isPersistent()} an active QGeoAreaMonitorInfo
    will be stopped once the current application terminates.

    \sa QGeoShape
*/


/*!
    \fn void QGeoAreaMonitorSource::monitorExpired(const QGeoAreaMonitorInfo &monitor)

    Emitted when \a monitor has expired. An expired area monitor is automatically
    removed from the list of \l activeMonitors().

    \sa activeMonitors()
*/

/*!
    \fn void QGeoAreaMonitorSource::areaEntered(const QGeoAreaMonitorInfo &monitor, const QGeoPositionInfo &update)

    Emitted when the current position has moved from a position outside of the active \a monitor
    to a position within the monitored area.

    The \a update holds the new position.
*/

/*!
    \fn void QGeoAreaMonitorSource::areaExited(const QGeoAreaMonitorInfo &monitor, const QGeoPositionInfo &update)

    Emitted when the current position has moved from a position within the active \a monitor
    to a position outside the monitored area.

    The \a update holds the new position.
*/

/*!
    \fn void QGeoAreaMonitorSource::errorOccurred(QGeoAreaMonitorSource::Error areaMonitoringError)

    This signal is emitted after an error occurred. The \a areaMonitoringError
    parameter describes the type of error that occurred.

*/

QT_END_NAMESPACE

#include "moc_qgeoareamonitorsource.cpp"
