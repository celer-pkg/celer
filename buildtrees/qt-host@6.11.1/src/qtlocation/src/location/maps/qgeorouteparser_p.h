// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QOSRMROUTEPARSER_P_H
#define QOSRMROUTEPARSER_P_H

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

#include <QtLocation/private/qlocationglobal_p.h>
#include <QtLocation/qgeoroutereply.h>

QT_BEGIN_NAMESPACE

class QByteArray;
class QUrl;
class QGeoRouteRequest;
class QGeoRouteParserPrivate;
class Q_LOCATION_EXPORT QGeoRouteParser : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QGeoRouteParser)
    Q_PROPERTY(TrafficSide trafficSide READ trafficSide WRITE setTrafficSide NOTIFY trafficSideChanged)
public:
    enum TrafficSide {
        RightHandTraffic,
        LeftHandTraffic
    };
    Q_ENUM(TrafficSide)

    virtual ~QGeoRouteParser();
    QGeoRouteReply::Error parseReply(QList<QGeoRoute> &routes, QString &errorString, const QByteArray &reply) const;
    QUrl requestUrl(const QGeoRouteRequest &request, const QString &prefix) const;

    TrafficSide trafficSide() const;

public Q_SLOTS:
    void setTrafficSide(TrafficSide trafficSide);

Q_SIGNALS:
    void trafficSideChanged(TrafficSide trafficSide);

protected:
    QGeoRouteParser(QGeoRouteParserPrivate &dd, QObject *parent = nullptr);

private:
    Q_DISABLE_COPY(QGeoRouteParser)
};

QT_END_NAMESPACE

#endif // QOSRMROUTEPARSER_P_H
