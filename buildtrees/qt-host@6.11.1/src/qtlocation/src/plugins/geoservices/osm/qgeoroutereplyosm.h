// Copyright (C) 2016 Aaron McCarthy <mccarthy.aaron@gmail.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QGEOROUTEREPLYOSM_H
#define QGEOROUTEREPLYOSM_H

#include <QtNetwork/QNetworkReply>
#include <QtLocation/QGeoRouteReply>

QT_BEGIN_NAMESPACE

class QGeoRouteReplyOsm : public QGeoRouteReply
{
    Q_OBJECT

public:
    QGeoRouteReplyOsm(QNetworkReply *reply, const QGeoRouteRequest &request,
                      QObject *parent = nullptr);
    ~QGeoRouteReplyOsm();

private Q_SLOTS:
    void networkReplyFinished();
    void networkReplyError(QNetworkReply::NetworkError error);
};

QT_END_NAMESPACE

#endif // QGEOROUTEREPLYOSM_H

