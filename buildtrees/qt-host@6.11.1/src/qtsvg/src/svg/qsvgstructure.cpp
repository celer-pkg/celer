// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qsvgstructure_p.h"
#include "qsvggraphics_p.h"

#include "qsvgstyle_p.h"
#include "qsvgdocument_p.h"
#include "qsvggraphics_p.h"
#include "qsvgstyle_p.h"
#include "qsvgfilter_p.h"

#include "qpainter.h"
#include "qlocale.h"
#include "qdebug.h"

#include <QLoggingCategory>
#include <qscopedvaluerollback.h>
#include <QtGui/qimageiohandler.h>

QT_BEGIN_NAMESPACE

QSvgG::QSvgG(QSvgNode *parent)
    : QSvgStructureNode(parent)
{

}

QSvgStructureNode::~QSvgStructureNode()
{
}

void QSvgG::drawCommand(QPainter *p, QSvgExtraStates &states)
{
    for (const auto &node : renderers()) {
        if ((node->isVisible()) && (node->displayMode() != QSvgNode::NoneMode))
            node->draw(p, states);
    }
}

bool QSvgG::shouldDrawNode(QPainter *, QSvgExtraStates &) const
{
    return true;
}

QSvgNode::Type QSvgG::type() const
{
    return Group;
}

bool QSvgG::requiresGroupRendering() const
{
    return m_renderers.size() > 1;
}

QSvgStructureNode::QSvgStructureNode(QSvgNode *parent)
    :QSvgNode(parent)
{

}

QSvgNode * QSvgStructureNode::scopeNode(const QString &id) const
{
    QSvgDocument *doc = document();
    return doc ? doc->namedNode(id) : 0;
}

void QSvgStructureNode::addChild(std::unique_ptr<QSvgNode> child, const QString &id)
{
    if (!id.isEmpty()) {
        QSvgDocument *doc = document();
        if (doc)
            doc->addNamedNode(id, child.get());
    }

    m_renderers.push_back(std::move(child));
}

QSvgDefs::QSvgDefs(QSvgNode *parent)
    : QSvgStructureNode(parent)
{
}

bool QSvgDefs::shouldDrawNode(QPainter *, QSvgExtraStates &) const
{
    return false;
}

QSvgNode::Type QSvgDefs::type() const
{
    return Defs;
}

QSvgSymbolLike::QSvgSymbolLike(QSvgNode *parent, QRectF bounds, QRectF viewBox, QPointF refP,
               QSvgSymbolLike::PreserveAspectRatios pAspectRatios, QSvgSymbolLike::Overflow overflow)
    : QSvgStructureNode(parent)
    , m_rect(bounds)
    , m_viewBox(viewBox)
    , m_refP(refP)
    , m_pAspectRatios(pAspectRatios)
    , m_overflow(overflow)
{

}

QRectF QSvgSymbolLike::decoratedInternalBounds(QPainter *p, QSvgExtraStates &states) const
{
    p->save();
    setPainterToRectAndAdjustment(p);
    QRectF rect = internalBounds(p, states);
    p->restore();
    return rect;
}

bool QSvgSymbolLike::requiresGroupRendering() const
{
    return m_renderers.size() > 1;
}

QRectF QSvgSymbolLike::clipRect() const
{
    if (m_overflow != Overflow::Hidden || !m_viewBox.isValid())
        return QRectF{};

    qreal scaleX = 1.0;
    if (m_rect.width() > 0 && m_viewBox.width() > 0)
        scaleX = m_rect.width() / m_viewBox.width();
    qreal scaleY = 1.0;
    if (m_rect.height() > 0 && m_viewBox.height() > 0)
        scaleY = m_rect.height() / m_viewBox.height();

    QTransform t;
    t.translate(- m_refP.x() * scaleX - m_rect.left() - m_viewBox.left() * scaleX,
                - m_refP.y() * scaleY - m_rect.top() - m_viewBox.top() * scaleY);
    t.scale(scaleX, scaleY);

    return t.mapRect(m_viewBox);
}

QTransform QSvgSymbolLike::aspectRatioTransform() const
{
    QTransform xform;

    qreal offsetX = 0.0;
    qreal offsetY = 0.0;

    qreal scaleX = 1.0;
    if (m_rect.width() > 0 && m_viewBox.width() > 0)
        scaleX = m_rect.width() / m_viewBox.width();
    qreal scaleY = 1.0;
    if (m_rect.height() > 0 && m_viewBox.height() > 0)
        scaleY = m_rect.height() / m_viewBox.height();

    if (!qFuzzyCompare(scaleX, scaleY) &&
        m_pAspectRatios.testAnyFlag(PreserveAspectRatio::xyMask)) {

        if (m_pAspectRatios.testAnyFlag(PreserveAspectRatio::meet))
            scaleX = scaleY = qMin(scaleX, scaleY);
        else
            scaleX = scaleY = qMax(scaleX, scaleY);

        qreal xOverflow = scaleX * m_viewBox.width() - m_rect.width();
        qreal yOverflow = scaleY * m_viewBox.height() - m_rect.height();

        if ((m_pAspectRatios & PreserveAspectRatio::xMask) == PreserveAspectRatio::xMid)
            offsetX -= xOverflow / 2.;
        else if ((m_pAspectRatios & PreserveAspectRatio::xMask) == PreserveAspectRatio::xMax)
            offsetX -= xOverflow;

        if ((m_pAspectRatios & PreserveAspectRatio::yMask) == PreserveAspectRatio::yMid)
            offsetY -= yOverflow / 2.;
        else if ((m_pAspectRatios & PreserveAspectRatio::yMask) == PreserveAspectRatio::yMax)
            offsetY -= yOverflow;
    }

    xform.translate(offsetX - m_refP.x() * scaleX, offsetY - m_refP.y() * scaleY);
    xform.scale(scaleX, scaleY);

    return xform;
}

void QSvgSymbolLike::setPainterToRectAndAdjustment(QPainter *p) const
{
    QRectF clip = clipRect();
    if (!clip.isEmpty())
        p->setClipRect(clip);

    p->setTransform(aspectRatioTransform(), true);
}

QSvgSymbol::QSvgSymbol(QSvgNode *parent, QRectF bounds, QRectF viewBox, QPointF refP,
                       QSvgSymbol::PreserveAspectRatios pAspectRatios,
                       QSvgSymbol::Overflow overflow)
    : QSvgSymbolLike(parent, bounds, viewBox, refP, pAspectRatios, overflow)
{
}

void QSvgSymbol::drawCommand(QPainter *p, QSvgExtraStates &states)
{
    if (!states.inUse) //Symbol is only drawn when within a use node.
        return;

    p->save();
    setPainterToRectAndAdjustment(p);
    for (const auto &node : renderers()) {
        if ((node->isVisible()) && (node->displayMode() != QSvgNode::NoneMode))
            node->draw(p, states);
    }
    p->restore();
}

QSvgNode::Type QSvgSymbol::type() const
{
    return Symbol;
}

QSvgMarker::QSvgMarker(QSvgNode *parent, QRectF bounds, QRectF viewBox, QPointF refP,
                       QSvgSymbol::PreserveAspectRatios pAspectRatios, QSvgSymbol::Overflow overflow,
                       Orientation orientation, qreal orientationAngle, MarkerUnits markerUnits)
    : QSvgSymbolLike(parent, bounds, viewBox, refP, pAspectRatios, overflow)
    , m_orientation(orientation)
    , m_orientationAngle(orientationAngle)
    , m_markerUnits(markerUnits)
{
    // apply the svg standard style
    QSvgFillStyle *fillProp = new QSvgFillStyle();
    fillProp->setBrush(Qt::black);
    appendStyleProperty(fillProp, QStringLiteral(""));

    QSvgStrokeStyle *strokeProp = new QSvgStrokeStyle();
    strokeProp->setMiterLimit(4);
    strokeProp->setWidth(1);
    strokeProp->setLineCap(Qt::FlatCap);
    strokeProp->setLineJoin(Qt::SvgMiterJoin);
    strokeProp->setStroke(Qt::NoBrush);
    appendStyleProperty(strokeProp, QStringLiteral(""));
}

QSvgFilterContainer::QSvgFilterContainer(QSvgNode *parent, const QSvgRectF &bounds,
                                         QtSvg::UnitTypes filterUnits, QtSvg::UnitTypes primitiveUnits)
    : QSvgStructureNode(parent)
    , m_rect(bounds)
    , m_filterUnits(filterUnits)
    , m_primitiveUnits(primitiveUnits)
    , m_supported(true)
{

}

bool QSvgFilterContainer::shouldDrawNode(QPainter *, QSvgExtraStates &) const
{
    return false;
}

void QSvgMarker::drawCommand(QPainter *p, QSvgExtraStates &states)
{
    if (!states.inUse) //Symbol is only drawn in combination with another node.
        return;

    if (Q_UNLIKELY(m_recursing))
        return;
    QScopedValueRollback<bool> recursingGuard(m_recursing, true);

    p->save();
    setPainterToRectAndAdjustment(p);

    for (const auto &node : renderers()) {
        if ((node->isVisible()) && (node->displayMode() != QSvgNode::NoneMode))
            node->draw(p, states);
    }
    p->restore();
}

namespace {

struct PositionMarkerPair {
    qreal x;
    qreal y;
    qreal angle;
    QString markerId;
    bool isStartNode = false;
};

QList<PositionMarkerPair> markersForNode(const QSvgNode *node)
{
    if (!node->hasAnyMarker())
        return {};

    auto getMeanAngle = [](QPointF p0, QPointF p1, QPointF p2) -> qreal {
        QPointF t1 = p1 - p0;
        QPointF t2 = p2 - p1;
        qreal hyp1 =  hypot(t1.x(), t1.y());
        if (hyp1 > 0)
            t1 /= hyp1;
        else
            return 0.;
        qreal hyp2 =  hypot(t2.x(), t2.y());
        if (hyp2 > 0)
            t2 /= hyp2;
        else
            return 0.;
        QPointF tangent = t1 + t2;
        return -atan2(tangent.y(), tangent.x()) / M_PI * 180.;
    };

    QList<PositionMarkerPair> markers;

    switch (node->type()) {
    case QSvgNode::Line: {
        const QSvgLine *line = static_cast<const QSvgLine*>(node);
        if (node->hasMarkerStart())
            markers << PositionMarkerPair { line->line().p1().x(), line->line().p1().y(),
                                            line->line().angle(), line->markerStartId(),
                                            true};
        if (node->hasMarkerEnd())
            markers << PositionMarkerPair { line->line().p2().x(), line->line().p2().y(),
                                            line->line().angle(), line->markerEndId() };
        break;
    }
    case QSvgNode::Polyline:
    case QSvgNode::Polygon: {
        const QPolygonF &polyData = (node->type() == QSvgNode::Polyline)
                ? static_cast<const QSvgPolyline*>(node)->polygon()
                : static_cast<const QSvgPolygon*>(node)->polygon();

        if (node->hasMarkerStart() && polyData.size() > 1) {
            QLineF line(polyData.at(0), polyData.at(1));
            markers << PositionMarkerPair { line.p1().x(),
                                            line.p1().y(),
                                            line.angle(),
                                            node->markerStartId(),
                                            true };
        }
        if (node->hasMarkerMid()) {
            for (int i = 1; i < polyData.size() - 1; i++) {
                QPointF p0 = polyData.at(i - 1);
                QPointF p1 = polyData.at(i);
                QPointF p2 = polyData.at(i + 1);

                markers << PositionMarkerPair { p1.x(),
                                                p1.y(),
                                                getMeanAngle(p0, p1, p2),
                                                node->markerStartId() };
            }
        }
        if (node->hasMarkerEnd() && polyData.size() > 1) {
            QLineF line(polyData.at(polyData.size() - 1), polyData.last());
            markers << PositionMarkerPair { line.p2().x(),
                                            line.p2().y(),
                                            line.angle(),
                                            node->markerEndId() };
        }
        break;
    }
    case QSvgNode::Path: {
        const QSvgPath *path = static_cast<const QSvgPath*>(node);
        if (node->hasMarkerStart())
            markers << PositionMarkerPair { path->path().pointAtPercent(0.).x(),
                                            path->path().pointAtPercent(0.).y(),
                                            path->path().angleAtPercent(0.),
                                            path->markerStartId(),
                                            true };
        if (node->hasMarkerMid()) {
            for (int i = 1; i < path->path().elementCount() - 1; i++) {
                if (path->path().elementAt(i).type == QPainterPath::MoveToElement)
                    continue;
                if (path->path().elementAt(i).type == QPainterPath::CurveToElement)
                    continue;
                if (( path->path().elementAt(i).type == QPainterPath::CurveToDataElement &&
                      path->path().elementAt(i + 1).type != QPainterPath::CurveToDataElement ) ||
                      path->path().elementAt(i).type == QPainterPath::LineToElement) {

                    QPointF p0(path->path().elementAt(i - 1).x, path->path().elementAt(i - 1).y);
                    QPointF p1(path->path().elementAt(i).x, path->path().elementAt(i).y);
                    QPointF p2(path->path().elementAt(i + 1).x, path->path().elementAt(i + 1).y);

                    markers << PositionMarkerPair { p1.x(),
                                                    p1.y(),
                                                    getMeanAngle(p0, p1, p2),
                                                    path->markerMidId() };
                }
            }
        }
        if (node->hasMarkerEnd())
            markers << PositionMarkerPair { path->path().pointAtPercent(1.).x(),
                                            path->path().pointAtPercent(1.).y(),
                                            path->path().angleAtPercent(1.),
                                            path->markerEndId() };
        break;
    }
    default:
        Q_UNREACHABLE();
        break;
    }

    return markers;
}

} // anonymous namespace


void QSvgMarker::drawMarkersForNode(QSvgNode *node, QPainter *p, QSvgExtraStates &states)
{
    drawHelper(node, p, states);
}

QRectF QSvgMarker::markersBoundsForNode(const QSvgNode *node, QPainter *p, QSvgExtraStates &states)
{
    QRectF bounds;
    drawHelper(node, p, states, &bounds);
    return bounds;
}

QSvgNode::Type QSvgMarker::type() const
{
    return Marker;
}

void QSvgMarker::drawHelper(const QSvgNode *node, QPainter *p,
                            QSvgExtraStates &states, QRectF *boundingRect)
{
    QScopedValueRollback<bool> inUseGuard(states.inUse, true);

    const bool isPainting = (boundingRect == nullptr);
    const auto markers = markersForNode(node);
    for (auto &i : markers) {
        QSvgNode *referencedNode = node->document()->namedNode(i.markerId);
        if (!referencedNode || referencedNode->type() != QSvgNode::Marker)
            continue;
        QSvgMarker *markNode = static_cast<QSvgMarker *>(referencedNode);

        p->save();
        p->translate(i.x, i.y);
        if (markNode->orientation() == QSvgMarker::Orientation::Value) {
            p->rotate(markNode->orientationAngle());
        } else {
            p->rotate(-i.angle);
            if (i.isStartNode && markNode->orientation()
                        == QSvgMarker::Orientation::AutoStartReverse) {
                p->scale(-1, -1);
            }
        }
        QRectF oldRect = markNode->m_rect;
        if (markNode->markerUnits() == QSvgMarker::MarkerUnits::StrokeWidth) {
            markNode->m_rect.setWidth(markNode->m_rect.width() * p->pen().widthF());
            markNode->m_rect.setHeight(markNode->m_rect.height() * p->pen().widthF());
        }
        if (isPainting)
            markNode->draw(p, states);

        if (boundingRect) {
            QTransform xf = p->transform();
            p->resetTransform();
            *boundingRect |=  xf.mapRect(markNode->decoratedInternalBounds(p, states));
        }

        markNode->m_rect = oldRect;
        p->restore();
    }
}

QImage QSvgFilterContainer::applyFilter(const QImage &buffer, QPainter *p, const QRectF &bounds) const
{
    QRectF localFilterRegion = m_rect.resolveRelativeLengths(bounds, m_filterUnits);
    QRect globalFilterRegion = p->transform().mapRect(localFilterRegion).toRect();
    QRect globalFilterRegionRel = globalFilterRegion.translated(-buffer.offset());

    if (globalFilterRegionRel.isEmpty())
        return buffer;

    QImage proxy;
    if (!QImageIOHandler::allocateImage(globalFilterRegionRel.size(), buffer.format(), &proxy)) {
        qCWarning(lcSvgDraw) << "The requested filter is too big, ignoring";
        return buffer;
    }
    proxy = buffer.copy(globalFilterRegionRel);
    proxy.setOffset(globalFilterRegion.topLeft());
    if (proxy.isNull())
        return buffer;

    QMap<QString, QImage> buffers;
    buffers[QStringLiteral("")] = proxy;
    buffers[QStringLiteral("SourceGraphic")] = proxy;

    bool requiresSourceAlpha = false;

    for (const auto &node : renderers()) {
        const QSvgFeFilterPrimitive *filter = QSvgFeFilterPrimitive::castToFilterPrimitive(node.get());
        if (filter && filter->requiresSourceAlpha()) {
            requiresSourceAlpha = true;
            break;
        }
    }

    if (requiresSourceAlpha) {
        QImage proxyAlpha = proxy.convertedTo(QImage::Format_Alpha8).convertedTo(proxy.format());
        proxyAlpha.setOffset(proxy.offset());
        if (proxyAlpha.isNull())
            return buffer;
        buffers[QStringLiteral("SourceAlpha")] = proxyAlpha;
    }

    QImage result;
    for (const auto &node : renderers()) {
        const QSvgFeFilterPrimitive *filter = QSvgFeFilterPrimitive::castToFilterPrimitive(node.get());
        if (filter) {
            result = filter->apply(buffers, p, bounds, localFilterRegion, m_primitiveUnits, m_filterUnits);
            if (!result.isNull()) {
                buffers[QStringLiteral("")] = result;
                buffers[filter->result()] = result;
            }
        }
    }
    return result;
}

void QSvgFilterContainer::setSupported(bool supported)
{
    m_supported = supported;
}

bool QSvgFilterContainer::supported() const
{
    return m_supported;
}

QRectF QSvgFilterContainer::filterRegion(const QRectF &itemBounds) const
{
    return m_rect.resolveRelativeLengths(itemBounds, m_filterUnits);
}

QSvgNode::Type QSvgFilterContainer::type() const
{
    return Filter;
}


inline static bool isSupportedSvgFeature(const QString &str)
{
    static const QStringList wordList = {
        QStringLiteral("http://www.w3.org/Graphics/SVG/feature/1.2/#Text"),
        QStringLiteral("http://www.w3.org/Graphics/SVG/feature/1.2/#Shape"),
        QStringLiteral("http://www.w3.org/Graphics/SVG/feature/1.2/#SVG"),
        QStringLiteral("http://www.w3.org/Graphics/SVG/feature/1.2/#Structure"),
        QStringLiteral("http://www.w3.org/Graphics/SVG/feature/1.2/#SolidColor"),
        QStringLiteral("http://www.w3.org/Graphics/SVG/feature/1.2/#Hyperlinking"),
        QStringLiteral("http://www.w3.org/Graphics/SVG/feature/1.2/#CoreAttribute"),
        QStringLiteral("http://www.w3.org/Graphics/SVG/feature/1.2/#XlinkAttribute"),
        QStringLiteral("http://www.w3.org/Graphics/SVG/feature/1.2/#SVG-static"),
        QStringLiteral("http://www.w3.org/Graphics/SVG/feature/1.2/#OpacityAttribute"),
        QStringLiteral("http://www.w3.org/Graphics/SVG/feature/1.2/#Gradient"),
        QStringLiteral("http://www.w3.org/Graphics/SVG/feature/1.2/#Font"),
        QStringLiteral("http://www.w3.org/Graphics/SVG/feature/1.2/#Image"),
        QStringLiteral("http://www.w3.org/Graphics/SVG/feature/1.2/#ConditionalProcessing"),
        QStringLiteral("http://www.w3.org/Graphics/SVG/feature/1.2/#Extensibility"),
        QStringLiteral("http://www.w3.org/Graphics/SVG/feature/1.2/#GraphicsAttribute"),
        QStringLiteral("http://www.w3.org/Graphics/SVG/feature/1.2/#Prefetch"),
        QStringLiteral("http://www.w3.org/Graphics/SVG/feature/1.2/#PaintAttribute"),
        QStringLiteral("http://www.w3.org/Graphics/SVG/feature/1.2/#ConditionalProcessingAttribute"),
        QStringLiteral("http://www.w3.org/Graphics/SVG/feature/1.2/#ExternalResourcesRequiredAttribute")
    };

    return wordList.contains(str);
}

static inline bool isSupportedSvgExtension(const QString &)
{
    return false;
}


QSvgSwitch::QSvgSwitch(QSvgNode *parent)
    : QSvgStructureNode(parent)
{
    init();
}

QSvgNode *QSvgSwitch::childToRender() const
{
    auto itr = m_renderers.begin();

    for (const auto &node : renderers()) {
        if (node->isVisible() && (node->displayMode() != QSvgNode::NoneMode)) {
            const QStringList &features  = node->requiredFeatures();
            const QStringList &extensions = node->requiredExtensions();
            const QStringList &languages = node->requiredLanguages();
            const QStringList &formats = node->requiredFormats();
            const QStringList &fonts = node->requiredFonts();

            bool okToRender = true;
            if (!features.isEmpty()) {
                QStringList::const_iterator sitr = features.constBegin();
                for (; sitr != features.constEnd(); ++sitr) {
                    if (!isSupportedSvgFeature(*sitr)) {
                        okToRender = false;
                        break;
                    }
                }
            }

            if (okToRender && !extensions.isEmpty()) {
                QStringList::const_iterator sitr = extensions.constBegin();
                for (; sitr != extensions.constEnd(); ++sitr) {
                    if (!isSupportedSvgExtension(*sitr)) {
                        okToRender = false;
                        break;
                    }
                }
            }

            if (okToRender && !languages.isEmpty()) {
                QStringList::const_iterator sitr = languages.constBegin();
                okToRender = false;
                for (; sitr != languages.constEnd(); ++sitr) {
                    if ((*sitr).startsWith(m_systemLanguagePrefix)) {
                        okToRender = true;
                        break;
                    }
                }
            }

            if (okToRender && !formats.isEmpty())
                okToRender = false;

            if (okToRender && !fonts.isEmpty())
                okToRender = false;

            if (okToRender)
                return node.get();
        }

        ++itr;
    }

    return nullptr;
}

void QSvgSwitch::drawCommand(QPainter *p, QSvgExtraStates &states)
{
    QSvgNode *node = childToRender();
    if (node != nullptr)
        node->draw(p, states);
}

QSvgNode::Type QSvgSwitch::type() const
{
    return Switch;
}

void QSvgSwitch::init()
{
    QLocale locale;
    m_systemLanguage = locale.name().replace(QLatin1Char('_'), QLatin1Char('-'));
    int idx = m_systemLanguage.indexOf(QLatin1Char('-'));
    m_systemLanguagePrefix = m_systemLanguage.mid(0, idx);
}

QRectF QSvgStructureNode::internalBounds(QPainter *p, QSvgExtraStates &states) const
{
    QRectF bounds;
    if (!m_recursing) {
        QScopedValueRollback<bool> guard(m_recursing, true);
        for (const auto &node : renderers())
            bounds |= node->bounds(p, states);
    }
    return bounds;
}

QRectF QSvgStructureNode::decoratedInternalBounds(QPainter *p, QSvgExtraStates &states) const
{
    QRectF bounds;
    if (!m_recursing) {
        QScopedValueRollback<bool> guard(m_recursing, true);
        for (const auto &node : renderers())
            bounds |= node->decoratedBounds(p, states);
    }
    return bounds;
}

QSvgNode* QSvgStructureNode::previousSiblingNode(QSvgNode *n) const
{
    QSvgNode *prev = nullptr;
    for (const auto &node : renderers()) {
        if (node.get() == n)
            return prev;
        prev = node.get();
    }
    return prev;
}

QSvgMask::QSvgMask(QSvgNode *parent, QSvgRectF bounds,
                   QtSvg::UnitTypes contentUnits)
    : QSvgStructureNode(parent)
    , m_rect(bounds)
    , m_contentUnits(contentUnits)
{
}

bool QSvgMask::shouldDrawNode(QPainter *, QSvgExtraStates &) const
{
    return false;
}

QImage QSvgMask::createMask(QPainter *p, QSvgExtraStates &states, QSvgNode *targetNode, QRectF *globalRect) const
{
    QTransform t = p->transform();
    p->resetTransform();
    QRectF basicRect = targetNode->internalBounds(p, states);
    *globalRect = t.mapRect(basicRect);
    p->setTransform(t);
    return createMask(p, states, basicRect, globalRect);
}

QImage QSvgMask::createMask(QPainter *p, QSvgExtraStates &states, const QRectF &localRect, QRectF *globalRect) const
{
    QRect imageBound = globalRect->toAlignedRect();
    *globalRect = imageBound.toRectF();

    QImage mask;
    if (!QImageIOHandler::allocateImage(imageBound.size(), QImage::Format_RGBA8888, &mask)) {
        qCWarning(lcSvgDraw) << "The requested mask size is too big, ignoring";
        return mask;
    }

    if (Q_UNLIKELY(m_recursing))
        return mask;
    QScopedValueRollback<bool> recursingGuard(m_recursing, true);

    // Chrome seems to return the mask of the mask if a mask is set on the mask
    if (this->hasMask()) {
        QSvgNode *referencedNode = document()->namedNode(this->maskId());
        if (referencedNode && referencedNode->type() == QSvgNode::Mask) {
            QSvgMask *maskNode = static_cast<QSvgMask *>(referencedNode);
            QRectF boundsRect;
            return maskNode->createMask(p, states, localRect, &boundsRect);
        }
    }

    // The mask is created with other elements during rendering.
    // Black pixels are masked out, white pixels are not masked.
    // The strategy is to draw the elements in a buffer (QImage) and to map
    // the white-black image into a transparent-white image that can be used
    // with QPainters composition mode to set the mask.

    mask.fill(Qt::transparent);
    QPainter painter(&mask);
    initPainter(&painter);

    QSvgExtraStates maskNodeStates;
    applyStyleRecursive(&painter, maskNodeStates);

    // The transformation of the mask node is not relevant. What matters are the contentUnits
    // and the position/scale of the node that the mask is applied to.
    painter.resetTransform();
    painter.translate(-imageBound.topLeft());
    painter.setTransform(p->transform(), true);

    QTransform oldT = painter.transform();
    if (m_contentUnits == QtSvg::UnitTypes::objectBoundingBox){
        painter.translate(localRect.topLeft());
        painter.scale(localRect.width(), localRect.height());
    }

    // Draw all content items of the mask to generate the mask
    for (const auto &node : renderers())
    {
        if ((node->isVisible()) && (node->displayMode() != QSvgNode::NoneMode))
            node->draw(&painter, maskNodeStates);
    }

    for (int i=0; i < mask.height(); i++) {
        QRgb *line = reinterpret_cast<QRgb *>(mask.scanLine(i));
        for (int j=0; j < mask.width(); j++) {
            const qreal rC = 0.2125, gC = 0.7154, bC = 0.0721; //luminanceToAlpha times alpha following SVG 1.1
            int alpha = 255 - (qRed(line[j]) * rC + qGreen(line[j]) * gC + qBlue(line[j]) * bC) * qAlpha(line[j])/255.;
            line[j] = qRgba(0, 0, 0, alpha);
        }
    }

    // Make a path out of the clipRectangle and draw it inverted - black over all content items.
    // This is required to apply a clip rectangle with transformations.
    // painter.setClipRect(clipRect) sounds like the obvious thing to do but
    // created artifacts due to antialiasing.
    QRectF clipRect = m_rect.resolveRelativeLengths(localRect);
    QPainterPath clipPath;
    clipPath.setFillRule(Qt::OddEvenFill);
    clipPath.addRect(mask.rect().adjusted(-10, -10, 20, 20));
    clipPath.addPolygon(oldT.map(QPolygonF(clipRect)));
    painter.resetTransform();
    painter.fillPath(clipPath, Qt::black);
    revertStyleRecursive(&painter, maskNodeStates);
    return mask;
}

QSvgNode::Type QSvgMask::type() const
{
    return Mask;
}

QSvgPattern::QSvgPattern(QSvgNode *parent, QSvgRectF bounds, QRectF viewBox,
                         QtSvg::UnitTypes contentUnits, QTransform transform)
    : QSvgStructureNode(parent),
    m_rect(bounds),
    m_viewBox(viewBox),
    m_contentUnits(contentUnits),
    m_isRendering(false),
    m_transform(transform)

{

}

bool QSvgPattern::shouldDrawNode(QPainter *, QSvgExtraStates &) const
{
    return false;
}

static QImage& defaultPattern()
{
    static QImage checkerPattern;

    if (checkerPattern.isNull()) {
        checkerPattern = QImage(QSize(8, 8), QImage::Format_ARGB32);
        QPainter p(&checkerPattern);
        p.fillRect(QRect(0, 0, 4, 4), QColorConstants::Svg::white);
        p.fillRect(QRect(4, 0, 4, 4), QColorConstants::Svg::black);
        p.fillRect(QRect(0, 4, 4, 4), QColorConstants::Svg::black);
        p.fillRect(QRect(4, 4, 4, 4), QColorConstants::Svg::white);
    }

    return checkerPattern;
}

QImage QSvgPattern::patternImage(QPainter *p, QSvgExtraStates &states, const QSvgNode *patternElement)
{
    // pe stands for Pattern Element
    QRectF peBoundingBox;
    QRectF peWorldBoundingBox;

    QTransform t = p->transform();
    p->resetTransform();
    peBoundingBox = patternElement->internalBounds(p, states);
    peWorldBoundingBox = t.mapRect(peBoundingBox);
    p->setTransform(t);

    // This function renders the pattern into an Image, so we need to apply the correct
    // scaling values when we draw the pattern. The scaling is affected by two factors :
    //      - The "patternTransform" attribute which itself might contain a scaling
    //      - The scaling applied globally.
    // The first is obtained from m11 and m22 matrix elements,
    // while the second is calculated by dividing the patternElement global size
    // by its local size.
    qreal contentScaleFactorX = m_transform.m11();
    qreal contentScaleFactorY = m_transform.m22();
    if (m_contentUnits == QtSvg::UnitTypes::userSpaceOnUse) {
        contentScaleFactorX *= t.m11();
        contentScaleFactorY *= t.m22();
    } else {
        contentScaleFactorX *= peWorldBoundingBox.width();
        contentScaleFactorY *= peWorldBoundingBox.height();
    }

    // Calculate the pattern bounding box depending on the used UnitTypes
    QRectF patternBoundingBox = m_rect.resolveRelativeLengths(peBoundingBox);

    QSize imageSize;
    imageSize.setWidth(qCeil(patternBoundingBox.width() * t.m11() * m_transform.m11()));
    imageSize.setHeight(qCeil(patternBoundingBox.height() * t.m22() * m_transform.m22()));
    if (imageSize.isEmpty())
        return QImage(); // Avoid division by zero in calculateAppliedTransform()

    calculateAppliedTransform(t, peBoundingBox, imageSize);
    if (document()->isCalculatingImplicitViewBox())
        return QImage(imageSize, QImage::Format_ARGB32); // dummy image to avoid endless recursion
    else
        return renderPattern(imageSize, contentScaleFactorX, contentScaleFactorY);
}

QSvgNode::Type QSvgPattern::type() const
{
    return Pattern;
}

QImage QSvgPattern::renderPattern(QSize size, qreal contentScaleX, qreal contentScaleY)
{
    if (size.isEmpty() || !qIsFinite(contentScaleX) || !qIsFinite(contentScaleY))
        return defaultPattern();

    // Allocate a QImage to draw the pattern in with the calculated size.
    QImage pattern;
    if (!QImageIOHandler::allocateImage(size, QImage::Format_ARGB32, &pattern)) {
        qCWarning(lcSvgDraw) << "The requested pattern size is too big, ignoring";
        return defaultPattern();
    }
    pattern.fill(Qt::transparent);

    if (m_isRendering) {
        qCWarning(lcSvgDraw) << "The pattern is trying to render itself recursively. "
                                "Returning a transparent QImage of the right size.";
        return pattern;
    }
    QScopedValueRollback<bool> guard(m_isRendering, true);

    // Draw the pattern using our QPainter.
    QPainter patternPainter(&pattern);
    QSvgExtraStates patternStates;
    initPainter(&patternPainter);
    applyStyleRecursive(&patternPainter, patternStates);
    patternPainter.resetTransform();

    // According to the <pattern> definition, if viewBox exists then patternContentUnits
    // is ignored
    if (m_viewBox.isNull())
        patternPainter.scale(contentScaleX, contentScaleY);
    else
        patternPainter.setWindow(m_viewBox.toRect());

    // Draw all this Pattern children nodes with our QPainter,
    // no need to use any Extra States
    for (const auto &node : renderers())
        node->draw(&patternPainter, patternStates);

    revertStyleRecursive(&patternPainter, patternStates);
    return pattern;
}

void QSvgPattern::calculateAppliedTransform(QTransform &worldTransform, QRectF peLocalBB, QSize imageSize)
{
    // Calculate the required transform to be applied to the QBrush used for correct
    // pattern drawing with the object being rendered.
    // Scale : Apply inverse the scale used above because QBrush uses the transform used
    //         by the QPainter and this function has already rendered the QImage with the
    //         correct size. Moreover, take into account the difference between the required
    //         ideal image size in float and the QSize given to image as an integer value.
    //
    // Translate : Apply translation depending on the calculated x and y values so that the
    //             drawn pattern can be shifted inside the object.
    // Pattern Transform : Apply the transform in the "patternTransform" attribute. This
    //                     transform contains everything except scaling, because it is
    //                     already applied above on the QImage and the QPainter while
    //                     drawing the pattern tile.
    m_appliedTransform.reset();
    qreal imageDownScaleFactorX = 1 / worldTransform.m11();
    qreal imageDownScaleFactorY = 1 / worldTransform.m22();

    m_appliedTransform.scale(qIsFinite(imageDownScaleFactorX) ? imageDownScaleFactorX : 1.0,
                             qIsFinite(imageDownScaleFactorY) ? imageDownScaleFactorY : 1.0);

    QRectF p = m_rect.resolveRelativeLengths(peLocalBB);
    m_appliedTransform.scale((p.width() * worldTransform.m11() * m_transform.m11()) / imageSize.width(),
                             (p.height() * worldTransform.m22() * m_transform.m22()) / imageSize.height());

    QPointF translation = m_rect.translationRelativeToBoundingBox(peLocalBB);
    m_appliedTransform.translate(translation.x() * worldTransform.m11(), translation.y() * worldTransform.m22());

    QTransform scalelessTransform = m_transform;
    scalelessTransform.scale(1 / m_transform.m11(), 1 / m_transform.m22());

    m_appliedTransform = m_appliedTransform * scalelessTransform;
}

QT_END_NAMESPACE
