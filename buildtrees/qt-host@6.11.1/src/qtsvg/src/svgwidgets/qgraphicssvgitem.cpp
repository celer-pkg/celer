// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qgraphicssvgitem.h"

#if !defined(QT_NO_GRAPHICSVIEW)

#include "qpainter.h"
#include "qstyleoption.h"
#include "qsvgrenderer.h"
#include "qdebug.h"

#include <QtCore/private/qobject_p.h>
#include <QtWidgets/private/qgraphicsitem_p.h>

QT_BEGIN_NAMESPACE

class QGraphicsSvgItemPrivate : public QGraphicsItemPrivate
{
public:
    Q_DECLARE_PUBLIC(QGraphicsSvgItem)

    QGraphicsSvgItemPrivate()
        : renderer(0), shared(false)
    {
    }

    void init(QGraphicsItem *parent)
    {
        Q_Q(QGraphicsSvgItem);
        q->setParentItem(parent);
        renderer = new QSvgRenderer(q);
        QObject::connect(renderer, SIGNAL(repaintNeeded()),
                         q, SLOT(_q_repaintItem()));
        q->setCacheMode(QGraphicsItem::DeviceCoordinateCache);
        q->setMaximumCacheSize(QSize(1024, 768));
    }

    void _q_repaintItem()
    {
        q_func()->update();
    }

    inline void updateDefaultSize()
    {
        QRectF bounds;
        if (elemId.isEmpty()) {
            bounds = QRectF(QPointF(0, 0), renderer->defaultSize());
        } else {
            bounds = renderer->boundsOnElement(elemId);
        }
        if (boundingRect.size() != bounds.size()) {
            q_func()->prepareGeometryChange();
            boundingRect.setSize(bounds.size());
        }
    }

    QSvgRenderer *renderer;
    QRectF boundingRect;
    bool shared;
    QString elemId;
};

/*!
    \class QGraphicsSvgItem
    \inmodule QtSvgWidgets
    \ingroup graphicsview-api
    \brief The QGraphicsSvgItem class is a QGraphicsItem that can be used to render
           the contents of SVG files.

    \since 4.2

    QGraphicsSvgItem provides a way of rendering SVG files onto QGraphicsView.
    QGraphicsSvgItem can be created by passing the SVG file to be rendered to
    its constructor or by explicit setting a shared QSvgRenderer on it.

    Note that setting QSvgRenderer on a QGraphicsSvgItem doesn't make the item take
    ownership of the renderer, therefore if using setSharedRenderer() method one has
    to make sure that the lifetime of the QSvgRenderer object will be at least as long
    as that of the QGraphicsSvgItem.

    QGraphicsSvgItem provides a way of rendering only parts of the SVG files via
    the setElementId. If setElementId() method is called, only the SVG element
    (and its children) with the passed id will be renderer. This provides a convenient
    way of selectively rendering large SVG files that contain a number of discrete
    elements. For example the following code renders only jokers from a SVG file
    containing a whole card deck:

    \snippet src_svg_qgraphicssvgitem.cpp 0

    Size of the item can be set via direct manipulation of the items
    transformation matrix.

    By default the SVG rendering is cached using QGraphicsItem::DeviceCoordinateCache
    mode to speedup the display of items. Caching can be disabled by passing
    QGraphicsItem::NoCache to the QGraphicsItem::setCacheMode() method.

    \sa QSvgWidget, {Qt SVG C++ Classes}, QGraphicsItem, QGraphicsView
*/

/*!
    Constructs a new SVG item with the given \a parent.
*/
QGraphicsSvgItem::QGraphicsSvgItem(QGraphicsItem *parent)
    : QGraphicsObject(*new QGraphicsSvgItemPrivate(), 0)
{
    Q_D(QGraphicsSvgItem);
    d->init(parent);
}

/*!
    Constructs a new item with the given \a parent and loads the contents of the
    SVG file with the specified \a fileName.
*/
QGraphicsSvgItem::QGraphicsSvgItem(const QString &fileName, QGraphicsItem *parent)
    : QGraphicsObject(*new QGraphicsSvgItemPrivate(), 0)
{
    Q_D(QGraphicsSvgItem);
    d->init(parent);
    d->renderer->load(fileName);
    d->updateDefaultSize();
}

/*!
    Returns the currently use QSvgRenderer.
*/
QSvgRenderer *QGraphicsSvgItem::renderer() const
{
    return d_func()->renderer;
}


/*!
    Returns the bounding rectangle of this item.
*/
QRectF QGraphicsSvgItem::boundingRect() const
{
    Q_D(const QGraphicsSvgItem);
    return d->boundingRect;
}

// from qgraphicsitem.cpp
void Q_WIDGETS_EXPORT qt_graphicsItem_highlightSelected(QGraphicsItem *item, QPainter *painter,
                                                        const QStyleOptionGraphicsItem *option);

/*!
    \reimp
*/
void QGraphicsSvgItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
                             QWidget *widget)
{
//    Q_UNUSED(option);
    Q_UNUSED(widget);

    Q_D(QGraphicsSvgItem);
    if (!d->renderer->isValid())
        return;

    if (d->elemId.isEmpty())
        d->renderer->render(painter, d->boundingRect);
    else
        d->renderer->render(painter, d->elemId, d->boundingRect);

    if (option->state & QStyle::State_Selected)
        qt_graphicsItem_highlightSelected(this, painter, option);
}

/*!
    \reimp
*/
int QGraphicsSvgItem::type() const
{
    return Type;
}

/*!
  \property QGraphicsSvgItem::maximumCacheSize
  \since 4.6

  This property holds the maximum size of the device coordinate cache
  for this item.
 */

/*!
    Sets the maximum device coordinate cache size of the item to \a size.
    If the item is cached using QGraphicsItem::DeviceCoordinateCache mode,
    caching is bypassed if the extension of the item in device coordinates
    is larger than \a size.

    The cache corresponds to the QPixmap which is used to cache the
    results of the rendering.
    Use QPixmapCache::setCacheLimit() to set limitations on the whole cache
    and use setMaximumCacheSize() when setting cache size for individual
    items.

    \sa QGraphicsItem::cacheMode()
*/
void QGraphicsSvgItem::setMaximumCacheSize(const QSize &size)
{
    QGraphicsItem::d_ptr->setExtra(QGraphicsItemPrivate::ExtraMaxDeviceCoordCacheSize, size);
    update();
}

/*!
    Returns the current maximum size of the device coordinate cache for this item.
    If the item is cached using QGraphicsItem::DeviceCoordinateCache mode,
    caching is bypassed if the extension of the item in device coordinates
    is larger than the maximum size.

    The default maximum cache size is 1024x768.
    QPixmapCache::cacheLimit() gives the
    cumulative bounds of the whole cache, whereas maximumCacheSize() refers
    to a maximum cache size for this particular item.

    \sa QGraphicsItem::cacheMode()
*/
QSize QGraphicsSvgItem::maximumCacheSize() const
{
    return QGraphicsItem::d_ptr->extra(QGraphicsItemPrivate::ExtraMaxDeviceCoordCacheSize).toSize();
}

/*!
  \property QGraphicsSvgItem::elementId
  \since 4.6
  
  This property holds the element's XML ID.
 */

/*!
    Sets the XML ID of the element to \a id.
*/
void QGraphicsSvgItem::setElementId(const QString &id)
{
    Q_D(QGraphicsSvgItem);
    d->elemId = id;
    d->updateDefaultSize();
    update();
}

/*!
    Returns the XML ID the element that is currently
    being rendered. Returns an empty string if the whole
    file is being rendered.
*/
QString QGraphicsSvgItem::elementId() const
{
    Q_D(const QGraphicsSvgItem);
    return d->elemId;
}

/*!
    Sets \a renderer to be a shared QSvgRenderer on the item. By
    using this method one can share the same QSvgRenderer on a number
    of items. This means that the SVG file will be parsed only once.
    QSvgRenderer passed to this method has to exist for as long as
    this item is used.
*/
void QGraphicsSvgItem::setSharedRenderer(QSvgRenderer *renderer)
{
    Q_D(QGraphicsSvgItem);
    if (!d->shared)
        delete d->renderer;

    d->renderer = renderer;
    d->shared = true;

    d->updateDefaultSize();

    update();
}

/*!
    \deprecated

    Use QGraphicsItem::setCacheMode() instead. Passing true to this function is equivalent
    to QGraphicsItem::setCacheMode(QGraphicsItem::DeviceCoordinateCache).
*/
void QGraphicsSvgItem::setCachingEnabled(bool caching)
{
    setCacheMode(caching ? QGraphicsItem::DeviceCoordinateCache : QGraphicsItem::NoCache);
}

/*!
    \deprecated

    Use QGraphicsItem::cacheMode() instead.
*/
bool QGraphicsSvgItem::isCachingEnabled() const
{
    return cacheMode() != QGraphicsItem::NoCache;
}

QT_END_NAMESPACE

#include "moc_qgraphicssvgitem.cpp"

#endif // QT_NO_GRAPHICSVIEW
