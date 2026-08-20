// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qquick3dxritem_p.h"
#include "qquick3dxrview_p.h"
#include <QtQuick3D/private/qquick3dnode_p_p.h>
#include <QtQuick/private/qquickrectangle_p.h>
#include <QtQuick/private/qquickitem_p.h>
#include <QColor>

QT_BEGIN_NAMESPACE


class QQuick3DXrItemPrivate : public QQuick3DNodePrivate
{
    Q_DECLARE_PUBLIC(QQuick3DXrItem)
public:
    QQuick3DXrItemPrivate() : QQuick3DNodePrivate(QQuick3DXrItemPrivate::Type::Node)
    {
    }

    void setContentItem(QQuickItem *newContentItem)
    {
        Q_Q(QQuick3DXrItem);
        m_contentItem = newContentItem;

        initParentItem();

        if (m_contentItemDestroyedConnection) {
            QObject::disconnect(m_contentItemDestroyedConnection);
            m_contentItemDestroyedConnection = {};
        }
        if (m_contentItem) {
            m_contentItem->setParentItem(m_containerItem);
            m_contentItem->setParent(m_containerItem);
            m_contentItemDestroyedConnection = QObject::connect(m_contentItem, &QObject::destroyed, q, [q]() {
                q->setContentItem(nullptr);
            });

            if (m_automaticHeight) {
                setAutomaticHeightConnection();
            }

            if (m_automaticWidth) {
                setAutomaticWidthConnection();
            }
        }

        updateContent();
    }

    void initParentItem()
    {
        Q_Q(QQuick3DXrItem);
        if (!m_containerItem) {
            m_containerItem = new QQuickRectangle;
            m_containerItem->setTransformOrigin(QQuickItem::TopLeft);
            m_containerItem->setParent(q);
            m_containerItem->setVisible(q->visible());
            QObject::connect(q, &QQuick3DNode::visibleChanged, m_containerItem, [this, q](){
                m_containerItem->setVisible(q->visible());
            });
            QQuickItemPrivate::get(m_containerItem)->requestCustomOverlay();
            auto dataProp = data();
            dataProp.append(&dataProp, m_containerItem);
        }
    }

    void updateContent();

void setAutomaticHeightConnection()
{
    Q_Q(QQuick3DXrItem);
    if (m_automaticHeightConnection && ((m_contentItem == nullptr) || !m_automaticHeight)) {
        QObject::disconnect(m_automaticHeightConnection);
        m_automaticHeightConnection = {};
    }
    if (m_contentItem) {
        m_automaticHeightConnection = QObject::connect(m_contentItem, &QQuickItem::heightChanged, q, [this, q](){
            qreal newHeight = m_contentItem->height()/m_pixelsPerUnit;
            if (m_height != newHeight) {
                m_height = newHeight;
                emit q->heightChanged();
                updateContent();
            }
        });
    }
}

void setAutomaticWidthConnection()
{
    Q_Q(QQuick3DXrItem);
    if (m_automaticWidthConnection && ((m_contentItem == nullptr) || !m_automaticHeight)) {
        QObject::disconnect(m_automaticWidthConnection);
        m_automaticWidthConnection = {};
    }

    if (m_contentItem) {
        m_automaticWidthConnection = QObject::connect(m_contentItem, &QQuickItem::widthChanged, q, [this, q](){
            qreal newWidth = m_contentItem->width()/m_pixelsPerUnit;
            if (m_width != newWidth) {
                m_width = newWidth;
                emit q->widthChanged();
                updateContent();
            }
        });
    }
}

    QQuickItem *m_contentItem = nullptr;
    QQuickRectangle *m_containerItem = nullptr;
    QPointer<QQuick3DXrView> m_XrView;
    QMetaObject::Connection m_contentItemDestroyedConnection;
    QMetaObject::Connection m_automaticHeightConnection;
    QMetaObject::Connection m_automaticWidthConnection;
    QColor m_color = Qt::white;
    qreal m_pixelsPerUnit { 1.0 };
    qreal m_width = 1;
    qreal m_height = 1;
    bool m_manualPixelsPerUnit = false;
    bool m_automaticHeight = false;
    bool m_automaticWidth = false;
};

static inline qreal calculatePPU(qreal pxWidth, qreal pxHeight, qreal diagonal)
{
    return (diagonal > 0) ? std::sqrt((pxWidth * pxWidth) + (pxHeight * pxHeight)) / diagonal : 1.0;
}

void QQuick3DXrItemPrivate::updateContent()
{
    if (!componentComplete)
        return;
    Q_Q(QQuick3DXrItem);
    initParentItem();
    m_containerItem->setColor(m_color);
    if (m_contentItem) {
        if (Q_UNLIKELY(m_manualPixelsPerUnit && m_pixelsPerUnit < 0)) {
            qWarning() << "XrItem invalid pixelPerUnit" << m_pixelsPerUnit;
            return;
        }
        qreal newScale;
        if (m_manualPixelsPerUnit) {
            newScale = 1.0 / m_pixelsPerUnit;

        } else {
            qreal diagonal = std::sqrt((m_width * m_width) + (m_height * m_height));
            qreal ppu = calculatePPU(m_contentItem->width(), m_contentItem->height(), diagonal);
            if (ppu <= 0)
                ppu = 1.0;
            q->setPixelsPerUnit(ppu);
            newScale = 1.0 / ppu;
        }
        QSizeF newSize(m_width / newScale, m_height / newScale);
        m_containerItem->setSize(newSize);
        m_containerItem->setScale(newScale);
    }
}

/*!
    \qmltype XrItem
    \inqmlmodule QtQuick3D.Xr
    \inherits Node
    \brief A virtual surface in 3D space that can hold 2D user interface content.
    \since 6.8

    The XrItem type is a Qt Quick 3D \l Node that represents a rectangle with \l width and \l height.
    It holds one Qt Quick \l Item, specified by \l contentItem, and scales it to fit.
    This gives a convenient way to take traditional 2D user interfaces and display them on a virtual surface that has
    a real world size.

    Any other children of the XrItem will be treated as normal children of a Node, and will not be scaled.

    For example the following code will create a virtual surface that's 1 meter by 1 meter and with a content item
    that's 600 pixels by 600 pixels. Note that the effect here is achieved by scaling the content item and not
    by rendering the content item at a higher resolution.

    \code
    XrItem {
        width: 100
        height: 100
        contentItem: Rectangle {
            width: 600
            height: 600
            color: "red"
        }
    }
    \endcode
*/
QQuick3DXrItem::QQuick3DXrItem(QQuick3DNode *parent)
    : QQuick3DNode(*(new QQuick3DXrItemPrivate()), parent)
{
}

QQuick3DXrItem::~QQuick3DXrItem()
{
    Q_D(QQuick3DXrItem);
    if (d->m_XrView)
        d->m_XrView->unregisterXrItem(this);
}

void QQuick3DXrItem::componentComplete()
{
    Q_D(QQuick3DXrItem);
    QQuick3DNode::componentComplete(); // Sets d->componentComplete, so must be called first

    auto findView = [this]() -> QQuick3DXrView * {
        QQuick3DNode *parent = parentNode();
        while (parent) {
            if (auto *xrView = qobject_cast<QQuick3DXrView*>(parent))
                return xrView;
            parent = parent->parentNode();
        }
        return nullptr;
    };
    d->m_XrView = findView();
    if (d->m_XrView)
        d->m_XrView->registerXrItem(this);
    else
        qWarning("Could not find XrView for XrItem");
    d->updateContent();
}

/*!
    \qmlproperty Item XrItem::contentItem

    This property holds the content item that will be displayed on the virtual surface.
    The content item's size will be used to calculate the pixels per unit value and scale based on this item's size.

    \sa pixelsPerUnit
 */
QQuickItem *QQuick3DXrItem::contentItem() const
{
    Q_D(const QQuick3DXrItem);
    return d->m_contentItem;
}

void QQuick3DXrItem::setContentItem(QQuickItem *newContentItem)
{
    Q_D(QQuick3DXrItem);
    if (d->m_contentItem == newContentItem)
        return;

    d->setContentItem(newContentItem);
    emit contentItemChanged();
}

/*!
    \qmlproperty real XrItem::pixelsPerUnit

    This property determines how many pixels in the contentItems's 2D coordinate system will fit
    in one unit of the XrItem's 3D coordinate system. By default, this is calculated based on the
    content item's size and the size of the XrItem.

    Set \l manualPixelsPerUnit to disable the default behavior and set the pixels per unit value manually.

    \sa manualPixelsPerUnit
 */
qreal QQuick3DXrItem::pixelsPerUnit() const
{
    Q_D(const QQuick3DXrItem);
    return d->m_pixelsPerUnit;
}

void QQuick3DXrItem::setPixelsPerUnit(qreal newPixelsPerUnit)
{
    Q_D(QQuick3DXrItem);
    if (qFuzzyCompare(d->m_pixelsPerUnit, newPixelsPerUnit))
        return;

    d->m_pixelsPerUnit = newPixelsPerUnit;

    if (d->m_manualPixelsPerUnit)
        d->updateContent();

    emit pixelsPerUnitChanged();
}

/*!
    \qmlproperty bool XrItem::manualPixelsPerUnit

    If this property is \c true, the ratio between the contentItems's 2D coordinate system and this
    XrItem's 3D coordinate system is defined by the value of \l pixelsPerUnit. If this property is \c false,
    the ratio is calculated based on the content item's size and the size of the XrItem.

    \default false
    \sa pixelsPerUnit
*/

bool QQuick3DXrItem::manualPixelsPerUnit() const
{
    Q_D(const QQuick3DXrItem);
    return d->m_manualPixelsPerUnit;
}

void QQuick3DXrItem::setManualPixelsPerUnit(bool newManualPixelsPerUnit)
{
    Q_D(QQuick3DXrItem);
    if (d->m_manualPixelsPerUnit == newManualPixelsPerUnit)
        return;
    d->m_manualPixelsPerUnit = newManualPixelsPerUnit;
    d->updateContent();
    emit manualPixelsPerUnitChanged();
}

/*!
    \qmlproperty real XrItem::width

    This property defines the width of the XrItem in the 3D coordinate system.
    \sa height
*/

qreal QQuick3DXrItem::width() const
{
    Q_D(const QQuick3DXrItem);
    return d->m_width;
}

void QQuick3DXrItem::setWidth(qreal newWidth)
{
    Q_D(QQuick3DXrItem);
    if ((d->m_width == newWidth) || d->m_automaticWidth)
        return;
    d->m_width = newWidth;
    emit widthChanged();

    d->updateContent();
}

/*!
    \qmlproperty real XrItem::height

    This property defines the height of the XrItem in the 3D coordinate system.
    \sa width
*/

qreal QQuick3DXrItem::height() const
{
    Q_D(const QQuick3DXrItem);
    return d->m_height;
}

void QQuick3DXrItem::setHeight(qreal newHeight)
{
    Q_D(QQuick3DXrItem);
    if ((d->m_height == newHeight) || d->m_automaticHeight)
        return;
    d->m_height = newHeight;
    emit heightChanged();

    d->updateContent();
}

/*!
   \qmlproperty color XrItem::color

    This property defines the background color of the XrItem.

    \default "white"
 */

QColor QQuick3DXrItem::color() const
{
    Q_D(const QQuick3DXrItem);
    return d->m_color;
}

void QQuick3DXrItem::setColor(const QColor &newColor)
{
    Q_D(QQuick3DXrItem);
    if (d->m_color == newColor)
        return;
    d->m_color = newColor;
    emit colorChanged();
    d->updateContent();
}

/*!
   \qmlproperty bool XrItem::automaticHeight

    When set to true XrItem will ignore height set through height property and use height calculated from contentItem height.

    \default "false"
    \sa automaticWidth
 */

bool QQuick3DXrItem::automaticHeight() const
{
    Q_D(const QQuick3DXrItem);
    return d->m_automaticHeight;
}

void QQuick3DXrItem::setAutomaticHeight(bool newAutomaticHeight)
{
    Q_D(QQuick3DXrItem);
    if (d->m_automaticHeight == newAutomaticHeight) {
        return;
    }

    d->m_automaticHeight = newAutomaticHeight;
    d->setAutomaticHeightConnection();
    d->updateContent();
    emit automaticHeightChanged();
}

/*!
   \qmlproperty bool XrItem::automaticWidth

    When set to true XrItem will ignore width set through width property and use width calculated from contentItem width.

    \default "false"
    \sa automaticHeight
 */

bool QQuick3DXrItem::automaticWidth() const
{
    Q_D(const QQuick3DXrItem);
    return d->m_automaticWidth;
}

void QQuick3DXrItem::setAutomaticWidth(bool newAutomaticWidth)
{
    Q_D(QQuick3DXrItem);
    if (d->m_automaticWidth == newAutomaticWidth)
        return;

    d->m_automaticWidth = newAutomaticWidth;
    d->setAutomaticWidthConnection();
    d->updateContent();
    emit automaticWidthChanged();
}

QT_END_NAMESPACE
