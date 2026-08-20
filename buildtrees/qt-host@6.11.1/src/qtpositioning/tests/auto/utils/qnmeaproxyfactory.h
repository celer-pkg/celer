// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QNMEAPROXYFACTORY_H
#define QNMEAPROXYFACTORY_H

#include <QObject>

QT_BEGIN_NAMESPACE
class QTcpServer;
class QTcpSocket;
class QIODevice;
class QNmeaPositionInfoSource;
class QNmeaSatelliteInfoSource;
class QGeoPositionInfoSource;
class QGeoSatelliteInfoSource;
QT_END_NAMESPACE

class QNmeaPositionInfoSourceProxy : public QObject
{
    Q_OBJECT
public:
    QNmeaPositionInfoSourceProxy(QNmeaPositionInfoSource *source, QIODevice *outDevice);
    ~QNmeaPositionInfoSourceProxy();

    QGeoPositionInfoSource *source() const;

    void feedUpdate(const QDateTime &dt);

    void feedBytes(const QByteArray &bytes);

    int updateIntervalErrorMargin() const { return 50; }

private:
    QNmeaPositionInfoSource *m_source;
    QIODevice *m_outDevice;
};

class QNmeaSatelliteInfoSourceProxy : public QObject
{
    Q_OBJECT
public:
    QNmeaSatelliteInfoSourceProxy(QNmeaSatelliteInfoSource *source, QIODevice *outDevice);
    ~QNmeaSatelliteInfoSourceProxy();

    QGeoSatelliteInfoSource *source() const;

    void feedBytes(const QByteArray &bytes);

private:
    QNmeaSatelliteInfoSource *m_source;
    QIODevice *m_outDevice;
};

class QNmeaProxyFactory : public QObject
{
    Q_OBJECT
public:
    QNmeaProxyFactory();

    // proxy is created as child of source
    QNmeaPositionInfoSourceProxy *createPositionInfoSourceProxy(QNmeaPositionInfoSource *source);
    QNmeaSatelliteInfoSourceProxy *createSatelliteInfoSourceProxy(QNmeaSatelliteInfoSource *source);

private:
    QIODevice *createServerConnection(QTcpSocket *client);

    QTcpServer *m_server;
};

#endif
