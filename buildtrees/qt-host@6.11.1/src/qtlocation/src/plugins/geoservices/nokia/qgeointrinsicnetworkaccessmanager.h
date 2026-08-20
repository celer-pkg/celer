// Copyright (C) 2015 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QGEOINTRINSICNETWORKACCESSMANAGER_H
#define QGEOINTRINSICNETWORKACCESSMANAGER_H

#include "qgeonetworkaccessmanager.h"

#include <QMap>

QT_BEGIN_NAMESPACE

class QNetworkAccessManager;

class QGeoIntrinsicNetworkAccessManager : public QGeoNetworkAccessManager
{
public:
    explicit QGeoIntrinsicNetworkAccessManager(QObject *parent = nullptr);
    QGeoIntrinsicNetworkAccessManager(const QVariantMap &parameters,
                                      const QString &token = QString(), QObject *parent = nullptr);

    QNetworkReply *get(const QNetworkRequest &request) override;
    QNetworkReply *post(const QNetworkRequest &request, const QByteArray &data) override;

private:
    void configure(const QVariantMap &parameters);

    const QString m_customProxyToken;
    QNetworkAccessManager *m_networkManager;
};

QT_END_NAMESPACE

#endif // QGEOINTRINSICNETWORKACCESSMANAGER_H
