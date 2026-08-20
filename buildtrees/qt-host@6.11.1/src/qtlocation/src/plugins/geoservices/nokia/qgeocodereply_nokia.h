// Copyright (C) 2015 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QGEOCODEREPLY_NOKIA_H
#define QGEOCODEREPLY_NOKIA_H

#include <qgeocodereply.h>
#include <QNetworkReply>

QT_BEGIN_NAMESPACE

class QGeoCodeReplyNokia : public QGeoCodeReply
{
    Q_OBJECT
public:
    QGeoCodeReplyNokia(QNetworkReply *reply, int limit, int offset, const QGeoShape &viewport,
                       bool manualBoundsRequired, QObject *parent = nullptr);
    ~QGeoCodeReplyNokia();

private Q_SLOTS:
    void networkFinished();
    void networkError(QNetworkReply::NetworkError error);
    void appendResults(const QList<QGeoLocation> &locations);
    void parseError(const QString &errorString);

private:
    bool m_parsing;
    bool m_manualBoundsRequired;
};

QT_END_NAMESPACE

#endif
