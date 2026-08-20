// Copyright (C) 2016 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com, author Milian Wolff <milian.wolff@kdab.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QWEBCHANNELABSTRACTTRANSPORT_H
#define QWEBCHANNELABSTRACTTRANSPORT_H

#include <QtCore/QObject>
#include <QtWebChannel/qwebchannelglobal.h>

QT_BEGIN_NAMESPACE

class QJsonObject;
class Q_WEBCHANNEL_EXPORT QWebChannelAbstractTransport : public QObject
{
    Q_OBJECT
public:
    explicit QWebChannelAbstractTransport(QObject *parent = nullptr);
    ~QWebChannelAbstractTransport() override;

public Q_SLOTS:
    virtual void sendMessage(const QJsonObject &message) = 0;

Q_SIGNALS:
    void messageReceived(const QJsonObject &message, QWebChannelAbstractTransport *transport);
};

QT_END_NAMESPACE

#endif // QWEBCHANNELABSTRACTTRANSPORT_H
