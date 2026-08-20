// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qgeoareamonitor_polling.h"
#include <QtPositioning/qgeocoordinate.h>
#include <QtPositioning/qgeorectangle.h>
#include <QtPositioning/qgeocircle.h>

#include <QtCore/qhash.h>
#include <QtCore/qmetaobject.h>
#include <QtCore/qtimer.h>
#include <QtCore/qdebug.h>
#include <QtCore/qmutex.h>
#include <QtCore/qset.h>

#include <mutex>

#define UPDATE_INTERVAL_5S  5000

QT_BEGIN_NAMESPACE

typedef QHash<QString, QGeoAreaMonitorInfo> MonitorTable;

static QMetaMethod areaEnteredSignal()
{
    static QMetaMethod signal = QMetaMethod::fromSignal(&QGeoAreaMonitorPolling::areaEntered);
    return signal;
}

static QMetaMethod areaExitedSignal()
{
    static QMetaMethod signal = QMetaMethod::fromSignal(&QGeoAreaMonitorPolling::areaExited);
    return signal;
}

static QMetaMethod monitorExpiredSignal()
{
    static QMetaMethod signal = QMetaMethod::fromSignal(&QGeoAreaMonitorPolling::monitorExpired);
    return signal;
}

class QGeoAreaMonitorPollingPrivate : public QObject
{
    Q_OBJECT
public:
    QGeoAreaMonitorPollingPrivate()
    {
        nextExpiryTimer = new QTimer(this);
        nextExpiryTimer->setSingleShot(true);
        connect(nextExpiryTimer, SIGNAL(timeout()),
                this, SLOT(timeout()));
    }

    void startMonitoring(const QGeoAreaMonitorInfo &monitor)
    {
        const std::lock_guard<QRecursiveMutex> locker(mutex);

        activeMonitorAreas.insert(monitor.identifier(), monitor);
        singleShotTrigger.remove(monitor.identifier());

        checkStartStop();
        setupNextExpiryTimeout();
    }

    void requestUpdate(const QGeoAreaMonitorInfo &monitor, int signalId)
    {
        const std::lock_guard<QRecursiveMutex> locker(mutex);

        activeMonitorAreas.insert(monitor.identifier(), monitor);
        singleShotTrigger.insert(monitor.identifier(), signalId);

        checkStartStop();
        setupNextExpiryTimeout();
    }

    QGeoAreaMonitorInfo stopMonitoring(const QGeoAreaMonitorInfo &monitor)
    {
        const std::lock_guard<QRecursiveMutex> locker(mutex);

        QGeoAreaMonitorInfo mon = activeMonitorAreas.take(monitor.identifier());

        checkStartStop();
        setupNextExpiryTimeout();

        return mon;
    }

    void registerClient(QGeoAreaMonitorPolling *client)
    {
        const std::lock_guard<QRecursiveMutex> locker(mutex);

        connect(this, SIGNAL(timeout(QGeoAreaMonitorInfo)),
                client, SLOT(timeout(QGeoAreaMonitorInfo)));

        connect(this, SIGNAL(positionError(QGeoPositionInfoSource::Error)),
                client, SLOT(positionError(QGeoPositionInfoSource::Error)));

        connect(this, SIGNAL(areaEventDetected(QGeoAreaMonitorInfo,QGeoPositionInfo,bool)),
                client, SLOT(processAreaEvent(QGeoAreaMonitorInfo,QGeoPositionInfo,bool)));

        registeredClients.append(client);
    }

    void deregisterClient(QGeoAreaMonitorPolling *client)
    {
        const std::lock_guard<QRecursiveMutex> locker(mutex);

        registeredClients.removeAll(client);
        if (registeredClients.isEmpty())
            checkStartStop();
    }

    void setPositionSource(QGeoPositionInfoSource *newSource)
    {
        const std::lock_guard<QRecursiveMutex> locker(mutex);

        if (newSource == source)
            return;

        if (source)
            delete source;

        source = newSource;

        if (source) {
            source->setParent(this);
            source->moveToThread(this->thread());
            if (source->updateInterval() == 0)
                source->setUpdateInterval(UPDATE_INTERVAL_5S);
            disconnect(source, 0, 0, 0); //disconnect all
            connect(source, SIGNAL(positionUpdated(QGeoPositionInfo)),
                    this, SLOT(positionUpdated(QGeoPositionInfo)));
            connect(source, SIGNAL(errorOccurred(QGeoPositionInfoSource::Error)),
                    this, SIGNAL(positionError(QGeoPositionInfoSource::Error)));
            checkStartStop();
        }
    }

    QGeoPositionInfoSource* positionSource() const
    {
        const std::lock_guard<QRecursiveMutex> locker(mutex);
        return source;
    }

    MonitorTable activeMonitors() const
    {
        const std::lock_guard<QRecursiveMutex> locker(mutex);

        return activeMonitorAreas;
    }

    void checkStartStop()
    {
        const std::lock_guard<QRecursiveMutex> locker(mutex);

        bool signalsConnected = false;
        for (const QGeoAreaMonitorPolling *client : std::as_const(registeredClients)) {
            if (client->hasConnections()) {
                signalsConnected = true;
                break;
            }
        }

        if (signalsConnected && !activeMonitorAreas.isEmpty()) {
            if (source)
                source->startUpdates();
            else
                //translated to InsufficientPositionInfo
                emit positionError(QGeoPositionInfoSource::ClosedError);
        } else {
            if (source)
                source->stopUpdates();
        }
    }

private:
    void setupNextExpiryTimeout()
    {
        nextExpiryTimer->stop();
        activeExpiry.first = QDateTime();
        activeExpiry.second = QString();

        const auto infos = activeMonitors();
        for (const QGeoAreaMonitorInfo &info : infos) {
            if (info.expiration().isValid()) {
                if (!activeExpiry.first.isValid()) {
                    activeExpiry.first = info.expiration();
                    activeExpiry.second = info.identifier();
                    continue;
                }
                if (info.expiration() < activeExpiry.first) {
                    activeExpiry.first = info.expiration();
                    activeExpiry.second = info.identifier();
                }
            }
        }

        if (activeExpiry.first.isValid())
            nextExpiryTimer->start(QDateTime::currentDateTime().msecsTo(activeExpiry.first));
    }


    //returns true if areaEntered should be emitted
    bool processInsideArea(const QString &monitorIdent)
    {
        if (!insideArea.contains(monitorIdent)) {
            if (singleShotTrigger.value(monitorIdent, -1) == areaEnteredSignal().methodIndex()) {
                //this is the finishing singleshot event
                singleShotTrigger.remove(monitorIdent);
                activeMonitorAreas.remove(monitorIdent);
                setupNextExpiryTimeout();
            } else {
                insideArea.insert(monitorIdent);
            }
            return true;
        }

        return false;
    }

    //returns true if areaExited should be emitted
    bool processOutsideArea(const QString &monitorIdent)
    {
        if (insideArea.contains(monitorIdent)) {
            if (singleShotTrigger.value(monitorIdent, -1) == areaExitedSignal().methodIndex()) {
                //this is the finishing singleShot event
                singleShotTrigger.remove(monitorIdent);
                activeMonitorAreas.remove(monitorIdent);
                setupNextExpiryTimeout();
            } else {
                insideArea.remove(monitorIdent);
            }
            return true;
        }
        return false;
    }



Q_SIGNALS:
    void timeout(const QGeoAreaMonitorInfo &info);
    void positionError(const QGeoPositionInfoSource::Error error);
    void areaEventDetected(const QGeoAreaMonitorInfo &minfo,
                           const QGeoPositionInfo &pinfo, bool isEnteredEvent);
private Q_SLOTS:
    void timeout()
    {
        /*
         * Don't block timer firing even if monitorExpiredSignal is not connected.
         * This allows us to continue to remove the existing monitors as they expire.
         **/
        const QGeoAreaMonitorInfo info = activeMonitorAreas.take(activeExpiry.second);
        setupNextExpiryTimeout();
        emit timeout(info);

    }

    void positionUpdated(const QGeoPositionInfo &info)
    {
        const auto monInfos = activeMonitors();
        for (const QGeoAreaMonitorInfo &monInfo : monInfos) {
            const QString identifier = monInfo.identifier();
            if (monInfo.area().contains(info.coordinate())) {
                if (processInsideArea(identifier))
                    emit areaEventDetected(monInfo, info, true);
            } else {
                if (processOutsideArea(identifier))
                    emit areaEventDetected(monInfo, info, false);
            }
        }
    }

private:
    QPair<QDateTime, QString> activeExpiry;
    QHash<QString, int> singleShotTrigger;
    QTimer* nextExpiryTimer;
    QSet<QString> insideArea;

    MonitorTable activeMonitorAreas;

    QGeoPositionInfoSource* source = nullptr;
    QList<QGeoAreaMonitorPolling*> registeredClients;
    mutable QRecursiveMutex mutex;
};

Q_GLOBAL_STATIC(QGeoAreaMonitorPollingPrivate, pollingPrivate)

QGeoAreaMonitorPolling::QGeoAreaMonitorPolling(QObject *parent) : QGeoAreaMonitorSource(parent)
{
    d = pollingPrivate();
    d->registerClient(this);
    //hookup to default source if existing
    if (!positionInfoSource())
        setPositionInfoSource(QGeoPositionInfoSource::createDefaultSource(this));
}

QGeoAreaMonitorPolling::~QGeoAreaMonitorPolling()
{
    d->deregisterClient(this);
}

QGeoPositionInfoSource* QGeoAreaMonitorPolling::positionInfoSource() const
{
    return d->positionSource();
}

void QGeoAreaMonitorPolling::setPositionInfoSource(QGeoPositionInfoSource *source)
{
    d->setPositionSource(source);
}

QGeoAreaMonitorSource::Error QGeoAreaMonitorPolling::error() const
{
    return lastError;
}

bool QGeoAreaMonitorPolling::startMonitoring(const QGeoAreaMonitorInfo &monitor)
{
    if (!monitor.isValid())
        return false;

    //reject an expiry in the past
    if (monitor.expiration().isValid() &&
            (monitor.expiration() < QDateTime::currentDateTime()))
        return false;

    //don't accept persistent monitor since we don't support it
    if (monitor.isPersistent())
        return false;

    lastError = QGeoAreaMonitorSource::NoError;

    //update or insert
    d->startMonitoring(monitor);

    return true;
}

int QGeoAreaMonitorPolling::idForSignal(const char *signal)
{
    if (qstrlen(signal) < 2)
        return -1;

    const QByteArray sig = QMetaObject::normalizedSignature(signal + 1);
    const QMetaObject * const mo = metaObject();

    return mo->indexOfSignal(sig.constData());
}

bool QGeoAreaMonitorPolling::hasConnections() const
{
    // This method is internal and requires the mutex to be already locked.
    return signalConnections > 0;
}

bool QGeoAreaMonitorPolling::requestUpdate(const QGeoAreaMonitorInfo &monitor, const char *signal)
{
    if (!monitor.isValid())
        return false;
    //reject an expiry in the past
    if (monitor.expiration().isValid() &&
            (monitor.expiration() < QDateTime::currentDateTime()))
        return false;

    //don't accept persistent monitor since we don't support it
    if (monitor.isPersistent())
        return false;

    // signal is validated in idForSignal()
    const int signalId = idForSignal(signal);
    if (signalId < 0)
        return false;

    //only accept area entered or exit signal
    if (signalId != areaEnteredSignal().methodIndex() &&
        signalId != areaExitedSignal().methodIndex())
    {
        return false;
    }

    lastError = QGeoAreaMonitorSource::NoError;

    d->requestUpdate(monitor, signalId);

    return true;
}

bool QGeoAreaMonitorPolling::stopMonitoring(const QGeoAreaMonitorInfo &monitor)
{
    QGeoAreaMonitorInfo info = d->stopMonitoring(monitor);

    return info.isValid();
}

QList<QGeoAreaMonitorInfo> QGeoAreaMonitorPolling::activeMonitors() const
{
    return d->activeMonitors().values();
}

QList<QGeoAreaMonitorInfo> QGeoAreaMonitorPolling::activeMonitors(const QGeoShape &region) const
{
    QList<QGeoAreaMonitorInfo> results;
    if (region.isEmpty())
        return results;

    const MonitorTable list = d->activeMonitors();
    for (const QGeoAreaMonitorInfo &monitor : list) {
        if (region.contains(monitor.area().center()))
            results.append(monitor);
    }

    return results;
}

QGeoAreaMonitorSource::AreaMonitorFeatures QGeoAreaMonitorPolling::supportedAreaMonitorFeatures() const
{
    return {};
}

void QGeoAreaMonitorPolling::connectNotify(const QMetaMethod &signal)
{
    QMutexLocker locker(&connectionMutex);
    if (signal == areaEnteredSignal() || signal == areaExitedSignal()) {
        const bool alreadyConnected = hasConnections();
        signalConnections++;
        if (!alreadyConnected)
            d->checkStartStop();
    }
}

void QGeoAreaMonitorPolling::disconnectNotify(const QMetaMethod &signal)
{
    QMutexLocker locker(&connectionMutex);
    if (signal == areaEnteredSignal() || signal == areaExitedSignal()) {
        if (hasConnections())
            signalConnections--;
        if (!hasConnections())
            d->checkStartStop();
    }
}

void QGeoAreaMonitorPolling::positionError(const QGeoPositionInfoSource::Error error)
{
    switch (error) {
    case QGeoPositionInfoSource::AccessError:
        lastError = QGeoAreaMonitorSource::AccessError;
        break;
    case QGeoPositionInfoSource::UnknownSourceError:
        lastError = QGeoAreaMonitorSource::UnknownSourceError;
        break;
    case QGeoPositionInfoSource::ClosedError:
    case QGeoPositionInfoSource::UpdateTimeoutError:
        lastError = QGeoAreaMonitorSource::InsufficientPositionInfo;
        break;
    case QGeoPositionInfoSource::NoError:
        return;
    }

    emit QGeoAreaMonitorSource::errorOccurred(lastError);
}

void QGeoAreaMonitorPolling::timeout(const QGeoAreaMonitorInfo& monitor)
{
    if (isSignalConnected(monitorExpiredSignal()))
        emit monitorExpired(monitor);
}

void QGeoAreaMonitorPolling::processAreaEvent(const QGeoAreaMonitorInfo &minfo,
                                              const QGeoPositionInfo &pinfo, bool isEnteredEvent)
{
    if (isEnteredEvent)
        emit areaEntered(minfo, pinfo);
    else
        emit areaExited(minfo, pinfo);
}

QT_END_NAMESPACE

#include "qgeoareamonitor_polling.moc"
#include "moc_qgeoareamonitor_polling.cpp"
