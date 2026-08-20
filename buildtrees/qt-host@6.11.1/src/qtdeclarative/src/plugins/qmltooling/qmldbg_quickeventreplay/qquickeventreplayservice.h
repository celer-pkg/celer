// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant

#ifndef QQUICKEVENTREPLAYSERVICE_H
#define QQUICKEVENTREPLAYSERVICE_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <private/qqmlconfigurabledebugservice_p.h>
#include <private/qqmldebugserviceinterfaces_p.h>
#include <private/qqmlprofilerdefinitions_p.h>
#include <private/qqmldebugconnector_p.h>
#include <private/qversionedpacket_p.h>
#include <private/qquickprofiler_p.h>

#include <QtCore/qqueue.h>
#include <QtCore/qtimer.h>

QT_BEGIN_NAMESPACE

using QQmlDebugPacket = QVersionedPacket<QQmlDebugConnector>;

class QQuickEventReplayServiceImpl : public QQuickEventReplayService, public QQmlProfilerDefinitions
{
    Q_OBJECT
public:
    QQuickEventReplayServiceImpl(QObject *parent = nullptr);

protected:
    void messageReceived(const QByteArray &) override;

Q_SIGNALS:
    void dataAvailable();

private:
    void start();
    void sendNextEvent();
    void scheduleNextEvent(const QQuickProfilerData &nextEvent);
    QQuickProfilerData takeNextEvent();

    QQueue<QQuickProfilerData> m_data;
    QMutex m_dataMutex;

    QElapsedTimer m_elapsed;
    QTimer m_schedule;

    Qt::KeyboardModifiers m_currentModifiers = Qt::NoModifier;
    Qt::MouseButtons m_currentButtons = Qt::NoButton;
    QPoint m_currentPos;
};

QT_END_NAMESPACE

#endif // QQUICKEVENTREPLAYSERVICE_H

