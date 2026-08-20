// Copyright (C) 2016 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com, author Milian Wolff <milian.wolff@kdab.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "testtransport.h"

#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>

QT_BEGIN_NAMESPACE

TestTransport::TestTransport(QObject *parent)
: QWebChannelAbstractTransport(parent)
{

}

void TestTransport::sendMessage(const QJsonObject &message)
{
    emit sendMessageRequested(message);
}

void TestTransport::receiveMessage(const QString &message)
{
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8(), &error);
    if (error.error) {
        qWarning("Failed to parse JSON message: %s\nError is: %s",
                 qPrintable(message), qPrintable(error.errorString()));
        return;
    } else if (!doc.isObject()) {
        qWarning("Received JSON message that is not an object: %s",
                 qPrintable(message));
        return;
    }
    emit messageReceived(doc.object(), this);
}

QT_END_NAMESPACE
