// Copyright (C) 2017-2016 Ford Motor Company
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QNXIPCPRIVATE_GLOBAL_H
#define QNXIPCPRIVATE_GLOBAL_H

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

#include <sys/neutrino.h>
#include <sys/dispatch.h>
#include <sys/siginfo.h>
#include <unistd.h> // provides SETIOV
#include <sys/netmgr.h>  //FOR ND_LOCAL_NODE
#include <errno.h>
#include <QtCore/qthread.h>
#include <QtCore/private/qglobal_p.h>
#ifdef USE_HAM
# include <ha/ham.h>
#endif

#define WARNING(cmd) qCWarning(QT_REMOTEOBJECT) << "Warning " #cmd << strerror(errno) \
                        << Q_FUNC_INFO << __FILE__ << __LINE__;

#define WARN_ON_ERROR(cmd, ...) if (cmd(__VA_ARGS__) == -1) qCWarning(QT_REMOTEOBJECT) \
                        << "Error " #cmd << strerror(errno) << Q_FUNC_INFO << __FILE__ << __LINE__;

#define WARN_AND_RETURN_ON_ERROR(cmd, retval, ...) if (cmd(__VA_ARGS__) == -1) \
                        { qCWarning(QT_REMOTEOBJECT) << "Error " #cmd << strerror(errno) \
                        << Q_FUNC_INFO << __FILE__ << __LINE__; return (retval); }

#define FATAL_ON_ERROR(cmd, ...) if (cmd(__VA_ARGS__) == -1) qFatal("Error %s: %s %s %s %d", \
                        #cmd, strerror(errno), Q_FUNC_INFO, __FILE__, __LINE__);

QT_BEGIN_NAMESPACE

const int MAX_RETRIES = 3;

enum MsgType : uint16_t { REPLICA_INIT = _IO_MAX+100,
                          REPLICA_TX_RECV,
                          SOURCE_TX_RESP,
                          SOURCE_TX_RESP_REPEAT,
                        };
enum PulseType : uint8_t { SOURCE_TX_RQ = _PULSE_CODE_MINAVAIL+42,
                           REPLICA_WRITE,
                           TERMINATE,
                           NODE_DEATH
                         };
union recv_msgs
{
    struct _pulse pulse;
    uint16_t type;
};

template <typename T>
class Thread : public QThread
{
public:
    Thread(T *obj, const QString &name = QString()) : QThread(), m_obj(obj)
    {
        if (!name.isEmpty())
            setObjectName(name);
    }
    void run() override
    {
        m_obj->thread_func();
    }
private:
    T *m_obj;
};

QT_END_NAMESPACE

#endif // QNXIPCPRIVATE_GLOBAL_H

