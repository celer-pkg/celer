// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "engine/qctextlayout_p.h"
#include "qcdistancefieldglyphcache_p.h"
#include "qcanvaspainter_p.h"
#include <private/qrawfont_p.h>

QT_BEGIN_NAMESPACE

QCDistanceFieldGlyphCache::QCDistanceFieldGlyphCache(QRhi *rhi /*, QRawFont& font*/)
    : m_rhi(rhi)
{
    m_layout.setCacheEnabled(true);
}

QCDistanceFieldGlyphCache::~QCDistanceFieldGlyphCache()
{
    for (auto &cache : std::as_const(m_glyphCaches)) {
        delete cache.nativeGlyphCache;
        /*delete cache.layout;*/
    }
}

QList<QGlyphRun> QCDistanceFieldGlyphCache::generateGlyphRuns(
    const QString &text, const QRectF &rect, const QFont &font,
    const QFontMetrics &metrics, QCState *state, QCanvasPainter::TextAlign alignment)
{
    QTextOption option;
    option.setFlags(QTextOption::DisableEmojiParsing);
    option.setWrapMode(QCTextLayout::convertToQtWrapMode(state->textWrapMode));
    option.setAlignment(QCTextLayout::convertToQtAlignment(alignment));
    option.setTextDirection(QCTextLayout::convertToQtDirection(state->textDirection));
    m_layout.clearLayout();
    m_layout.setTextOption(option);
    m_layout.setFont(font);
    m_layout.setText(text);
    const int leading = metrics.leading();
    float layoutHeight = 0;
    m_layout.beginLayout();
    while (true) {
        QTextLine line = m_layout.createLine();
        if (!line.isValid())
            break;

        line.setLineWidth(rect.width());
        layoutHeight += leading;
        line.setPosition(QPointF(0, layoutHeight));
        layoutHeight += line.height() + state->textLineHeight;
    }
    m_layout.endLayout();
    return m_layout.glyphRuns();
}

void QCDistanceFieldGlyphCache::generate(const QString &text, const QRectF &rect, const QFont &font, QCState *state, QCanvasPainter::TextAlign alignment,
                                         QCRhiDistanceFieldGlyphCache::VertexList *verts, QCRhiDistanceFieldGlyphCache::IndexList *indices)
{
    // Remove raw fonts
    auto rFont = QRawFont::fromFont(font);
    FontKeyData *data;
    const auto metrics = QFontMetrics(font);

    // Get glyph cache
    QCRhiDistanceFieldGlyphCache *cache;
    FontKey key = {rFont};
    if (m_glyphCaches.contains(key)) {
        data = &m_glyphCaches[key];
        cache = data->nativeGlyphCache;
    } else {
        cache = new QCRhiDistanceFieldGlyphCache(m_rhi);
        FontKeyData f{ nullptr, cache, {} };
        m_glyphCaches.insert(key, std::move(f));
        data = &m_glyphCaches[key];
    }

    QList<QGlyphRun> glyphRuns;
    float textY = 0;
#ifdef QCPAINTER_CACHE_GLYPH_RUNS
    const GlyphCacheKey hashKey {
        key,
        text,
        state->textAlignment,
        state->textWrapMode,
        state->textDirection,
        float(rFont.pixelSize()),
        float(rect.width()),
        float(state->textLineHeight),
        float(font.letterSpacing()),
        float(font.wordSpacing())
    };

    if (m_glyphRunCache.contains(hashKey)) {
        // Matching QGlyphRuns exists
        auto &c = m_glyphRunCache[hashKey];
        glyphRuns = c.runs;
        textY = QCTextLayout::calculateVerticalAlignment(state->textBaseline, rect, metrics, c.rect);
        c.usageCounter++;
    } else {
        // Not in cache yet, generate
        glyphRuns = generateGlyphRuns(text, rect,
                                      font, metrics,
                                      state, alignment);
        textY = QCTextLayout::calculateVerticalAlignment(state->textBaseline, rect, metrics, m_layout.boundingRect());
        if (m_layout.cacheEnabled())
            m_glyphRunCache.insert(hashKey, { glyphRuns, m_layout.boundingRect(), 0 });
    }
#else
    // QGlyphRuns caching disabled, generate
    glyphRuns = generateGlyphRuns(text, rect,
                                  font, metrics,
                                  state, alignment);
    textY = QCTextLayout::calculateVerticalAlignment(state->textBaseline, rect, metrics, m_layout.boundingRect());
#endif

    // use clear(), so that the containers' allocations are potentially kept
    verts->clear();
    indices->clear();

    const QPointF glyphPos(rect.x(), rect.y() + textY);
    for (const auto &run : std::as_const(glyphRuns)) {
        cache->setRawFont(run.rawFont());
        cache->addGlyphs(glyphPos, run);
        cache->update();

        // TODO: Add proper bounding box
        QRectF box{};
        cache->generateVertices(verts, indices, state->transform, &box);
    }
}

void QCDistanceFieldGlyphCache::commitResourceUpdates(QRhiResourceUpdateBatch *batch)
{
    for (auto it = m_glyphCaches.begin(); it != m_glyphCaches.end(); ++it)
        it.value().nativeGlyphCache->commitResourceUpdate(batch);
}

QRhiTexture *QCDistanceFieldGlyphCache::getCurrentTextures(const FontKey &key) const
{
    if (!m_glyphCaches.contains(key))
        return nullptr;

    const auto l = m_glyphCaches.value(key).nativeGlyphCache->getTextures();

    if (l.size() == 0)
        return nullptr;

    return l.constFirst().texture;
}

QRhiTexture *QCDistanceFieldGlyphCache::getOldTextures(const FontKey &key) const
{
    if (!m_glyphCaches.contains(key))
        return nullptr;

    return m_glyphCaches.value(key).prevTextureState;
}

void QCDistanceFieldGlyphCache::setOldTexture(FontKey key, QRhiTexture *tex)
{
    if (!m_glyphCaches.contains(key))
        return;

    m_glyphCaches[key].prevTextureState = tex;
}

void QCDistanceFieldGlyphCache::optimizeCache()
{
#ifdef QCPAINTER_CACHE_GLYPH_RUNS
#ifdef QCPAINTER_GLYPH_RUNS_CACHE_SIZE
    const int maxGlyphRuns = QCPAINTER_GLYPH_RUNS_CACHE_SIZE;
#else
    const int maxGlyphRuns = 1024;
#endif
    if (m_glyphRunCache.size() > maxGlyphRuns) {
        int eraseCount = 0;
        // First remove the ones which have been used only few times.
        // These can come when e.g. animating letterSpacing.
        erase_if(m_glyphRunCache, [&eraseCount](const QHash<GlyphCacheKey, GlyphCacheValue>::iterator it) {
            if (it->usageCounter < 5) {
                eraseCount++;
                return true;
            }
            return false;
        });
        // In the rare case where this isn't enough, clear the whole cache.
        const int cleanedMaxGlyphRuns = 0.9 * maxGlyphRuns;
        if (m_glyphRunCache.size() > cleanedMaxGlyphRuns) {
            eraseCount += m_glyphRunCache.size();
            m_glyphRunCache.clear();
        }
        qCDebug(QC_INFO) << "Max text glyph cache size" << maxGlyphRuns
                         << "reached. Removing" << eraseCount << "elements.";
    }
#endif
}

QCDistanceFieldGlyphCache::FontKey::FontKey(const QRawFont &font)
{
    QFontEngine *fe = QRawFontPrivate::get(font)->fontEngine;
    if (fe != nullptr)
        faceId = fe->faceId();
    style = font.style();
    weight = font.weight();
    if (faceId.filename.isEmpty()) {
        familyName = font.familyName();
        styleName = font.styleName();
    }
}

QT_END_NAMESPACE
