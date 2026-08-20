// Copyright (C) 2014 Canonical Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QGEOMAPREPLYMAPBOX_H
#define QGEOMAPREPLYMAPBOX_H

#include <QtNetwork/QNetworkReply>
#include <QtLocation/private/qgeotiledmapreply_p.h>
#include <QtCore/QPointer>

QT_BEGIN_NAMESPACE

class QGeoMapReplyMapbox : public QGeoTiledMapReply
{
    Q_OBJECT

public:
    explicit QGeoMapReplyMapbox(QNetworkReply *reply, const QGeoTileSpec &spec, const QString &format, QObject *parent = nullptr);
    ~QGeoMapReplyMapbox();

private Q_SLOTS:
    void networkReplyFinished();
    void networkReplyError(QNetworkReply::NetworkError error);

private:
    QString m_format;
};

QT_END_NAMESPACE

#endif // QGEOMAPREPLYMAPBOX_H
