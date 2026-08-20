// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QSignalSpy>
#include <QtPositioning/qgeoareamonitorsource.h>
#include "positionconsumerthread.h"

PositionConsumerThread::PositionConsumerThread(QGeoAreaMonitorSource *source, QObject *parent)
    : QThread(parent), m_source(source)
{
}

PositionConsumerThread::~PositionConsumerThread()
{
    stopProcessing();
    wait();
}

int PositionConsumerThread::detectedEnterCount() const
{
    QMutexLocker locker(&m_mutex);
    return m_detectedEnterCount;
}

int PositionConsumerThread::detectedExitCount() const
{
    QMutexLocker locker(&m_mutex);
    return m_detectedExitCount;
}

void PositionConsumerThread::stopProcessing()
{
    m_mutex.lock();
    m_waitCondition.wakeOne();
    m_mutex.unlock();
}

void PositionConsumerThread::run()
{
    QSignalSpy enterSpy(m_source, &QGeoAreaMonitorSource::areaEntered);
    QSignalSpy exitSpy(m_source, &QGeoAreaMonitorSource::areaExited);

    m_mutex.lock();
    m_waitCondition.wait(&m_mutex);
    m_detectedEnterCount = enterSpy.size();
    m_detectedExitCount = exitSpy.size();
    m_mutex.unlock();
}
