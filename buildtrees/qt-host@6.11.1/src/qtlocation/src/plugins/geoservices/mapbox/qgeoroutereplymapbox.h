// Copyright (C) 2016 Vlad Seryakov <vseryakov@gmail.com>
// Copyright (C) 2016 Aaron McCarthy <mccarthy.aaron@gmail.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QGEOROUTEREPLYMAPBOX_H
#define QGEOROUTEREPLYMAPBOX_H

#include <QtNetwork/QNetworkReply>
#include <QtLocation/QGeoRouteReply>

QT_BEGIN_NAMESPACE

class QGeoRouteReplyMapbox : public QGeoRouteReply
{
    Q_OBJECT

public:
    QGeoRouteReplyMapbox(QNetworkReply *reply, const QGeoRouteRequest &request, QObject *parent = nullptr);
    ~QGeoRouteReplyMapbox();

private Q_SLOTS:
    void networkReplyFinished();
    void networkReplyError(QNetworkReply::NetworkError error);
};

QT_END_NAMESPACE

#endif // QGEOROUTEREPLYMAPBOX_H

