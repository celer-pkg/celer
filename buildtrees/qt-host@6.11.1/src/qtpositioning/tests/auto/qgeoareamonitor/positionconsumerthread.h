// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef POSITIONCONSUMERTHREAD_H
#define POSITIONCONSUMERTHREAD_H

#include <QThread>
#include <QMutex>
#include <QWaitCondition>

QT_BEGIN_NAMESPACE
class QGeoAreaMonitorSource;
QT_END_NAMESPACE

// This class is created to test the behavior of QGeoAreaMonitorPolling class,
// that reimplements the connectNotify() and disconnetNotify() methods, by
// triggering these methods from multiple threads.
// The thread creates two QSignalSpy instances, that connect to the signals of
// QGeoAreaMonitorSource. Once constructed, they trigger connectNotify().
// Once destroyed, they trigger disconnectNotify.
// With the previous implementation of these overridden methods, that could lead
// to a deadlock in a rare case.
class PositionConsumerThread : public QThread
{
    Q_OBJECT
public:
    explicit PositionConsumerThread(QGeoAreaMonitorSource *source, QObject *parent = nullptr);
    ~PositionConsumerThread();

    int detectedEnterCount() const;
    int detectedExitCount() const;

public slots:
    void stopProcessing();

protected:
    void run() override;

private:
    QGeoAreaMonitorSource *m_source;

    int m_detectedEnterCount = 0;
    int m_detectedExitCount = 0;

    mutable QMutex m_mutex;
    QWaitCondition m_waitCondition;
};

#endif // POSITIONCONSUMERTHREAD_H
