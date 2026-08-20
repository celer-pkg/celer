// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant

#include "qlanguageserverjsonrpctransport_p.h"

#include <QtCore/QtGlobal>

#include <iostream>

QT_BEGIN_NAMESPACE

using namespace Qt::StringLiterals;

static const QByteArray s_contentLengthFieldName = "Content-Length";
static const QByteArray s_contentTypeFieldName = "Content-Type";
static const QByteArray s_fieldSeparator = ": ";
static const QByteArray s_headerSeparator = "\r\n";
static const QByteArray s_headerEnd = "\r\n\r\n";
static const QByteArray s_utf8 = "utf-8";
static const QByteArray s_brokenUtf8 = "utf8";

QLanguageServerJsonRpcTransport::QLanguageServerJsonRpcTransport() noexcept
    : m_messageStreamParser(
            [this](const QByteArray &field, const QByteArray &value) { hasHeader(field, value); },
            [this](const QByteArray &body) { hasBody(body); },
            [this](QtMsgType error, QString msg) {
                if (auto handler = diagnosticHandler()) {
                    if (error == QtWarningMsg || error == QtInfoMsg || error == QtDebugMsg)
                        handler(Warning, msg);
                    else
                        handler(Error, msg);
                }
            })
{
}

void QLanguageServerJsonRpcTransport::sendMessage(const QJsonDocument &packet)
{
    const QByteArray content = packet.toJson(QJsonDocument::Compact);
    if (auto handler = dataHandler()) {
        // send all data in one go, this way if handler is threadsafe the whole sendMessage is
        // threadsafe
        QByteArray msg;
        msg.append(s_contentLengthFieldName);
        msg.append(s_fieldSeparator);
        msg.append(QByteArray::number(content.size()));
        msg.append(s_headerSeparator);
        msg.append(s_headerSeparator);
        msg.append(content);
        handler(msg);
    }
}

void QLanguageServerJsonRpcTransport::receiveData(const QByteArray &data)
{
    m_messageStreamParser.receiveData(data);
}

void QLanguageServerJsonRpcTransport::hasHeader(const QByteArray &fieldName,
                                                const QByteArray &fieldValue)
{
    if (s_contentLengthFieldName.compare(fieldName, Qt::CaseInsensitive) == 0) {
        // already handled by parser
    } else if (s_contentTypeFieldName.compare(fieldName, Qt::CaseInsensitive) == 0) {
        if (fieldValue != s_utf8 && fieldValue != s_brokenUtf8) {
            if (auto handler = diagnosticHandler()) {
                handler(Warning,
                        QString::fromLatin1("Invalid %1: %2")
                                .arg(QString::fromUtf8(fieldName))
                                .arg(QString::fromUtf8(fieldValue)));
            }
        }
    } else if (auto handler = diagnosticHandler()) {
        handler(Warning,
                QString::fromLatin1("Unknown header field: %1").arg(QString::fromUtf8(fieldName)));
    }
}

void QLanguageServerJsonRpcTransport::hasBody(const QByteArray &body)
{
    QJsonParseError error = { 0, QJsonParseError::NoError };
    const QJsonDocument doc = QJsonDocument::fromJson(body, &error);

    if (error.error != QJsonParseError::NoError) {
        if (auto handler = diagnosticHandler()) {
            handler(Error,
                    u"Error %1 decoding json: %2"_s.arg(int(error.error))
                            .arg(error.errorString()));
        }
    }
    if (auto handler = messageHandler()) {
        handler(doc, error);
    }
}

QT_END_NAMESPACE
