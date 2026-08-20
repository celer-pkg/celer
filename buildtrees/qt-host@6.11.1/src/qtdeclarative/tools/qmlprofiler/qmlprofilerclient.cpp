// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "qmlprofilerclient.h"

#include <private/qqmlprofilerclient_p_p.h>

#include <QtCore/QStack>
#include <QtCore/QStringList>

#include <limits>

class QmlProfilerClientPrivate : public QQmlProfilerClientPrivate
{
    Q_DECLARE_PUBLIC(QmlProfilerClient)
public:
    QmlProfilerClientPrivate(QQmlDebugConnection *connection, QQmlProfilerEventReceiver *data);

    QQmlProfilerEventReceiver *data;
    bool enabled;
};

QmlProfilerClientPrivate::QmlProfilerClientPrivate(QQmlDebugConnection *connection,
                                                   QQmlProfilerEventReceiver *data) :
    QQmlProfilerClientPrivate(connection, data), data(data), enabled(false)
{
}

QmlProfilerClient::QmlProfilerClient(QQmlDebugConnection *connection, QQmlProfilerEventReceiver *data) :
    QQmlProfilerClient(*(new QmlProfilerClientPrivate(connection, data)))
{
    Q_D(QmlProfilerClient);
    setRequestedFeatures(std::numeric_limits<quint64>::max());
    connect(this, &QQmlDebugClient::stateChanged,
            this, &QmlProfilerClient::onStateChanged);
    connect(this, &QQmlProfilerClient::traceStarted,
            d->data, &QQmlProfilerEventReceiver::startTrace);
    connect(this, &QQmlProfilerClient::traceFinished,
            d->data, &QQmlProfilerEventReceiver::endTrace);
    connect(this, &QQmlProfilerClient::complete,
            d->data, &QQmlProfilerEventReceiver::complete);
}

void QmlProfilerClient::onStateChanged(State state)
{
    Q_D(QmlProfilerClient);
    if ((d->enabled && state != Enabled) || (!d->enabled && state == Enabled)) {
        d->enabled = (state == Enabled);
        emit enabledChanged(d->enabled);
    }
}

#include "moc_qmlprofilerclient.cpp"
