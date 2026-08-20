// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QGEOMAPITEMGEOMETRY_H
#define QGEOMAPITEMGEOMETRY_H

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
#include <QtLocation/private/qdeclarativegeomapitemutils_p.h>
#include <QtPositioning/private/qdoublevector2d_p.h>
#include <QtPositioning/private/qwebmercator_p.h>

#include <QPainterPath>
#include <QPointF>
#include <QRectF>
#include <QList>
#include <QGeoCoordinate>
#include <QGeoRectangle>
#include <QVector2D>
#include <QList>

QT_BEGIN_NAMESPACE

class QSGGeometry;
class QGeoMap;

class Q_LOCATION_EXPORT QGeoMapItemGeometry
{
public:
    static QList<QGeoCoordinate> path(const QGeoRectangle &rect)
    {
        QList<QGeoCoordinate> res;
        res << rect.topLeft();
        res << QGeoCoordinate(rect.topLeft().latitude(), rect.bottomRight().longitude());
        res << rect.bottomRight();
        res << QGeoCoordinate(rect.bottomRight().latitude(), rect.topLeft().longitude());
        return res;
    }

    static QList<QGeoCoordinate> perimeter(const QGeoRectangle &rect)
    {
        QList<QGeoCoordinate> res = path(rect);
        res.append(res.first());
        return res;
    }

    static QList<QDoubleVector2D> pathMercator(const QList<QGeoCoordinate> &p)
    {
        QList<QDoubleVector2D> res;
        for (const auto &c: p)
            res << QWebMercator::coordToMercator(c);
        return res;
    }

    QGeoMapItemGeometry();
    virtual ~QGeoMapItemGeometry();

    inline bool isSourceDirty() const { return sourceDirty_; }
    inline bool isScreenDirty() const { return screenDirty_; }
    inline void markSourceDirty() { sourceDirty_ = true; screenDirty_ = true; }
    inline void markScreenDirty() { screenDirty_ = true; clipToViewport_ = true; }
    inline void markFullScreenDirty() { screenDirty_ = true; clipToViewport_ = false;}
    inline void markClean() { screenDirty_ = (sourceDirty_ = false); clipToViewport_ = true;}
    inline void clearScreen() { screenDirty_ = false; }

    inline QGeoCoordinate geoLeftBound() { return geoLeftBound_; }

    inline QRectF sourceBoundingBox() const { return sourceBounds_; }
    inline QRectF screenBoundingBox() const { return screenBounds_; }
    inline void clearBounds() { sourceBounds_ = screenBounds_ = QRectF(); firstPointOffset_ = QPointF(); }

    inline QPointF firstPointOffset() const { return firstPointOffset_; }

    inline const QGeoCoordinate &origin() const { return srcOrigin_; }

    QPainterPath screenOutline() const {
        return screenOutline_;
    }

    virtual bool contains(const QPointF &screenPoint) const {
        return screenOutline_.contains(screenPoint);
    }

    inline QVector2D vertex(quint32 index) const {
        return QVector2D(screenVertices_[index]);
    }

    inline QList<QPointF> vertices() const { return screenVertices_; }
    inline QList<quint32> indices() const { return screenIndices_; }

    inline bool isIndexed() const { return (!screenIndices_.isEmpty()); }

    /* Size is # of triangles */
    inline quint32 size() const
    {
        if (isIndexed())
            return screenIndices_.size() / 3;
        else
            return screenVertices_.size() / 3;
    }

    inline void clear() { firstPointOffset_ = QPointF(0,0);
                          screenVertices_.clear(); screenIndices_.clear(); }

    mutable bool m_dataChanged = false;

private:
    Q_DISABLE_COPY(QGeoMapItemGeometry);

protected:
    bool sourceDirty_ = true;
    bool screenDirty_ = true;
    bool clipToViewport_ = true;
    bool preserveGeometry_ = false;
    QGeoCoordinate geoLeftBound_;

    QPointF firstPointOffset_;

    QPainterPath screenOutline_;

    QRectF sourceBounds_;
    QRectF screenBounds_;

    QGeoCoordinate srcOrigin_;

    QList<QPointF> screenVertices_;
    QList<quint32> screenIndices_;
};

QT_END_NAMESPACE

#endif // QGEOMAPITEMGEOMETRY_H
