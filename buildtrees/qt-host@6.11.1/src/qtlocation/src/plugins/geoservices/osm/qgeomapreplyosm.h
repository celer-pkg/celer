// Copyright (C) 2016 Aaron McCarthy <mccarthy.aaron@gmail.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QGEOMAPREPLYOSM_H
#define QGEOMAPREPLYOSM_H

#include <QtNetwork/QNetworkReply>
#include <QtLocation/private/qgeotiledmapreply_p.h>

QT_BEGIN_NAMESPACE

class QGeoMapReplyOsm : public QGeoTiledMapReply
{
    Q_OBJECT

public:
    QGeoMapReplyOsm(QNetworkReply *reply, const QGeoTileSpec &spec, const QString &imageFormat,
                    QObject *parent = nullptr);
    ~QGeoMapReplyOsm();

private Q_SLOTS:
    void networkReplyFinished();
    void networkReplyError(QNetworkReply::NetworkError error);
};

QT_END_NAMESPACE

#endif // QGEOMAPREPLYOSM_H
