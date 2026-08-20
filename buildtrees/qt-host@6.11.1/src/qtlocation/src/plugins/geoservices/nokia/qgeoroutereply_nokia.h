// Copyright (C) 2015 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QGEOROUTEREPLY_NOKIA_H
#define QGEOROUTEREPLY_NOKIA_H

#include <qgeoroutereply.h>
#include <QNetworkReply>

QT_BEGIN_NAMESPACE

class QGeoRouteXmlParser;

class QGeoRouteReplyNokia : public QGeoRouteReply
{
    Q_OBJECT
public:
    QGeoRouteReplyNokia(const QGeoRouteRequest &request, const QList<QNetworkReply*> &replies,
                        QObject *parent = nullptr);
    ~QGeoRouteReplyNokia();

private Q_SLOTS:
    void networkFinished();
    void networkError(QNetworkReply::NetworkError error);
    void appendResults(const QList<QGeoRoute> &routes);
    void parserError(const QString &errorString);

private:
    int m_parsers;
};

QT_END_NAMESPACE

#endif
