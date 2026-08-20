// Copyright (C) 2015 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QDECLARATIVEROUTEMAPITEM_H_
#define QDECLARATIVEROUTEMAPITEM_H_

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
#include <QtLocation/private/qdeclarativegeomapitembase_p.h>
#include <QtLocation/private/qdeclarativegeomap_p.h>
#include <QtLocation/private/qdeclarativepolylinemapitem_p.h>
#include <QtLocation/QGeoRoute>
#include <QPen>
#include <QBrush>

QT_BEGIN_NAMESPACE

class Q_LOCATION_EXPORT QDeclarativeRouteMapItem : public QDeclarativePolylineMapItem
{
    Q_OBJECT
    QML_NAMED_ELEMENT(MapRoute)
    QML_ADDED_IN_VERSION(5, 0)

    Q_PROPERTY(QGeoRoute route READ route WRITE setRoute NOTIFY routeChanged)

public:
    explicit QDeclarativeRouteMapItem(QQuickItem *parent = nullptr);
    ~QDeclarativeRouteMapItem();

    QGeoRoute route() const;
    void setRoute(const QGeoRoute &route);

Q_SIGNALS:
    void routeChanged(const QGeoRoute &route);

private slots:
    void updateRoutePath();

protected:
    void setPath(const QList<QGeoCoordinate> &value) override;

private:
    QGeoRoute route_;
};

QT_END_NAMESPACE

QML_DECLARE_TYPE(QDeclarativeRouteMapItem)

#endif /* QDECLARATIVEROUTEMAPITEM_H_ */
