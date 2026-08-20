// Copyright (C) 2021 Ford Motor Company
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QCONNECTIONFACTORIES_H
#define QCONNECTIONFACTORIES_H

//
//  W A R N I N G
//  -------------
//
// This file is part of the QtRO internal API and is not meant to be used
// in applications. It contains "internal" components, which are exported
// to allow new QIODevice channels to be added to extend QtRO. Usage of these
// APIs may make your code source and binary incompatible with future versions
// of Qt.
//

#include <QtNetwork/qabstractsocket.h>

#include <QtRemoteObjects/qtremoteobjectglobal.h>
#include <QtRemoteObjects/qremoteobjectnode.h>

QT_BEGIN_NAMESPACE

class QtROIoDeviceBasePrivate;
class QtROClientIoDevicePrivate;
class QtROExternalIoDevicePrivate;

class Q_REMOTEOBJECTS_EXPORT QtROIoDeviceBase : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(QtROIoDeviceBase)

public:
    explicit QtROIoDeviceBase(QObject *parent = nullptr);
    ~QtROIoDeviceBase() override;

    bool read(QtRemoteObjects::QRemoteObjectPacketTypeEnum &, QString &);

    virtual void write(const QByteArray &data);
    virtual void write(const QByteArray &data, qint64);
    virtual bool isOpen() const;
    virtual void close();
    virtual qint64 bytesAvailable() const;
    virtual QIODevice *connection() const = 0;
    void initializeDataStream();
    bool isClosing() const;
    void addSource(const QString &);
    void removeSource(const QString &);
    QSet<QString> remoteObjects() const;

Q_SIGNALS:
    void readyRead();
    void disconnected();

protected:
    explicit QtROIoDeviceBase(QtROIoDeviceBasePrivate &, QObject *parent);
    virtual QString deviceType() const = 0;
    virtual void doClose() = 0;

private:
    Q_DECLARE_PRIVATE(QtROIoDeviceBase)
    friend class QRemoteObjectNodePrivate;
    friend class QConnectedReplicaImplementation;
    friend class QRemoteObjectSourceIo;
};

class Q_REMOTEOBJECTS_EXPORT QtROServerIoDevice : public QtROIoDeviceBase
{
    Q_OBJECT
    Q_DISABLE_COPY(QtROServerIoDevice)

public:
    explicit QtROServerIoDevice(QObject *parent = nullptr);

protected:
    QString deviceType() const override;
};

class Q_REMOTEOBJECTS_EXPORT QConnectionAbstractServer : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(QConnectionAbstractServer)

public:
    explicit QConnectionAbstractServer(QObject *parent = nullptr);
    ~QConnectionAbstractServer() override;

    virtual bool hasPendingConnections() const = 0;
    QtROServerIoDevice* nextPendingConnection();
    virtual QUrl address() const = 0;
    virtual bool listen(const QUrl &address) = 0;
    virtual QAbstractSocket::SocketError serverError() const = 0;
    virtual void close() = 0;

protected:
    virtual QtROServerIoDevice* configureNewConnection() = 0;

Q_SIGNALS:
    void newConnection();
};

class Q_REMOTEOBJECTS_EXPORT QtROClientIoDevice : public QtROIoDeviceBase
{
    Q_OBJECT
    Q_DISABLE_COPY(QtROClientIoDevice)

public:
    explicit QtROClientIoDevice(QObject *parent = nullptr);
    ~QtROClientIoDevice() override;

    void disconnectFromServer();
    virtual void connectToServer() = 0;

    QUrl url() const;

Q_SIGNALS:
    void shouldReconnect(QtROClientIoDevice*);
    void setError(QRemoteObjectNode::ErrorCode);

protected:
    virtual void doDisconnectFromServer() = 0;
    QString deviceType() const override;
    void setUrl(const QUrl &url);

private:
    Q_DECLARE_PRIVATE(QtROClientIoDevice)
    friend class QtROClientFactory;
};

class QtROServerFactory
{
public:
    Q_REMOTEOBJECTS_EXPORT static QtROServerFactory *instance();

    QConnectionAbstractServer *create(const QUrl &url, QObject *parent = nullptr)
    {
        auto creatorFunc = m_creatorFuncs.value(url.scheme());
        return creatorFunc ? (*creatorFunc)(parent) : nullptr;
    }

    template<typename T>
    void registerType(const QString &id)
    {
        m_creatorFuncs[id] = [](QObject *parent) -> QConnectionAbstractServer * {
            return new T(parent);
        };
    }

    bool isValid(const QUrl &url)
    {
        return m_creatorFuncs.contains(url.scheme());
    }

private:
    friend class QtROFactoryLoader;
    QtROServerFactory();

    using CreatorFunc = QConnectionAbstractServer * (*)(QObject *);
    QHash<QString, CreatorFunc> m_creatorFuncs;
};

class QtROClientFactory
{
public:
    Q_REMOTEOBJECTS_EXPORT static QtROClientFactory *instance();

    /// creates an object from a string
    QtROClientIoDevice *create(const QUrl &url, QObject *parent = nullptr)
    {
        auto creatorFunc = m_creatorFuncs.value(url.scheme());
        if (!creatorFunc)
            return nullptr;

        QtROClientIoDevice *res = (*creatorFunc)(parent);
        if (res)
            res->setUrl(url);
        return res;
    }

    template<typename T>
    void registerType(const QString &id)
    {
        m_creatorFuncs[id] = [](QObject *parent) -> QtROClientIoDevice * {
            return new T(parent);
        };
    }

    bool isValid(const QUrl &url)
    {
        return m_creatorFuncs.contains(url.scheme());
    }

private:
    friend class QtROFactoryLoader;
    QtROClientFactory();

    using CreatorFunc = QtROClientIoDevice * (*)(QObject *);
    QHash<QString, CreatorFunc> m_creatorFuncs;
};

template <typename T>
inline void qRegisterRemoteObjectsClient(const QString &id)
{
    QtROClientFactory::instance()->registerType<T>(id);
}

template <typename T>
inline void qRegisterRemoteObjectsServer(const QString &id)
{
    QtROServerFactory::instance()->registerType<T>(id);
}

QT_END_NAMESPACE

#endif // QCONNECTIONFACTORIES_H
