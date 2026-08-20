// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qsvgnode_p.h"
#include "qsvgdocument_p.h"
#include "qsvggraphics_p.h"

#include <QLoggingCategory>
#include<QElapsedTimer>
#include <QtGui/qimageiohandler.h>

#include "qdebug.h"

#include <QtGui/private/qoutlinemapper_p.h>

QT_BEGIN_NAMESPACE

#ifndef QT_NO_DEBUG
Q_STATIC_LOGGING_CATEGORY(lcSvgTiming, "qt.svg.timing")
#endif

#if !defined(QT_SVG_SIZE_LIMIT)
#  define QT_SVG_SIZE_LIMIT QT_RASTER_COORD_LIMIT
#endif

QSvgNode::QSvgNode(QSvgNode *parent)
    : m_parent(parent),
      m_displayMode(BlockMode),
      m_visible(true)
{
}

QSvgNode::~QSvgNode()
{

}

void QSvgNode::draw(QPainter *p, QSvgExtraStates &states)
{
#ifndef QT_NO_DEBUG
    QElapsedTimer qtSvgTimer; qtSvgTimer.start();
#endif

    if (shouldDrawNode(p, states)) {
        quint8 remainingDepth = states.trustedSource ? states.remainingNestedNodes
                                                     : states.remainingNestedNodes - 1;
        QScopedValueRollback<quint8> nestedNodesGuard(states.remainingNestedNodes, remainingDepth);
        if (states.remainingNestedNodes == 0) {
            qCWarning(lcSvgDraw) << "Too many nested nodes at" << qPrintable(typeName())
                                 << "exceeding max nested limit of" << QtSvg::renderingMaxNestedNodes << "."
                                 << "Enable AssumeTrustedSource in QSvgHandler or set QT_SVG_DEFAULT_OPTIONS=2 to disable this check.";
            return;
        }

        applyStyle(p, states);
        applyAnimatedStyle(p, states);
        QSvgNode *maskNode = this->hasMask() ? document()->namedNode(this->maskId()) : nullptr;
        QSvgFilterContainer *filterNode = this->hasFilter() ? static_cast<QSvgFilterContainer*>(document()->namedNode(this->filterId()))
                                                            : nullptr;
        if (filterNode && filterNode->type() == QSvgNode::Filter && filterNode->supported()) {
            QTransform xf = p->transform();
            p->resetTransform();
            QRectF localRect = internalBounds(p, states);
            p->setTransform(xf);
            QRectF boundsRect = xf.mapRect(filterNode->filterRegion(localRect));
            QImage proxy = drawIntoBuffer(p, states, boundsRect.toRect());
            proxy = filterNode->applyFilter(proxy, p, localRect);
            if (maskNode && maskNode->type() == QSvgNode::Mask) {
                boundsRect = QRectF(proxy.offset(), proxy.size());
                localRect = p->transform().inverted().mapRect(boundsRect);
                QImage mask = static_cast<QSvgMask*>(maskNode)->createMask(p, states, localRect, &boundsRect);
                applyMaskToBuffer(&proxy, mask);
            }
            applyBufferToCanvas(p, proxy);

        } else if (maskNode && maskNode->type() == QSvgNode::Mask) {
            QRectF boundsRect;
            QImage mask = static_cast<QSvgMask*>(maskNode)->createMask(p, states, this, &boundsRect);
            drawWithMask(p, states, mask, boundsRect.toRect());
        } else if (!qFuzzyCompare(p->opacity(), qreal(1.0)) && requiresGroupRendering()) {
            QTransform xf = p->transform();
            p->resetTransform();

            QRectF localRect = decoratedInternalBounds(p, states);
            // adding safety border needed because of the antialiazing effects
            QRectF boundsRect = xf.mapRect(localRect);
            const int deltaX = boundsRect.width() * 0.1;
            const int deltaY = boundsRect.height() * 0.1;
            boundsRect = boundsRect.adjusted(-deltaX, -deltaY, deltaX, deltaY);

            p->setTransform(xf);

            QImage proxy = drawIntoBuffer(p, states, boundsRect.toAlignedRect());
            applyBufferToCanvas(p, proxy);
        } else {
            if (separateFillStroke(p, states))
                fillThenStroke(p, states);
            else
                drawCommand(p, states);

        }
        revertAnimatedStyle(p ,states);
        revertStyle(p, states);
    }

#ifndef QT_NO_DEBUG
    if (Q_UNLIKELY(lcSvgTiming().isDebugEnabled()))
        qCDebug(lcSvgTiming) << "Drawing" << typeName() << "took" << (qtSvgTimer.nsecsElapsed() / 1000000.0f) << "ms";
#endif
}

void QSvgNode::fillThenStroke(QPainter *p, QSvgExtraStates &states)
{
    qreal oldOpacity = p->opacity();
    if (p->brush().style() != Qt::NoBrush) {
        QPen oldPen = p->pen();
        p->setPen(Qt::NoPen);
        p->setOpacity(oldOpacity * states.fillOpacity);

        drawCommand(p, states);

        p->setPen(oldPen);
    }
    if (p->pen() != Qt::NoPen && p->pen().brush() != Qt::NoBrush && p->pen().widthF() != 0) {
        QBrush oldBrush = p->brush();
        p->setOpacity(oldOpacity * states.strokeOpacity);
        p->setBrush(Qt::NoBrush);

        drawCommand(p, states);

        p->setBrush(oldBrush);
    }
    p->setOpacity(oldOpacity);
}

void QSvgNode::drawWithMask(QPainter *p, QSvgExtraStates &states, const QImage &mask, const QRect &boundsRect)
{
    QImage proxy = drawIntoBuffer(p, states, boundsRect);
    if (proxy.isNull())
        return;
    applyMaskToBuffer(&proxy, mask);

    p->save();
    p->resetTransform();
    p->drawImage(boundsRect, proxy);
    p->restore();
}

QImage QSvgNode::drawIntoBuffer(QPainter *p, QSvgExtraStates &states, const QRect &boundsRect)
{
    QImage proxy;
    if (!QImageIOHandler::allocateImage(boundsRect.size(), QImage::Format_ARGB32_Premultiplied, &proxy)) {
        qCWarning(lcSvgDraw) << "The requested buffer size is too big, ignoring";
        return proxy;
    }
    proxy.setOffset(boundsRect.topLeft());
    proxy.fill(Qt::transparent);
    QPainter proxyPainter(&proxy);
    proxyPainter.setPen(p->pen());
    proxyPainter.setBrush(p->brush());
    proxyPainter.setFont(p->font());
    proxyPainter.translate(-boundsRect.topLeft());
    proxyPainter.setTransform(p->transform(), true);
    proxyPainter.setRenderHints(p->renderHints());
    if (separateFillStroke(p, states))
        fillThenStroke(&proxyPainter, states);
    else
        drawCommand(&proxyPainter, states);
    return proxy;
}

void QSvgNode::applyMaskToBuffer(QImage *proxy, QImage mask) const
{
    QPainter proxyPainter(proxy);
    proxyPainter.setCompositionMode(QPainter::CompositionMode_DestinationOut);
    proxyPainter.resetTransform();
    proxyPainter.drawImage(QRect(0, 0, mask.width(), mask.height()), mask);
}

void QSvgNode::applyBufferToCanvas(QPainter *p, QImage proxy) const
{
    QTransform xf = p->transform();
    p->resetTransform();
    p->drawImage(QRect(proxy.offset(), proxy.size()), proxy);
    p->setTransform(xf);
}

bool QSvgNode::isDescendantOf(const QSvgNode *parent) const
{
    const QSvgNode *n = this;
    while (n) {
        if (n == parent)
            return true;
        n = n->m_parent;
    }
    return false;
}

void QSvgNode::appendStyleProperty(QSvgStyleProperty *prop, const QString &id)
{
    //qDebug()<<"appending "<<prop->type()<< " ("<< id <<") "<<"to "<<this<<this->type();
    QSvgDocument *doc;
    switch (prop->type()) {
    case QSvgStyleProperty::QUALITY:
        m_style.quality = static_cast<QSvgQualityStyle*>(prop);
        break;
    case QSvgStyleProperty::FILL:
        m_style.fill = static_cast<QSvgFillStyle*>(prop);
        break;
    case QSvgStyleProperty::VIEWPORT_FILL:
        m_style.viewportFill = static_cast<QSvgViewportFillStyle*>(prop);
        break;
    case QSvgStyleProperty::FONT:
        m_style.font = static_cast<QSvgFontStyle*>(prop);
        break;
    case QSvgStyleProperty::STROKE:
        m_style.stroke = static_cast<QSvgStrokeStyle*>(prop);
        break;
    case QSvgStyleProperty::SOLID_COLOR:
        m_style.solidColor = static_cast<QSvgSolidColorStyle*>(prop);
        doc = document();
        if (doc && !id.isEmpty())
            doc->addNamedStyle(id, m_style.solidColor);
        break;
    case QSvgStyleProperty::GRADIENT:
        m_style.gradient = static_cast<QSvgGradientStyle*>(prop);
        doc = document();
        if (doc && !id.isEmpty())
            doc->addNamedStyle(id, m_style.gradient);
        break;
    case QSvgStyleProperty::PATTERN:
        m_style.pattern = static_cast<QSvgPatternStyle*>(prop);
        doc = document();
        if (doc && !id.isEmpty())
            doc->addNamedStyle(id, m_style.pattern);
        break;
    case QSvgStyleProperty::TRANSFORM:
        m_style.transform = static_cast<QSvgTransformStyle*>(prop);
        break;
    case QSvgStyleProperty::OPACITY:
        m_style.opacity = static_cast<QSvgOpacityStyle*>(prop);
        break;
    case QSvgStyleProperty::COMP_OP:
        m_style.compop = static_cast<QSvgCompOpStyle*>(prop);
        break;
    case QSvgStyleProperty::OFFSET:
        m_style.offset = static_cast<QSvgOffsetStyle*>(prop);
        break;
    default:
        qDebug("QSvgNode: Trying to append unknown property!");
        break;
    }
}

void QSvgNode::applyStyle(QPainter *p, QSvgExtraStates &states) const
{
    m_style.apply(p, this, states);
}

/*!
    \internal

    Apply the styles of all parents to the painter and the states.
    The styles are applied from the top level node to the current node.
    This function can be used to set the correct style for a node
    if it's draw function is triggered out of the ordinary draw context,
    for example the mask node, that is cross-referenced.
*/
void QSvgNode::applyStyleRecursive(QPainter *p, QSvgExtraStates &states) const
{
    if (parent())
        parent()->applyStyleRecursive(p, states);
    applyStyle(p, states);
}

void QSvgNode::revertStyle(QPainter *p, QSvgExtraStates &states) const
{
    m_style.revert(p, states);
}

void QSvgNode::revertStyleRecursive(QPainter *p, QSvgExtraStates &states) const
{
    revertStyle(p, states);
    if (parent())
        parent()->revertStyleRecursive(p, states);
}

void QSvgNode::applyAnimatedStyle(QPainter *p, QSvgExtraStates &states) const
{
    if (document()->animated())
        m_animatedStyle.apply(p, this, states);
}

void QSvgNode::revertAnimatedStyle(QPainter *p, QSvgExtraStates &states) const
{
    if (document()->animated())
        m_animatedStyle.revert(p, states);
}

QSvgStyleProperty * QSvgNode::styleProperty(QSvgStyleProperty::Type type) const
{
    const QSvgNode *node = this;
    while (node) {
        switch (type) {
        case QSvgStyleProperty::QUALITY:
            if (node->m_style.quality)
                return node->m_style.quality;
            break;
        case QSvgStyleProperty::FILL:
            if (node->m_style.fill)
                return node->m_style.fill;
            break;
        case QSvgStyleProperty::VIEWPORT_FILL:
            if (m_style.viewportFill)
                return node->m_style.viewportFill;
            break;
        case QSvgStyleProperty::FONT:
            if (node->m_style.font)
                return node->m_style.font;
            break;
        case QSvgStyleProperty::STROKE:
            if (node->m_style.stroke)
                return node->m_style.stroke;
            break;
        case QSvgStyleProperty::SOLID_COLOR:
            if (node->m_style.solidColor)
                return node->m_style.solidColor;
            break;
        case QSvgStyleProperty::GRADIENT:
            if (node->m_style.gradient)
                return node->m_style.gradient;
            break;
        case QSvgStyleProperty::PATTERN:
            if (node->m_style.pattern)
                return node->m_style.pattern;
            break;
        case QSvgStyleProperty::TRANSFORM:
            if (node->m_style.transform)
                return node->m_style.transform;
            break;
        case QSvgStyleProperty::OPACITY:
            if (node->m_style.opacity)
                return node->m_style.opacity;
            break;
        case QSvgStyleProperty::COMP_OP:
            if (node->m_style.compop)
                return node->m_style.compop;
            break;
        default:
            break;
        }
        node = node->parent();
    }

    return 0;
}

QSvgPaintStyleProperty *QSvgNode::styleProperty(QStringView id) const
{
    if (id.startsWith(QLatin1Char('#')))
        id.slice(1);
    QSvgDocument *doc = document();
    return doc ? doc->namedStyle(id.toString()) : 0;
}

QRectF QSvgNode::internalFastBounds(QPainter *p, QSvgExtraStates &states) const
{
    return internalBounds(p, states);
}

QRectF QSvgNode::internalBounds(QPainter *, QSvgExtraStates &) const
{
    return QRectF(0, 0, 0, 0);
}

QRectF QSvgNode::bounds() const
{
    if (!m_cachedBounds.isEmpty())
        return m_cachedBounds;

    QImage dummy(1, 1, QImage::Format_RGB32);
    QPainter p(&dummy);
    initPainter(&p);
    QSvgExtraStates states;

    if (parent())
        parent()->applyStyleRecursive(&p, states);
    p.setWorldTransform(QTransform());
    m_cachedBounds = bounds(&p, states);
    if (parent()) // always revert the style to not store old transformations
        parent()->revertStyleRecursive(&p, states);
    return m_cachedBounds;
}

QSvgDocument * QSvgNode::document() const
{
    QSvgDocument *doc = nullptr;
    QSvgNode *node = const_cast<QSvgNode*>(this);
    while (node && node->type() != QSvgNode::Doc) {
        node = node->parent();
    }
    doc = static_cast<QSvgDocument*>(node);

    return doc;
}

QString QSvgNode::typeName() const
{
    switch (type()) {
        case Doc: return QStringLiteral("svg");
        case Group: return QStringLiteral("g");
        case Defs: return QStringLiteral("defs");
        case Switch: return QStringLiteral("switch");
        case AnimateColor: return QStringLiteral("animateColor");
        case AnimateTransform: return QStringLiteral("animateTransform");
        case Circle: return QStringLiteral("circle");
        case Ellipse: return QStringLiteral("ellipse");
        case Image: return QStringLiteral("image");
        case Line: return QStringLiteral("line");
        case Path: return QStringLiteral("path");
        case Polygon: return QStringLiteral("polygon");
        case Polyline: return QStringLiteral("polyline");
        case Rect: return QStringLiteral("rect");
        case Text: return QStringLiteral("text");
        case Textarea: return QStringLiteral("textarea");
        case Tspan: return QStringLiteral("tspan");
        case Use: return QStringLiteral("use");
        case Video: return QStringLiteral("video");
        case Mask: return QStringLiteral("mask");
        case Symbol: return QStringLiteral("symbol");
        case Marker: return QStringLiteral("marker");
        case Pattern: return QStringLiteral("pattern");
        case Filter: return QStringLiteral("filter");
        case FeMerge: return QStringLiteral("feMerge");
        case FeMergenode: return QStringLiteral("feMergeNode");
        case FeColormatrix: return QStringLiteral("feColorMatrix");
        case FeGaussianblur: return QStringLiteral("feGaussianBlur");
        case FeOffset: return QStringLiteral("feOffset");
        case FeComposite: return QStringLiteral("feComposite");
        case FeFlood: return QStringLiteral("feFlood");
        case FeBlend: return QStringLiteral("feBlend");
        case FeUnsupported: return QStringLiteral("feUnsupported");
    }
    return QStringLiteral("unknown");
}

void QSvgNode::setRequiredFeatures(const QStringList &lst)
{
    m_requiredFeatures = lst;
}

const QStringList & QSvgNode::requiredFeatures() const
{
    return m_requiredFeatures;
}

void QSvgNode::setRequiredExtensions(const QStringList &lst)
{
    m_requiredExtensions = lst;
}

const QStringList & QSvgNode::requiredExtensions() const
{
    return m_requiredExtensions;
}

void QSvgNode::setRequiredLanguages(const QStringList &lst)
{
    m_requiredLanguages = lst;
}

const QStringList & QSvgNode::requiredLanguages() const
{
    return m_requiredLanguages;
}

void QSvgNode::setRequiredFormats(const QStringList &lst)
{
    m_requiredFormats = lst;
}

const QStringList & QSvgNode::requiredFormats() const
{
    return m_requiredFormats;
}

void QSvgNode::setRequiredFonts(const QStringList &lst)
{
    m_requiredFonts = lst;
}

const QStringList & QSvgNode::requiredFonts() const
{
    return m_requiredFonts;
}

void QSvgNode::setVisible(bool visible)
{
    //propagate visibility change of true to the parent
    //not propagating false is just a small performance
    //degradation since we'll iterate over children without
    //drawing any of them
    if (m_parent && visible && !m_parent->isVisible())
        m_parent->setVisible(true);

    m_visible = visible;
}

QRectF QSvgNode::bounds(QPainter *p, QSvgExtraStates &states) const
{
    applyStyle(p, states);
    QRectF rect = internalBounds(p, states);
    revertStyle(p, states);
    return rect;
}

QRectF QSvgNode::decoratedInternalBounds(QPainter *p, QSvgExtraStates &states) const
{
    return filterRegion(internalBounds(p, states));
}

QRectF QSvgNode::decoratedBounds(QPainter *p, QSvgExtraStates &states) const
{
    applyStyle(p, states);
    QRectF rect = decoratedInternalBounds(p, states);
    revertStyle(p, states);
    return rect;
}

void QSvgNode::setNodeId(const QString &i)
{
    m_id = i;
}

void QSvgNode::setXmlClass(const QString &str)
{
    m_class = str;
}

QString QSvgNode::maskId() const
{
    return m_maskId;
}

void QSvgNode::setMaskId(const QString &str)
{
    m_maskId = str;
}

bool QSvgNode::hasMask() const
{
    if (document()->options().testFlag(QtSvg::Tiny12FeaturesOnly))
        return false;
    return !m_maskId.isEmpty();
}

QString QSvgNode::filterId() const
{
    return m_filterId;
}

void QSvgNode::setFilterId(const QString &str)
{
    m_filterId = str;
}

bool QSvgNode::hasFilter() const
{
    if (document()->options().testFlag(QtSvg::Tiny12FeaturesOnly))
        return false;
    return !m_filterId.isEmpty();
}

QString QSvgNode::markerStartId() const
{
    return m_markerStartId;
}

void QSvgNode::setMarkerStartId(const QString &str)
{
    m_markerStartId = str;
}

bool QSvgNode::hasMarkerStart() const
{
    if (document()->options().testFlag(QtSvg::Tiny12FeaturesOnly))
        return false;
    return !m_markerStartId.isEmpty();
}

QString QSvgNode::markerMidId() const
{
    return m_markerMidId;
}

void QSvgNode::setMarkerMidId(const QString &str)
{
    m_markerMidId = str;
}

bool QSvgNode::hasMarkerMid() const
{
    if (document()->options().testFlag(QtSvg::Tiny12FeaturesOnly))
        return false;
    return !m_markerMidId.isEmpty();
}

QString QSvgNode::markerEndId() const
{
    return m_markerEndId;
}

void QSvgNode::setMarkerEndId(const QString &str)
{
    m_markerEndId = str;
}

bool QSvgNode::hasMarkerEnd() const
{
    if (document()->options().testFlag(QtSvg::Tiny12FeaturesOnly))
        return false;
    return !m_markerEndId.isEmpty();
}

bool QSvgNode::hasAnyMarker() const
{
    if (document()->options().testFlag(QtSvg::Tiny12FeaturesOnly))
        return false;
    return hasMarkerStart() || hasMarkerMid() || hasMarkerEnd();
}

bool QSvgNode::requiresGroupRendering() const
{
    return false;
}

void QSvgNode::setDisplayMode(DisplayMode mode)
{
    m_displayMode = mode;
}

QSvgNode::DisplayMode QSvgNode::displayMode() const
{
    return m_displayMode;
}

qreal QSvgNode::strokeWidth(QPainter *p)
{
    const QPen &pen = p->pen();
    if (pen.style() == Qt::NoPen || pen.brush().style() == Qt::NoBrush || pen.isCosmetic())
        return 0;
    return pen.widthF();
}

void QSvgNode::initPainter(QPainter *p)
{
    QPen pen(Qt::NoBrush, 1, Qt::SolidLine, Qt::FlatCap, Qt::SvgMiterJoin);
    pen.setMiterLimit(4);
    p->setPen(pen);
    p->setBrush(Qt::black);
    p->setRenderHint(QPainter::Antialiasing);
    p->setRenderHint(QPainter::SmoothPixmapTransform);
    QFont font(p->font());
    if (font.pointSize() < 0 && font.pixelSize() > 0) {
        font.setPointSizeF(font.pixelSize() * 72.0 / p->device()->logicalDpiY());
        p->setFont(font);
    }
}

QRectF QSvgNode::boundsOnStroke(QPainter *p, const QPainterPath &path,
                                qreal width, BoundsMode mode)
{
    QPainterPathStroker stroker;
    stroker.setWidth(width);
    if (mode == BoundsMode::IncludeMiterLimit) {
        stroker.setJoinStyle(p->pen().joinStyle());
        stroker.setMiterLimit(p->pen().miterLimit());
    }
    QPainterPath stroke = stroker.createStroke(path);
    return p->transform().map(stroke).boundingRect();
}

bool QSvgNode::shouldDrawNode(QPainter *p, QSvgExtraStates &states) const
{
    if (m_displayMode == DisplayMode::NoneMode)
        return false;

    if (document() && states.trustedSource)
        return true;

    QRectF brect = internalFastBounds(p, states);
    if (brect.width() <= QT_SVG_SIZE_LIMIT && brect.height() <= QT_SVG_SIZE_LIMIT) {
        return true;
    } else {
        qCWarning(lcSvgDraw) << "Shape of type" << type() << "ignored because it will take too long to rasterize (bounding rect=" << brect << ")."
                             << "Enable AssumeTrustedSource in QSvgHandler or set QT_SVG_DEFAULT_OPTIONS=2 to disable this check.";
        return false;
    }
}

QRectF QSvgNode::filterRegion(QRectF bounds) const
{
    QSvgFilterContainer *filterNode = hasFilter()
            ? static_cast<QSvgFilterContainer*>(document()->namedNode(filterId()))
            : nullptr;

    if (filterNode && filterNode->type() == QSvgNode::Filter && filterNode->supported())
        return filterNode->filterRegion(bounds);

    return bounds;
}

QT_END_NAMESPACE
