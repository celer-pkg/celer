// Copyright (C) 2017-2016 Ford Motor Company
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:declarations-only

#ifndef QQNXNATIVESERVER_P_H
#define QQNXNATIVESERVER_P_H

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

#include "private/qobject_p.h"
#include "qconnection_qnx_server.h"
#include "qconnection_qnx_global_p.h"

#include <QtCore/qatomic.h>
#include <QtCore/qmutex.h>
#include <QtCore/qsharedpointer.h>

QT_BEGIN_NAMESPACE

class QQnxNativeServerPrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(QQnxNativeServer)

public:
    QQnxNativeServerPrivate();

    ~QQnxNativeServerPrivate();

    void thread_func();

    void cleanupIOSource(QIOQnxSource *conn);
    void teardownServer();
    void createSource(int rcvid, uint64_t uid, pid_t toPid);
#ifdef USE_HAM
    bool initializeHam();
    void configureHamDeath(int sentChannelId, pid_t toPid, uint64_t uid);
    void closeHamResources();
#endif

    bool listen(const QString &name);
    QString errorString;
    QAbstractSocket::SocketError error;
    QString serverName;
    name_attach_t *attachStruct;
    QHash<int, QSet<int> > connections;
    QHash<uint64_t, QSharedPointer<QIOQnxSource>> sources;
    QList<QSharedPointer<QIOQnxSource>> pending;
    QAtomicInt running;
    Thread<QQnxNativeServerPrivate> thread;
    mutable QMutex mutex;
    int terminateCoid;
#ifdef USE_HAM
    ham_entity_t *hamEntityHandle;
    ham_condition_t *hamConditionHandle;
    QHash<uint64_t, ham_action_t*> hamActions;
    bool hamAvailable = false;
    bool hamInitialized = false;
#endif
};

QT_END_NAMESPACE

#endif // QQNXNATIVESERVER_P_H

