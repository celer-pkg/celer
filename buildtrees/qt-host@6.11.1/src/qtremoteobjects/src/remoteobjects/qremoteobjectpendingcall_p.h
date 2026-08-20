// Copyright (C) 2017 Ford Motor Company
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QREMOTEOBJECTPENDINGCALL_P_H
#define QREMOTEOBJECTPENDINGCALL_P_H

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

#include "qremoteobjectpendingcall.h"

#include <QtCore/qmutex.h>
#include <QtCore/private/qglobal_p.h>

QT_BEGIN_NAMESPACE

class QRemoteObjectPendingCallWatcherHelper;
class QRemoteObjectReplicaImplementation;

class QRemoteObjectPendingCallData : public QSharedData
{
public:
    typedef QExplicitlySharedDataPointer<QRemoteObjectPendingCallData> Ptr;

    explicit QRemoteObjectPendingCallData(int serialId = -1, QRemoteObjectReplicaImplementation *replica = nullptr);
    ~QRemoteObjectPendingCallData();

    QRemoteObjectReplicaImplementation *replica;
    int serialId;

    QVariant returnValue;
    QRemoteObjectPendingCall::Error error;

    mutable QMutex mutex;

    mutable QScopedPointer<QRemoteObjectPendingCallWatcherHelper> watcherHelper;
};

class QRemoteObjectPendingCallWatcherHelper: public QObject
{
    Q_OBJECT
public:
    void add(QRemoteObjectPendingCallWatcher *watcher);

    void emitSignals();

Q_SIGNALS:
    void finished();
};

QT_END_NAMESPACE

#endif
