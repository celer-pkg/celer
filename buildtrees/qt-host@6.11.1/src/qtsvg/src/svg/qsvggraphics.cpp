// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qsvggraphics_p.h"
#include "qsvgstructure_p.h"
#include "qsvgfont_p.h"

#include <qabstracttextdocumentlayout.h>
#include <qdebug.h>
#include <qloggingcategory.h>
#include <qpainter.h>
#include <qscopedvaluerollback.h>
#include <qtextcursor.h>
#include <qtextdocument.h>
#include <private/qfixed_p.h>

#include <QElapsedTimer>
#include <QLoggingCategory>

#include <math.h>

QT_BEGIN_NAMESPACE

Q_LOGGING_CATEGORY(lcSvgDraw, "qt.svg.draw")

#ifndef QT_SVG_MAX_LAYOUT_SIZE
#define QT_SVG_MAX_LAYOUT_SIZE (qint64(QFIXED_MAX / 2))
#endif

void QSvgDummyNode::drawCommand(QPainter *, QSvgExtraStates &)
{
    qWarning("Dummy node not meant to be drawn");
}

QSvgEllipse::QSvgEllipse(QSvgNode *parent, const QRectF &rect)
    : QSvgNode(parent), m_bounds(rect)
{
}

QRectF QSvgEllipse::internalFastBounds(QPainter *p, QSvgExtraStates &) const
{
    return p->transform().mapRect(m_bounds);
}

QRectF QSvgEllipse::internalBounds(QPainter *p, QSvgExtraStates &) const
{
    QPainterPath path;
    path.addEllipse(m_bounds);
    qreal sw = strokeWidth(p);
    return qFuzzyIsNull(sw) ? p->transform().map(path).boundingRect()
                            : boundsOnStroke(p, path, sw, BoundsMode::Simplistic);
}

QRectF QSvgEllipse::decoratedInternalBounds(QPainter *p, QSvgExtraStates &) const
{
    QPainterPath path;
    path.addEllipse(m_bounds);
    qreal sw = strokeWidth(p);
    QRectF rect = qFuzzyIsNull(sw) ? p->transform().map(path).boundingRect()
                                   : boundsOnStroke(p, path, sw, BoundsMode::IncludeMiterLimit);
    return filterRegion(rect);
}

void QSvgEllipse::drawCommand(QPainter *p, QSvgExtraStates &)
{
    p->drawEllipse(m_bounds);
}

bool QSvgEllipse::separateFillStroke(const QPainter *, const QSvgExtraStates &) const
{
    return true;
}

QSvgImage::QSvgImage(QSvgNode *parent,
                     const QImage &image,
                     const QString &filename,
                     const QRectF &bounds)
    : QSvgNode(parent)
    , m_filename(filename)
    , m_image(image)
    , m_bounds(bounds)
{
    if (m_bounds.width() == 0.0)
        m_bounds.setWidth(static_cast<qreal>(m_image.width()));
    if (m_bounds.height() == 0.0)
        m_bounds.setHeight(static_cast<qreal>(m_image.height()));
}

void QSvgImage::drawCommand(QPainter *p, QSvgExtraStates &)
{
    p->drawImage(m_bounds, m_image);
}

QSvgLine::QSvgLine(QSvgNode *parent, const QLineF &line)
    : QSvgNode(parent), m_line(line)
{
}

void QSvgLine::drawCommand(QPainter *p, QSvgExtraStates &states)
{
    if (p->pen().widthF() != 0) {
        qreal oldOpacity = p->opacity();
        p->setOpacity(oldOpacity * states.strokeOpacity);
        if (m_line.isNull() && p->pen().capStyle() != Qt::FlatCap)
            p->drawPoint(m_line.p1());
        else
            p->drawLine(m_line);
        p->setOpacity(oldOpacity);
    }
    QSvgMarker::drawMarkersForNode(this, p, states);
}

QSvgPath::QSvgPath(QSvgNode *parent, const QPainterPath &qpath)
    : QSvgNode(parent), m_path(qpath)
{
}

void QSvgPath::drawCommand(QPainter *p, QSvgExtraStates &states)
{
    const qreal oldOpacity = p->opacity();
    const bool drawingInOnePass = !separateFillStroke(p, states);
    if (drawingInOnePass)
        p->setOpacity(oldOpacity * states.fillOpacity);
    m_path.setFillRule(states.fillRule);
    if (m_path.boundingRect().isNull() && p->pen().capStyle() != Qt::FlatCap)
        p->drawPoint(m_path.boundingRect().topLeft());
    else
        p->drawPath(m_path);
    if (!path().isEmpty())
        QSvgMarker::drawMarkersForNode(this, p, states);
    if (drawingInOnePass)
        p->setOpacity(oldOpacity);
}

bool QSvgPath::separateFillStroke(const QPainter *p, const QSvgExtraStates &s) const
{
    return !qFuzzyCompare(s.fillOpacity, s.strokeOpacity)
            || qFuzzyIsNull(p->pen().widthF());
}

QRectF QSvgPath::internalFastBounds(QPainter *p, QSvgExtraStates &) const
{
    return p->transform().mapRect(m_path.controlPointRect());
}

QRectF QSvgPath::internalBounds(QPainter *p, QSvgExtraStates &) const
{
    qreal sw = strokeWidth(p);
    return qFuzzyIsNull(sw) ? p->transform().map(m_path).boundingRect()
                            : boundsOnStroke(p, m_path, sw, BoundsMode::Simplistic);
}

QRectF QSvgPath::decoratedInternalBounds(QPainter *p, QSvgExtraStates &s) const
{
    qreal sw = strokeWidth(p);
    QRectF rect = qFuzzyIsNull(sw) ? p->transform().map(m_path).boundingRect()
                                   : boundsOnStroke(p, m_path, sw, BoundsMode::IncludeMiterLimit);
    rect |= QSvgMarker::markersBoundsForNode(this, p, s);
    return filterRegion(rect);
}

bool QSvgPath::requiresGroupRendering() const
{
    return hasAnyMarker();
}

QSvgPolygon::QSvgPolygon(QSvgNode *parent, const QPolygonF &poly)
    : QSvgNode(parent), m_poly(poly)
{
}

QRectF QSvgPolygon::internalFastBounds(QPainter *p, QSvgExtraStates &) const
{
    return p->transform().mapRect(m_poly.boundingRect());
}

QRectF QSvgPolygon::internalBounds(QPainter *p, QSvgExtraStates &s) const
{
    return internalBounds(p, s, BoundsMode::Simplistic);
}

QRectF QSvgPolygon::decoratedInternalBounds(QPainter *p, QSvgExtraStates &s) const
{
    QRectF rect = internalBounds(p, s, BoundsMode::IncludeMiterLimit);
    rect |= QSvgMarker::markersBoundsForNode(this, p, s);
    return filterRegion(rect);
}

bool QSvgPolygon::requiresGroupRendering() const
{
    return hasAnyMarker();
}

QRectF QSvgPolygon::internalBounds(QPainter *p, QSvgExtraStates &, BoundsMode mode) const
{
    qreal sw = strokeWidth(p);
    if (qFuzzyIsNull(sw)) {
        return p->transform().map(m_poly).boundingRect();
    } else {
        QPainterPath path;
        path.addPolygon(m_poly);
        return boundsOnStroke(p, path, sw, mode);
    }
}

void QSvgPolygon::drawCommand(QPainter *p, QSvgExtraStates &states)
{
    if (m_poly.boundingRect().isNull() && p->pen().capStyle() != Qt::FlatCap)
        p->drawPoint(m_poly.first());
    else
        p->drawPolygon(m_poly, states.fillRule);
    QSvgMarker::drawMarkersForNode(this, p, states);
}

bool QSvgPolygon::separateFillStroke(const QPainter *, const QSvgExtraStates &) const
{
    return true;
}

QSvgPolyline::QSvgPolyline(QSvgNode *parent, const QPolygonF &poly)
    : QSvgNode(parent), m_poly(poly)
{

}

void QSvgPolyline::drawCommand(QPainter *p, QSvgExtraStates &states)
{
    if (p->brush().style() != Qt::NoBrush) {
        p->drawPolygon(m_poly, states.fillRule);
    } else {
        if (m_poly.boundingRect().isNull() && p->pen().capStyle() != Qt::FlatCap)
            p->drawPoint(m_poly.first());
        else
            p->drawPolyline(m_poly);
        QSvgMarker::drawMarkersForNode(this, p, states);
    }
}

bool QSvgPolyline::separateFillStroke(const QPainter *, const QSvgExtraStates &) const
{
    return true;
}

QSvgRect::QSvgRect(QSvgNode *node, const QRectF &rect, qreal rx, qreal ry)
    : QSvgNode(node),
      m_rect(rect), m_rx(rx), m_ry(ry)
{
}

QRectF QSvgRect::internalFastBounds(QPainter *p, QSvgExtraStates &) const
{
    return p->transform().mapRect(m_rect);
}

QRectF QSvgRect::internalBounds(QPainter *p, QSvgExtraStates &s) const
{
    return internalBounds(p, s, BoundsMode::Simplistic);
}

QRectF QSvgRect::decoratedInternalBounds(QPainter *p, QSvgExtraStates &s) const
{
    return filterRegion(internalBounds(p, s, BoundsMode::IncludeMiterLimit));
}

QRectF QSvgRect::internalBounds(QPainter *p, QSvgExtraStates &, BoundsMode mode) const
{
    qreal sw = strokeWidth(p);
    if (qFuzzyIsNull(sw)) {
        return p->transform().mapRect(m_rect);
    } else {
        QPainterPath path;
        path.addRect(m_rect);
        return boundsOnStroke(p, path, sw, mode);
    }
}

void QSvgRect::drawCommand(QPainter *p, QSvgExtraStates &)
{
    if (m_rx || m_ry)
        p->drawRoundedRect(m_rect, m_rx, m_ry, Qt::RelativeSize);
    else
        p->drawRect(m_rect);
}

bool QSvgRect::separateFillStroke(const QPainter *, const QSvgExtraStates &) const
{
    return true;
}

QSvgTspan * const QSvgText::LINEBREAK = 0;

QSvgText::QSvgText(QSvgNode *parent, const QPointF &coord)
    : QSvgNode(parent)
    , m_coord(coord)
    , m_size(0, 0)
    , m_type(Text)
    , m_mode(Default)
{
}

QSvgText::~QSvgText()
{
    for (int i = 0; i < m_tspans.size(); ++i) {
        if (m_tspans[i] != LINEBREAK)
            delete m_tspans[i];
    }
}

void QSvgText::setTextArea(const QSizeF &size)
{
    m_size = size;
    m_type = Textarea;
}

QRectF QSvgText::internalFastBounds(QPainter *p, QSvgExtraStates &) const
{
    QFont font = m_style.font ? m_style.font->qfont() : p->font();
    QFontMetricsF fm(font);

    int charCount = 0;
    for (int i = 0; i < m_tspans.size(); ++i) {
        if (m_tspans.at(i) != LINEBREAK)
            charCount += m_tspans.at(i)->text().size();
    }

    QRectF approxMaximumBrect(m_coord.x(),
                              m_coord.y(),
                              charCount * fm.averageCharWidth(),
                              -m_tspans.size() * fm.height());
    return p->transform().mapRect(approxMaximumBrect);
}

QRectF QSvgText::internalBounds(QPainter *p, QSvgExtraStates &states) const
{
    QRectF boundingRect;
    if (shouldDrawNode(p, states))
        draw_helper(p, states, &boundingRect);
    return p->transform().mapRect(boundingRect);
}

void QSvgText::drawCommand(QPainter *p, QSvgExtraStates &states)
{
    draw_helper(p, states);
}

bool QSvgText::shouldDrawNode(QPainter *p, QSvgExtraStates &) const
{
    qsizetype numChars = 0;
    qreal originalFontSize = p->font().pointSizeF();
    qreal maxFontSize = originalFontSize;
    for (const QSvgTspan *span : std::as_const(m_tspans)) {
        if (span == LINEBREAK)
            continue;

        numChars += span->text().size();

        QSvgFontStyle *style = static_cast<QSvgFontStyle *>(span->styleProperty(QSvgStyleProperty::FONT));
        if (style != nullptr && style->qfont().pointSizeF() > maxFontSize)
            maxFontSize = style->qfont().pointSizeF();
    }

    QFont font = p->font();
    font.setPixelSize(maxFontSize);
    QFontMetricsF fm(font);
    if (m_tspans.size() * fm.height() >= QT_SVG_MAX_LAYOUT_SIZE) {
        qCWarning(lcSvgDraw) << "Text element too high to lay out, ignoring";
        return false;
    }

    if (numChars * fm.maxWidth() >= QT_SVG_MAX_LAYOUT_SIZE) {
        qCWarning(lcSvgDraw) << "Text element too wide to lay out, ignoring";
        return false;
    }

    return true;
}

bool QSvgText::separateFillStroke(const QPainter *, const QSvgExtraStates &) const
{
    return true;
}

void QSvgText::draw_helper(QPainter *p, QSvgExtraStates &states, QRectF *boundingRect) const
{
    const bool isPainting = (boundingRect == nullptr);
    if (!isPainting || shouldDrawNode(p, states)) {
        QFont font = p->font();
        Qt::Alignment alignment = states.textAnchor;

        qreal y = 0;
        bool initial = true;
        qreal px = m_coord.x();
        qreal py = m_coord.y();

        if (m_type == Textarea) {
            if (alignment == Qt::AlignHCenter)
                px += m_size.width() / 2;
            else if (alignment == Qt::AlignRight)
                px += m_size.width();
        }

        QRectF bounds;
        if (m_size.height() != 0)
            bounds = QRectF(0, py, 1, m_size.height()); // x and width are not used.

        bool appendSpace = false;
        QStringList paragraphs;
        QList<QList<QTextLayout::FormatRange> > formatRanges(1);
        paragraphs.push_back(QString());

        for (int i = 0; i < m_tspans.size(); ++i) {
            if (m_tspans[i] == LINEBREAK) {
                if (m_type == Textarea) {
                    if (paragraphs.back().isEmpty()) {
                        font.setPixelSize(font.pointSizeF());
                        font.setHintingPreference(QFont::PreferNoHinting);

                        QTextLayout::FormatRange range;
                        range.start = 0;
                        range.length = 1;
                        range.format.setFont(font);
                        formatRanges.back().append(range);

                        paragraphs.back().append(QLatin1Char(' '));;
                    }
                    appendSpace = false;
                    paragraphs.push_back(QString());
                    formatRanges.resize(formatRanges.size() + 1);
                }
            } else {
                WhitespaceMode mode = m_tspans[i]->whitespaceMode();
                m_tspans[i]->applyStyle(p, states);

                font = p->font();
                font.setPixelSize(font.pointSizeF());
                font.setHintingPreference(QFont::PreferNoHinting);

                QString newText(m_tspans[i]->text());
                newText.replace(QLatin1Char('\t'), QLatin1Char(' '));
                newText.replace(QLatin1Char('\n'), QLatin1Char(' '));

                bool prependSpace = !appendSpace && !m_tspans[i]->isTspan() && (mode == Default) && !paragraphs.back().isEmpty() && newText.startsWith(QLatin1Char(' '));
                if (appendSpace || prependSpace)
                    paragraphs.back().append(QLatin1Char(' '));

                bool appendSpaceNext = (!m_tspans[i]->isTspan() && (mode == Default) && newText.endsWith(QLatin1Char(' ')));

                if (mode == Default) {
                    newText = newText.simplified();
                    if (newText.isEmpty())
                        appendSpaceNext = false;
                }

                QTextLayout::FormatRange range;
                range.start = paragraphs.back().size();
                range.length = newText.size();
                range.format.setFont(font);
                if (p->pen().style() != Qt::NoPen && p->pen().brush() != Qt::NoBrush)
                    range.format.setTextOutline(p->pen());
                // work around QTBUG-136696: NoBrush fills the foreground with the outline's pen
                range.format.setForeground(p->brush().style() == Qt::NoBrush
                                           ? QColor(Qt::transparent) : p->brush());

                if (appendSpace) {
                    Q_ASSERT(!formatRanges.back().isEmpty());
                    ++formatRanges.back().back().length;
                } else if (prependSpace) {
                    --range.start;
                    ++range.length;
                }
                formatRanges.back().append(range);

                appendSpace = appendSpaceNext;
                paragraphs.back() += newText;

                m_tspans[i]->revertStyle(p, states);
            }
        }

        if (states.svgFont) {
            // SVG fonts not fully supported...
            if (!m_glyphsToDraw)
                m_glyphsToDraw = states.svgFont->toGlyphs(paragraphs.join(QLatin1Char('\n')));
            if (isPainting) {
                states.svgFont->draw(p, m_coord, m_glyphsToDraw.value(),
                                     p->font().pointSizeF(), states.textAnchor);
            } else {
                *boundingRect = states.svgFont->boundingRect(p, m_coord, m_glyphsToDraw.value(),
                                                             p->font().pointSizeF(), states.textAnchor);
            }
        } else {
            QRectF brect;
            for (int i = 0; i < paragraphs.size(); ++i) {
                QTextLayout tl(paragraphs[i]);
                QTextOption op = tl.textOption();
                op.setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
                tl.setTextOption(op);
                tl.setFormats(formatRanges[i]);
                tl.beginLayout();

                forever {
                    QTextLine line = tl.createLine();
                    if (!line.isValid())
                        break;
                    if (m_size.width() != 0)
                        line.setLineWidth(m_size.width());
                }
                tl.endLayout();

                bool endOfBoundsReached = false;
                for (int i = 0; i < tl.lineCount(); ++i) {
                    QTextLine line = tl.lineAt(i);

                    qreal x = 0;
                    if (alignment == Qt::AlignHCenter)
                        x -= 0.5 * line.naturalTextWidth();
                    else if (alignment == Qt::AlignRight)
                        x -= line.naturalTextWidth();

                    if (initial && m_type == Text)
                        y -= line.ascent();
                    initial = false;

                    line.setPosition(QPointF(x, y));
                    brect |= line.naturalTextRect();

                    // Check if the current line fits into the bounding rectangle.
                    if ((m_size.width() != 0 && line.naturalTextWidth() > m_size.width())
                        || (m_size.height() != 0 && y + line.height() > m_size.height())) {
                        // I need to set the bounds height to 'y-epsilon' to avoid drawing the current
                        // line. Since the font is scaled to 100 units, 1 should be a safe epsilon.
                        bounds.setHeight(y - 1);
                        endOfBoundsReached = true;
                        break;
                    }

                    y += 1.1 * line.height();
                }
                if (isPainting)
                    tl.draw(p, QPointF(px, py), QList<QTextLayout::FormatRange>(), bounds);

                if (endOfBoundsReached)
                    break;
            }
            if (boundingRect) {
                brect.translate(m_coord);
                if (bounds.height() > 0)
                    brect.setBottom(qMin(brect.bottom(), bounds.bottom()));
                *boundingRect = brect;
            }
        }
    }
}

void QSvgText::addText(QStringView text)
{
    m_tspans.append(new QSvgTspan(this, false));
    m_tspans.back()->setWhitespaceMode(m_mode);
    m_tspans.back()->addText(text);
}

QSvgUse::QSvgUse(const QPointF &start, QSvgNode *parent, QSvgNode *node)
    : QSvgNode(parent), m_link(node), m_start(start), m_recursing(false)
{

}

void QSvgUse::drawCommand(QPainter *p, QSvgExtraStates &states)
{
    if (Q_UNLIKELY(!m_link || isDescendantOf(m_link) || m_recursing))
        return;

    Q_ASSERT(states.nestedUseCount == 0 || states.nestedUseLevel > 0);
    if (states.nestedUseLevel > 3 && states.nestedUseCount > (256 + states.nestedUseLevel * 2)) {
        qCDebug(lcSvgDraw, "Too many nested use nodes at #%s!", qPrintable(m_linkId));
        return;
    }

    QScopedValueRollback<bool> inUseGuard(states.inUse, true);

    if (!m_start.isNull()) {
        p->translate(m_start);
    }
    if (states.nestedUseLevel > 0)
        ++states.nestedUseCount;
    {
        QScopedValueRollback<int> useLevelGuard(states.nestedUseLevel, states.nestedUseLevel + 1);
        QScopedValueRollback<bool> recursingGuard(m_recursing, true);
        m_link->draw(p, states);
    }
    if (states.nestedUseLevel == 0)
        states.nestedUseCount = 0;

    if (!m_start.isNull()) {
        p->translate(-m_start);
    }
}

QSvgNode::Type QSvgDummyNode::type() const
{
    return FeUnsupported;
}

QSvgNode::Type QSvgCircle::type() const
{
    return Circle;
}

QSvgNode::Type QSvgEllipse::type() const
{
    return Ellipse;
}

QSvgNode::Type QSvgImage::type() const
{
    return Image;
}

QSvgNode::Type QSvgLine::type() const
{
    return Line;
}

QSvgNode::Type QSvgPath::type() const
{
    return Path;
}

QSvgNode::Type QSvgPolygon::type() const
{
    return Polygon;
}

QSvgNode::Type QSvgPolyline::type() const
{
    return Polyline;
}

QSvgNode::Type QSvgRect::type() const
{
    return Rect;
}

QSvgNode::Type QSvgText::type() const
{
    return m_type;
}

QSvgNode::Type QSvgUse::type() const
{
    return Use;
}

QSvgNode::Type QSvgVideo::type() const
{
    return Video;
}

QRectF QSvgUse::internalBounds(QPainter *p, QSvgExtraStates &states) const
{
    QRectF bounds;
    if (Q_LIKELY(m_link && !isDescendantOf(m_link) && !m_recursing)) {
        QScopedValueRollback<bool> guard(m_recursing, true);
        p->translate(m_start);
        bounds = m_link->bounds(p, states);
        p->translate(-m_start);
    }
    return bounds;
}

QRectF QSvgUse::decoratedInternalBounds(QPainter *p, QSvgExtraStates &states) const
{
    QRectF bounds;
    if (Q_LIKELY(m_link && !isDescendantOf(m_link) && !m_recursing)) {
        QScopedValueRollback<bool> guard(m_recursing, true);
        p->translate(m_start);
        bounds = m_link->decoratedBounds(p, states);
        p->translate(-m_start);
    }
    return bounds;
}

QRectF QSvgPolyline::internalFastBounds(QPainter *p, QSvgExtraStates &) const
{
    return p->transform().mapRect(m_poly.boundingRect());
}

QRectF QSvgPolyline::internalBounds(QPainter *p, QSvgExtraStates &s) const
{
    return internalBounds(p, s, BoundsMode::Simplistic);
}

QRectF QSvgPolyline::decoratedInternalBounds(QPainter *p, QSvgExtraStates &s) const
{
    QRectF rect = internalBounds(p, s, BoundsMode::IncludeMiterLimit);
    rect |= QSvgMarker::markersBoundsForNode(this, p, s);
    return filterRegion(rect);
}

bool QSvgPolyline::requiresGroupRendering() const
{
    return hasAnyMarker();
}

QRectF QSvgPolyline::internalBounds(QPainter *p, QSvgExtraStates &, BoundsMode mode) const
{
    qreal sw = strokeWidth(p);
    if (qFuzzyIsNull(sw)) {
        return p->transform().map(m_poly).boundingRect();
    } else {
        QPainterPath path;
        path.addPolygon(m_poly);
        return boundsOnStroke(p, path, sw, mode);
    }
}

QRectF QSvgImage::internalBounds(QPainter *p, QSvgExtraStates &) const
{
    return p->transform().mapRect(m_bounds);
}

QRectF QSvgLine::internalFastBounds(QPainter *p, QSvgExtraStates &) const
{
    QPointF p1 = p->transform().map(m_line.p1());
    QPointF p2 = p->transform().map(m_line.p2());
    qreal minX = qMin(p1.x(), p2.x());
    qreal minY = qMin(p1.y(), p2.y());
    qreal maxX = qMax(p1.x(), p2.x());
    qreal maxY = qMax(p1.y(), p2.y());
    return QRectF(minX, minY, maxX - minX, maxY - minY);
}

QRectF QSvgLine::internalBounds(QPainter *p, QSvgExtraStates &s) const
{
    return  internalBounds(p, s, BoundsMode::Simplistic);
}

QRectF QSvgLine::decoratedInternalBounds(QPainter *p, QSvgExtraStates &s) const
{
    QRectF rect = internalBounds(p, s, BoundsMode::IncludeMiterLimit);
    rect |= QSvgMarker::markersBoundsForNode(this, p, s);
    return filterRegion(rect);
}

bool QSvgLine::requiresGroupRendering() const
{
    return hasAnyMarker();
}

QRectF QSvgLine::internalBounds(QPainter *p, QSvgExtraStates &s, BoundsMode mode) const
{
    qreal sw = strokeWidth(p);
    if (qFuzzyIsNull(sw)) {
        return internalFastBounds(p, s);
    } else {
        QPainterPath path;
        path.moveTo(m_line.p1());
        path.lineTo(m_line.p2());
        return boundsOnStroke(p, path, sw, mode);
    }
}

QT_END_NAMESPACE
