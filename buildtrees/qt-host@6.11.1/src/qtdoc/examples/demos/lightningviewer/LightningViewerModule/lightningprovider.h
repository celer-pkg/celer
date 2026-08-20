// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef LIGHTNINGPROVIDER_H
#define LIGHTNINGPROVIDER_H

#include "lightningitemdata.h"

#include <QObject>

QT_BEGIN_NAMESPACE
class QWebSocket;
class QTimer;
QT_END_NAMESPACE

class LightningProvider : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(LightningProvider)

public:
    explicit LightningProvider(QObject *parent = nullptr);
    ~LightningProvider() override;

private slots:
    void onSocketMessageReceived(const QString &message);
    void onSocketConnected();
    void onTimerTimeout();
    void requestSocket();
    void openSocket();

signals:
    void dataReady(const LightningItemData &data);

private:
    std::unique_ptr<QTimer> m_timer;
    std::unique_ptr<QWebSocket> m_webSocket;
};

#endif // LIGHTNINGPROVIDER_H
