// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qsvgfont_p.h"

#include "qpainter.h"
#include "qpen.h"
#include "qdebug.h"

QT_BEGIN_NAMESPACE

QSvgGlyph::QSvgGlyph(const QString &unicode, const QPainterPath &path, qreal horizAdvX)
    : m_unicode(unicode), m_path(path), m_horizAdvX(horizAdvX)
{

}


QSvgFont::QSvgFont(qreal horizAdvX)
    : m_horizAdvX(horizAdvX)
{
}


QString QSvgFont::familyName() const
{
    return m_familyName;
}


void QSvgFont::addGlyph(const QString &unicode, const QPainterPath &path, qreal horizAdvX)
{
    m_glyphs.emplaceBack(unicode, path, (horizAdvX == -1) ? m_horizAdvX : horizAdvX);
}

bool QSvgFont::addMissingGlyph(const QPainterPath &path, qreal horizAdvX)
{
    if (m_missingGlyph) {
        qWarning("The font already has a 'missing-glyph' element.");
        return false;
    }
    m_missingGlyph.reset(new QSvgGlyph(QChar(), path, (horizAdvX == -1) ? m_horizAdvX : horizAdvX));
    return true;
}


void QSvgFont::draw(QPainter *p, const QPointF &point, const QList<const QSvgGlyph *> &glyphs,
                    qreal pixelSize, Qt::Alignment alignment) const
{
    draw_helper(p, point, glyphs, pixelSize, alignment, nullptr);
}

QRectF QSvgFont::boundingRect(QPainter *p, const QPointF &point,
                              const QList<const QSvgGlyph *> &glyphs,
                              qreal pixelSize, Qt::Alignment alignment) const
{
    QRectF bounds;
    draw_helper(p, point, glyphs, pixelSize, alignment, &bounds);
    return bounds;
}

// text must not be empty
char32_t firstUcs4(QStringView text)
{
    if (text.size() > 1 && text.at(0).isHighSurrogate() && text.at(1).isLowSurrogate())
        return QChar::surrogateToUcs4(text.at(0), text.at(1));

    return text.first().unicode();
}

// returns a pointer to the first QSvgGlyph to be used in the text
// or nullptr if none can be found
const QSvgGlyph *QSvgFont::findFirstGlyphFor(QStringView text) const
{
    const auto possibleIndices = m_possibleGlyphIndicesForChar.value(firstUcs4(text));
    for (const qsizetype i : possibleIndices) {
        const QSvgGlyph &currentGlyph = m_glyphs.at(i);
        if (text.startsWith(currentGlyph.m_unicode)) {
            return &currentGlyph;
        }
    }

    for (; m_firstUnscannedGlyphIdx < m_glyphs.size(); ++m_firstUnscannedGlyphIdx) {
        const QSvgGlyph &currentGlyph = m_glyphs.at(m_firstUnscannedGlyphIdx);
        m_possibleGlyphIndicesForChar[firstUcs4(currentGlyph.m_unicode)].push_back(m_firstUnscannedGlyphIdx);
        if (text.startsWith(currentGlyph.m_unicode)) {
            ++m_firstUnscannedGlyphIdx;
            return &currentGlyph;
        }
    }
    return nullptr;
}

QList<const QSvgGlyph *> QSvgFont::toGlyphs(QStringView text) const
{
    QList<const QSvgGlyph *> glyphs;
    glyphs.reserve(text.length());
    while (text.length()) {
        const QSvgGlyph *foundGlyph = findFirstGlyphFor(text);
        if (foundGlyph) {
            glyphs.append(foundGlyph);
            text.slice(foundGlyph->m_unicode.length());
        } else {
            if (m_missingGlyph)
                glyphs.append(m_missingGlyph.get());
            if (text.size() > 1
                && text.at(0).isHighSurrogate() && text.at(1).isLowSurrogate()) {
                text.slice(2);
            } else {
                text.slice(1);
            }
        }
    }
    return glyphs;
}

void QSvgFont::draw_helper(QPainter *p, const QPointF &point,
                           const QList<const QSvgGlyph *> &glyphs, qreal pixelSize,
                           Qt::Alignment alignment, QRectF *boundingRect) const
{
    const bool isPainting = (boundingRect == nullptr);

    p->save();
    p->translate(point);
    p->scale(pixelSize / m_unitsPerEm, -pixelSize / m_unitsPerEm);

    // Calculate the text width to be used for alignment
    int textWidth = 0;
    for (const auto *glyph : glyphs)
        textWidth += static_cast<int>(glyph->m_horizAdvX);

    QPoint alignmentOffset(0, 0);
    if (alignment == Qt::AlignHCenter) {
        alignmentOffset.setX(-textWidth / 2);
    } else if (alignment == Qt::AlignRight) {
        alignmentOffset.setX(-textWidth);
    }

    p->translate(alignmentOffset);

    // since in SVG the embedded font ain't really a path
    // the outline has got to stay untransformed...
    qreal penWidth = p->pen().widthF();
    penWidth /= (pixelSize/m_unitsPerEm);
    QPen pen = p->pen();
    pen.setWidthF(penWidth);
    p->setPen(pen);

    for (const auto *glyph : glyphs) {
        if (isPainting)
            p->drawPath(glyph->m_path);

        if (boundingRect) {
            QPainterPathStroker stroker;
            stroker.setWidth(penWidth);
            stroker.setJoinStyle(p->pen().joinStyle());
            stroker.setMiterLimit(p->pen().miterLimit());
            QPainterPath stroke = stroker.createStroke(glyph->m_path);
            *boundingRect |= p->transform().map(stroke).boundingRect();
        }

        p->translate(glyph->m_horizAdvX, 0);
    }

    p->restore();
}

void QSvgFont::setFamilyName(const QString &name)
{
    m_familyName = name;
}

void QSvgFont::setUnitsPerEm(qreal upem)
{
    m_unitsPerEm = upem;
}

QT_END_NAMESPACE
