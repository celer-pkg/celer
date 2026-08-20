// Copyright (C) 2017 Ford Motor Company
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QREMOTEOBJECTPENDINGCALL_H
#define QREMOTEOBJECTPENDINGCALL_H

#include <QtRemoteObjects/qtremoteobjectglobal.h>

#include <QtCore/qvariant.h>

QT_BEGIN_NAMESPACE

class QRemoteObjectPendingCallWatcherPrivate;
class QRemoteObjectPendingCallData;

class Q_REMOTEOBJECTS_EXPORT QRemoteObjectPendingCall
{
public:
    enum Error {
        NoError,
        InvalidMessage
    };

    QRemoteObjectPendingCall();
    QRemoteObjectPendingCall(const QRemoteObjectPendingCall &other);
    ~QRemoteObjectPendingCall();

    QRemoteObjectPendingCall &operator=(const QRemoteObjectPendingCall &other);

    QVariant returnValue() const;
    QRemoteObjectPendingCall::Error error() const;

    bool isFinished() const;

    bool waitForFinished(int timeout = 30000);

    static QRemoteObjectPendingCall fromCompletedCall(const QVariant &returnValue);

protected:
    QRemoteObjectPendingCall(QRemoteObjectPendingCallData *dd);

    /// Shared data, note: might be null
    QExplicitlySharedDataPointer<QRemoteObjectPendingCallData> d;

private:
    friend class QConnectedReplicaImplementation;
};

QT_END_NAMESPACE
QT_DECL_METATYPE_EXTERN(QRemoteObjectPendingCall, Q_REMOTEOBJECTS_EXPORT)
QT_BEGIN_NAMESPACE

class Q_REMOTEOBJECTS_EXPORT QRemoteObjectPendingCallWatcher: public QObject, public QRemoteObjectPendingCall
{
    Q_OBJECT

public:
    QRemoteObjectPendingCallWatcher(const QRemoteObjectPendingCall &call, QObject *parent = nullptr);
    ~QRemoteObjectPendingCallWatcher() override;

    bool isFinished() const;

    void waitForFinished();

Q_SIGNALS:
    void finished(QRemoteObjectPendingCallWatcher *self);

private:
    Q_DECLARE_PRIVATE(QRemoteObjectPendingCallWatcher)
};

template<typename T>
class QRemoteObjectPendingReply : public QRemoteObjectPendingCall
{
public:
    typedef T Type;

    QRemoteObjectPendingReply() = default;
    explicit QRemoteObjectPendingReply(const QRemoteObjectPendingCall &call)
        : QRemoteObjectPendingCall(call)
    {
    }

    QRemoteObjectPendingReply &operator=(const QRemoteObjectPendingCall &other)
    {
        QRemoteObjectPendingCall::operator=(other);
        return *this;
    }

    Type returnValue() const
    {
        return qvariant_cast<Type>(QRemoteObjectPendingCall::returnValue());
    }

};

QT_END_NAMESPACE

#endif
