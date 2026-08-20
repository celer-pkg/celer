// Copyright (C) 2016 Aaron McCarthy <mccarthy.aaron@gmail.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QGEOCODEREPLYOSM_H
#define QGEOCODEREPLYOSM_H

#include <QtNetwork/QNetworkReply>
#include <QtLocation/QGeoCodeReply>
#include <QtLocation/private/qgeocodereply_p.h>

QT_BEGIN_NAMESPACE

class QGeoCodeReplyOsm : public QGeoCodeReply
{
    Q_OBJECT

public:
    explicit QGeoCodeReplyOsm(QNetworkReply *reply, bool includeExtraData = false,
                              QObject *parent = nullptr);
    ~QGeoCodeReplyOsm();

private Q_SLOTS:
    void networkReplyFinished();
    void networkReplyError(QNetworkReply::NetworkError error);

private:
    bool m_includeExtraData = false;
};

QT_END_NAMESPACE

#endif // QGEOCODEREPLYOSM_H
