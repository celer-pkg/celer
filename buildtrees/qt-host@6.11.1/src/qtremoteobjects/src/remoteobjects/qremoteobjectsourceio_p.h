// Copyright (C) 2017 Ford Motor Company
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:declarations-only

#ifndef QREMOTEOBJECTSOURCEIO_P_H
#define QREMOTEOBJECTSOURCEIO_P_H

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

#include "qconnectionfactories_p.h"
#include "qtremoteobjectglobal.h"
#include "qremoteobjectpacket_p.h"

#include <QtCore/qiodevice.h>
#include <QtCore/qscopedpointer.h>
#include <QtNetwork/qlocalserver.h>

QT_BEGIN_NAMESPACE

class QRemoteObjectSourceBase;
class QRemoteObjectRootSource;
class SourceApiMap;
class QRemoteObjectHostBase;

class QRemoteObjectSourceIo : public QObject
{
    Q_OBJECT
public:
    explicit QRemoteObjectSourceIo(const QUrl &address, QObject *parent = nullptr);
    explicit QRemoteObjectSourceIo(QObject *parent = nullptr);
    ~QRemoteObjectSourceIo() override;

    bool startListening();
    bool enableRemoting(QObject *object, const QMetaObject *meta, const QString &name,
                        const QString &typeName);
    bool enableRemoting(QObject *object, const SourceApiMap *api, QObject *adapter = nullptr);
    bool disableRemoting(QObject *object);
    void newConnection(QtROIoDeviceBase *conn);
    void setSocketOptions(QLocalServer::SocketOptions options);

    QUrl serverAddress() const;

public Q_SLOTS:
    void handleConnection();
    void onServerDisconnect(QObject *obj = nullptr);
    void onServerRead(QObject *obj);

Q_SIGNALS:
    void remoteObjectAdded(const QRemoteObjectSourceLocation &);
    void remoteObjectRemoved(const QRemoteObjectSourceLocation &);
    void serverRemoved(const QUrl& url);

public:
    void registerSource(QRemoteObjectSourceBase *source);
    void unregisterSource(QRemoteObjectSourceBase *source);

    QHash<QIODevice*, quint32> m_readSize;
    QSet<QtROIoDeviceBase*> m_connections;
    QHash<QObject *, QRemoteObjectRootSource*> m_objectToSourceMap;
    QMap<QString, QRemoteObjectSourceBase*> m_sourceObjects;
    QMap<QString, QRemoteObjectRootSource*> m_sourceRoots;
    QHash<QtROIoDeviceBase*, QUrl> m_registryMapping;
    QScopedPointer<QConnectionAbstractServer> m_server;
    // TODO should have some sort of manager for the codec
    QScopedPointer<QRemoteObjectPackets::CodecBase> m_codec{new QRemoteObjectPackets::QDataStreamCodec};
    QString m_rxName;
    QVariantList m_rxArgs;
    QUrl m_address;
};

QT_END_NAMESPACE

#endif
