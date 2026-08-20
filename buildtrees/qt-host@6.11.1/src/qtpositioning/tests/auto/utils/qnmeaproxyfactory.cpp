// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qnmeaproxyfactory.h"
#include "qlocationtestutils_p.h"
#include <QtPositioning/QNmeaPositionInfoSource>
#include <QtPositioning/QNmeaSatelliteInfoSource>

#include <QIODevice>
#include <QTcpServer>
#include <QTcpSocket>

QNmeaPositionInfoSourceProxy::QNmeaPositionInfoSourceProxy(QNmeaPositionInfoSource *source,
                                                           QIODevice *outDevice)
    : m_source(source), m_outDevice(outDevice)
{
}

QNmeaPositionInfoSourceProxy::~QNmeaPositionInfoSourceProxy()
{
    m_outDevice->close();
    delete m_outDevice;
}

QGeoPositionInfoSource *QNmeaPositionInfoSourceProxy::source() const
{
    return m_source;
}

void QNmeaPositionInfoSourceProxy::feedUpdate(const QDateTime &dt)
{
    m_outDevice->write(QLocationTestUtils::createRmcSentence(dt).toLatin1());
}

void QNmeaPositionInfoSourceProxy::feedBytes(const QByteArray &bytes)
{
    m_outDevice->write(bytes);
}

QNmeaProxyFactory::QNmeaProxyFactory() : m_server(new QTcpServer(this))
{
    bool b = m_server->listen(QHostAddress::LocalHost);
    Q_ASSERT(b);
}

QNmeaPositionInfoSourceProxy *
QNmeaProxyFactory::createPositionInfoSourceProxy(QNmeaPositionInfoSource *source)
{
    QTcpSocket *client = new QTcpSocket;
    QIODevice *device = createServerConnection(client);
    source->setDevice(device);
    Q_ASSERT(source->device() != 0);
    QNmeaPositionInfoSourceProxy *proxy = new QNmeaPositionInfoSourceProxy(source, client);
    proxy->setParent(source);
    return proxy;
}

QNmeaSatelliteInfoSourceProxy *
QNmeaProxyFactory::createSatelliteInfoSourceProxy(QNmeaSatelliteInfoSource *source)
{
    QTcpSocket *client = new QTcpSocket;
    QIODevice *device = createServerConnection(client);
    source->setDevice(device);
    Q_ASSERT(source->device() != 0);
    QNmeaSatelliteInfoSourceProxy *proxy = new QNmeaSatelliteInfoSourceProxy(source, client);
    proxy->setParent(source);
    return proxy;
}

QIODevice *QNmeaProxyFactory::createServerConnection(QTcpSocket *client)
{
    client->connectToHost(m_server->serverAddress(), m_server->serverPort());
    qDebug() << "listening on" << m_server->serverAddress() << m_server->serverPort();
    bool b = m_server->waitForNewConnection(15000);
    if (!b)
        qWarning() << "Server didin't receive new connection";
    b = client->waitForConnected();
    if (!b)
        qWarning() << "Client could not connect to server";

    QIODevice *device = m_server->nextPendingConnection();
    return device;
}

QNmeaSatelliteInfoSourceProxy::QNmeaSatelliteInfoSourceProxy(QNmeaSatelliteInfoSource *source,
                                                             QIODevice *outDevice)
    : m_source(source), m_outDevice(outDevice)
{
}

QNmeaSatelliteInfoSourceProxy::~QNmeaSatelliteInfoSourceProxy()
{
    m_outDevice->close();
    delete m_outDevice;
}

QGeoSatelliteInfoSource *QNmeaSatelliteInfoSourceProxy::source() const
{
    return m_source;
}

void QNmeaSatelliteInfoSourceProxy::feedBytes(const QByteArray &bytes)
{
    m_outDevice->write(bytes);
}
