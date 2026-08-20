// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "oauthtestutils.h"

#ifndef QT_NO_SSL
#include <QtNetwork/qsslkey.h>
#include <QtCore/qfile.h>
#endif

#include <QtCore/qdatetime.h>
#include <QtCore/qjsondocument.h>
#include <QtCore/qmessageauthenticationcode.h>

QString createSignedJWT(const QVariantMap &header, const QVariantMap &payload)
{
    auto base64Encode = [](const QByteArray &input) {
        return input.toBase64(QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals);
    };
    // Repeating values which can be overridden and augmented by the supplied 'header' and 'payload'
    QVariantMap mergedHeader = {{"alg", "HS256"},
                                {"typ", "JWT"}};
    QVariantMap mergedPayload = {{"iss", "https://tst_oauth2.example.com"},
                                 {"sub", "tst_oauth2"},
                                 {"aud", "tst_oauth2_client_id"},
                                 {"exp", QDateTime::currentSecsSinceEpoch() + 300}, // valid 5 mins
                                 {"iat", QDateTime::currentSecsSinceEpoch()}, // issued now
                                 {"name", "No Body"},
                                 {"email", "no.body@example.com"}};
    mergedHeader.insert(header);
    mergedPayload.insert(payload);

    // Signed JWT within OIDC context is: header.payload.signature (separated with dots)
    auto header64 =
        base64Encode(QJsonDocument::fromVariant(mergedHeader).toJson(QJsonDocument::Compact));
    auto payload64 =
        base64Encode(QJsonDocument::fromVariant(mergedPayload).toJson(QJsonDocument::Compact));
    auto token = header64 + "." + payload64;
    auto signature64 =
        base64Encode(QMessageAuthenticationCode::hash(token, "secret", QCryptographicHash::Sha256));
    token = token + "." + signature64;
    return token;
}

#ifndef QT_NO_SSL
QSslConfiguration createSslConfiguration(const QString &keyFileName,
                                         const QString &certificateFileName)
{
    QSslConfiguration configuration(QSslConfiguration::defaultConfiguration());

    QFile keyFile(keyFileName);
    if (keyFile.open(QIODevice::ReadOnly)) {
        QSslKey key(keyFile.readAll(), QSsl::Rsa, QSsl::Pem, QSsl::PrivateKey);
        if (!key.isNull()) {
            configuration.setPrivateKey(key);
        } else {
            qCritical() << "Could not parse key: " << keyFileName;
        }
    } else {
        qCritical() << "Could not find key: " << keyFileName;
    }

    QList<QSslCertificate> localCert = QSslCertificate::fromPath(certificateFileName);
    if (!localCert.isEmpty() && !localCert.first().isNull()) {
        configuration.setLocalCertificate(localCert.first());
    } else {
        qCritical() << "Could not find certificate: " << certificateFileName;
    }

    configuration.setPeerVerifyMode(QSslSocket::VerifyPeer);

    return configuration;
}
#endif // QT_NO_SSL
