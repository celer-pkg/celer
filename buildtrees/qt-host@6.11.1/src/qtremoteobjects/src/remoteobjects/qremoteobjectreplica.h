// Copyright (C) 2017 Ford Motor Company
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QQREMOTEOBJECTREPLICA_H
#define QQREMOTEOBJECTREPLICA_H

#include <QtRemoteObjects/qtremoteobjectglobal.h>

#include <QtCore/qobject.h>
#include <QtCore/qsharedpointer.h>

Q_MOC_INCLUDE(<QtRemoteObjects/qremoteobjectnode.h>)

QT_BEGIN_NAMESPACE

class QObjectPrivate;
class QRemoteObjectPendingCall;
class QRemoteObjectReplicaImplementation;
class QReplicaImplementationInterface;
class QRemoteObjectNode;

class Q_REMOTEOBJECTS_EXPORT QRemoteObjectReplica : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QRemoteObjectNode *node READ node WRITE setNode)
    Q_PROPERTY(State state READ state NOTIFY stateChanged)
public:
    enum State {
        Uninitialized,
        Default,
        Valid,
        Suspect,
        SignatureMismatch
    };
    Q_ENUM(State)

public:
    ~QRemoteObjectReplica() override;

    bool isReplicaValid() const;
    bool waitForSource(int timeout = 30000);
    bool isInitialized() const;
    State state() const;
    QRemoteObjectNode *node() const;
    virtual void setNode(QRemoteObjectNode *node);

Q_SIGNALS:
    void initialized();
    void notified();
    void stateChanged(State state, State oldState);

protected:
    enum ConstructorType {DefaultConstructor, ConstructWithNode};
    explicit QRemoteObjectReplica(ConstructorType t = DefaultConstructor);
    QRemoteObjectReplica(QObjectPrivate &dptr, QObject *parent);

    virtual void initialize();
    void send(QMetaObject::Call call, int index, const QVariantList &args);
    QRemoteObjectPendingCall sendWithReply(QMetaObject::Call call, int index, const QVariantList &args);

protected:
    void setProperties(QVariantList &&);
    void setChild(int i, const QVariant &);
    const QVariant propAsVariant(int i) const;
    void persistProperties(const QString &repName, const QByteArray &repSig, const QVariantList &props) const;
    QVariantList retrieveProperties(const QString &repName, const QByteArray &repSig) const;
    void initializeNode(QRemoteObjectNode *node, const QString &name = QString());
    QSharedPointer<QReplicaImplementationInterface> d_impl;
private:
    friend class QRemoteObjectNodePrivate;
    friend class QConnectedReplicaImplementation;
};

QT_END_NAMESPACE

#endif
