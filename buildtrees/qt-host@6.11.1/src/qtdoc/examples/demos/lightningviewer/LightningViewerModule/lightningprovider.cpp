// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "lightningprovider.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QTimer>
#include <QWebSocket>

#define SECONDS_FROM_NS(ns) std::chrono::duration<int>{(ns) / 1000000000}


namespace {
constexpr auto REQUEST_MSG = "{\"action\": \"simulatelightningdata\"}";
constexpr auto WEB_SOCKET_URL = "wss://ewea0y4bn0.execute-api.eu-north-1.amazonaws.com/production/";
constexpr auto JK_TIME = "time";
constexpr auto JK_LATITUDE = "lat";
constexpr auto JK_LONGITUDE = "lon";
}

LightningProvider::LightningProvider(QObject *parent)
    : QObject{parent}
    , m_timer{new QTimer{}}
    , m_webSocket{new QWebSocket{}}
{
    connect(m_timer.get(), &QTimer::timeout, this, &LightningProvider::onTimerTimeout);
    connect(m_webSocket.get(), &QWebSocket::connected, this,
            &LightningProvider::onSocketConnected);
    connect(m_webSocket.get(), &QWebSocket::textMessageReceived, this,
            &LightningProvider::onSocketMessageReceived);

    m_timer->start(1000);
}

LightningProvider::~LightningProvider()
{
    m_webSocket.reset();
    m_timer.reset();
}

void LightningProvider::onSocketMessageReceived(const QString &message)
{
    QJsonParseError error;
    const QJsonDocument &&doc = QJsonDocument::fromJson(message.toLatin1(), &error);
    if (error.error != QJsonParseError::NoError)
        return;
    if (!doc.isObject())
        return;
    const QJsonObject &&object = doc.object();
    auto timeIter = object.constFind(JK_TIME);
    if (timeIter == object.constEnd())
        return;
    auto latIter = object.constFind(JK_LATITUDE);
    if (latIter == object.constEnd())
        return;
    auto lonIter = object.constFind(JK_LONGITUDE);
    if (lonIter == object.constEnd())
        return;

    LightningItemData data;
    data.timestamp = SECONDS_FROM_NS(timeIter->toInteger());
    data.latitude = latIter->toDouble();
    data.longitude = lonIter->toDouble();

    emit dataReady(data);
}

void LightningProvider::onSocketConnected()
{
    requestSocket();
}

void LightningProvider::onTimerTimeout()
{
    if (m_webSocket->state() == QAbstractSocket::UnconnectedState)
        openSocket();
}

void LightningProvider::requestSocket()
{
    if (m_webSocket->state() == QAbstractSocket::ConnectedState)
        m_webSocket->sendTextMessage(REQUEST_MSG);
}

void LightningProvider::openSocket()
{
    if (m_webSocket->state() != QAbstractSocket::UnconnectedState)
        return;
    m_webSocket->open( QUrl{WEB_SOCKET_URL} );
}
