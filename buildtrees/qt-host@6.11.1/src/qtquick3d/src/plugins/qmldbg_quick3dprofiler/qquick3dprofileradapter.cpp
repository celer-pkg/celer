// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qquick3dprofileradapter.h"

#include <QCoreApplication>
#include <private/qqmldebugconnector_p.h>
#include <private/qversionedpacket_p.h>
#include <private/qqmldebugserviceinterfaces_p.h>

QT_BEGIN_NAMESPACE

using QQmlDebugPacket = QVersionedPacket<QQmlDebugConnector>;

QQuick3DProfilerAdapter::QQuick3DProfilerAdapter(QObject *parent) :
    QQmlAbstractProfilerAdapter(parent), next(0)
{
    QQuick3DProfiler::initialize(this);
    // We can always do DirectConnection here as all methods are protected by mutexes
    connect(this, &QQmlAbstractProfilerAdapter::profilingEnabled,
            QQuick3DProfiler::s_instance, &QQuick3DProfiler::startProfilingImpl, Qt::DirectConnection);
    connect(this, &QQmlAbstractProfilerAdapter::profilingEnabledWhileWaiting,
            QQuick3DProfiler::s_instance, &QQuick3DProfiler::startProfilingImpl, Qt::DirectConnection);
    connect(this, &QQmlAbstractProfilerAdapter::referenceTimeKnown,
            QQuick3DProfiler::s_instance, &QQuick3DProfiler::setTimer, Qt::DirectConnection);
    connect(this, &QQmlAbstractProfilerAdapter::profilingDisabled,
            QQuick3DProfiler::s_instance, &QQuick3DProfiler::stopProfilingImpl, Qt::DirectConnection);
    connect(this, &QQmlAbstractProfilerAdapter::profilingDisabledWhileWaiting,
            QQuick3DProfiler::s_instance, &QQuick3DProfiler::stopProfilingImpl, Qt::DirectConnection);
    connect(this, &QQmlAbstractProfilerAdapter::dataRequested,
            QQuick3DProfiler::s_instance, &QQuick3DProfiler::reportDataImpl, Qt::DirectConnection);
    connect(QQuick3DProfiler::s_instance, &QQuick3DProfiler::dataReady,
            this, &QQuick3DProfilerAdapter::receiveData, Qt::DirectConnection);
}

QQuick3DProfilerAdapter::~QQuick3DProfilerAdapter()
{
    if (service)
        service->removeGlobalProfiler(this);
}


// convert to QByteArrays that can be sent to the debug client
static void QQuick3DProfilerDataToByteArrays(const QQuick3DProfilerData &data,
                                             QList<QByteArray> &messages,
                                             const QHash<int, QByteArray> &eventData)
{
    QQmlDebugPacket ds;

    // packet header
    ds << data.time << data.messageType << data.detailType;
    // packet data
    switch (data.messageType) {
    case QQuick3DProfiler::Event:
        break;
    case QQuick3DProfiler::Quick3DFrame:
        if (data.detailType == QQuick3DProfiler::Quick3DEventData) {
            ds << eventData[data.subdata1];
        } else {
            ds << data.subdata1 << data.subdata2;
            if (data.ids[0] || data.ids[1])
                ds << data.ids[0] << data.ids[1];
        }
        break;
    default:
        Q_ASSERT_X(false, Q_FUNC_INFO, "Invalid message type.");
        break;
    }

    messages.append(ds.squeezedData());
    ds.clear();
}

qint64 QQuick3DProfilerAdapter::sendMessages(qint64 until, QList<QByteArray> &messages)
{
    while (next < m_data.size()) {
        if (m_data[next].time <= until && messages.size() <= s_numMessagesPerBatch)
            QQuick3DProfilerDataToByteArrays(m_data[next++], messages, m_eventData);
        else
            return m_data[next].time;
    }
    m_data.clear();
    next = 0;
    return -1;
}

void QQuick3DProfilerAdapter::receiveData(const QVector<QQuick3DProfilerData> &new_data, const QHash<int, QByteArray> &eventData)
{
    if (m_data.isEmpty())
        m_data = new_data;
    else
        m_data.append(new_data);
    m_eventData = eventData;
    service->dataReady(this);
}

QT_END_NAMESPACE
