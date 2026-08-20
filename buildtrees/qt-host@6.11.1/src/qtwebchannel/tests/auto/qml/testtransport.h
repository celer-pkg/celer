// Copyright (C) 2016 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com, author Milian Wolff <milian.wolff@kdab.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef TESTTRANSPORT_H
#define TESTTRANSPORT_H

#include <QtWebChannel/QWebChannelAbstractTransport>

QT_BEGIN_NAMESPACE

class TestTransport : public QWebChannelAbstractTransport
{
    Q_OBJECT
public:
    explicit TestTransport(QObject *parent = 0);

    virtual void sendMessage(const QJsonObject &message) override;

    Q_INVOKABLE void receiveMessage(const QString &message);

Q_SIGNALS:
    void sendMessageRequested(const QJsonObject &message);
};

QT_END_NAMESPACE

#endif // TESTTRANSPORT_H
