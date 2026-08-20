// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QGEOROUTEPARSER_P_P_H
#define QGEOROUTEPARSER_P_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtCore/private/qobject_p.h>
#include <QtCore/QUrl>
#include <QtLocation/qgeoroutereply.h>
#include <QtLocation/qgeorouterequest.h>

QT_BEGIN_NAMESPACE

class Q_LOCATION_EXPORT QGeoRouteParserPrivate :  public QObjectPrivate
{
    Q_DECLARE_PUBLIC(QGeoRouteParser)
public:
    QGeoRouteParserPrivate();
    virtual ~QGeoRouteParserPrivate();

    virtual QGeoRouteReply::Error parseReply(QList<QGeoRoute> &routes, QString &errorString, const QByteArray &reply) const = 0;
    virtual QUrl requestUrl(const QGeoRouteRequest &request, const QString &prefix) const = 0;

    QGeoRouteParser::TrafficSide trafficSide = QGeoRouteParser::RightHandTraffic;
};

QT_END_NAMESPACE

#endif // QGEOROUTEPARSER_P_P_H
