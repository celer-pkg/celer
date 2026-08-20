// Copyright (C) 2015 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QGEOMAPREPLY_NOKIA_H
#define QGEOMAPREPLY_NOKIA_H

#include <QtNetwork/QNetworkReply>
#include <QtLocation/private/qgeotilespec_p.h>
#include <QtLocation/private/qgeotiledmapreply_p.h>
#include <QtCore/QPointer>

QT_BEGIN_NAMESPACE

class QGeoMapReplyNokia : public QGeoTiledMapReply
{
    Q_OBJECT

public:
    QGeoMapReplyNokia(QNetworkReply *reply, const QGeoTileSpec &spec, QObject *parent = nullptr);
    ~QGeoMapReplyNokia();

private Q_SLOTS:
    void networkFinished();
    void networkError(QNetworkReply::NetworkError error);
};

QT_END_NAMESPACE

#endif
