// Copyright (C) 2015 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QGEONETWORKACCESSMANAGER_H
#define QGEONETWORKACCESSMANAGER_H

#include <QObject>

QT_BEGIN_NAMESPACE

class QNetworkReply;
class QNetworkRequest;
class QByteArray;

class QGeoNetworkAccessManager : public QObject
{
    Q_OBJECT
public:
    virtual ~QGeoNetworkAccessManager() {}
    virtual QNetworkReply *get(const QNetworkRequest &request) = 0;
    virtual QNetworkReply *post(const QNetworkRequest &request, const QByteArray &data) = 0;

protected:
    QGeoNetworkAccessManager(QObject *parent) : QObject(parent) {}
};

QT_END_NAMESPACE

#endif // QGEONETWORKACCESSMANAGER_H
