// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QCDISTANCEFIELDGLYPHCACHE_P_H
#define QCDISTANCEFIELDGLYPHCACHE_P_H

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

#include "engine/qcpainterengineutils_p.h"
#include "engine/qcrhidistancefieldglyphcache_p.h"
#include <QFont>
#include <QRawFont>

QT_BEGIN_NAMESPACE

class QCDistanceFieldGlyphCache
{
public:
    // A FontKey map holding 2 values: The previous QRhiTexture * if the texture is updated, and the glyph cache with the
    // new texture. Currently there is one texture each
    struct FontKeyData
    {
        QRhiTexture *prevTextureState = nullptr;
        QCRhiDistanceFieldGlyphCache *nativeGlyphCache = nullptr;
        QRawFont rawFont;
    };

    struct FontKey
    {
        FontKey(const QRawFont &font);

        QFontEngine::FaceId faceId;
        QFont::Style style;
        int weight;
        QString familyName;
        QString styleName;
    };

    struct GlyphCacheKey
    {
        FontKey fontKey;
        QString text;
        QCanvasPainter::TextAlign textAlign;
        QCanvasPainter::WrapMode wrapMode;
        QCanvasPainter::TextDirection textDirection;
        float pixelSize;
        float lineWidth;
        float lineHeight;
        float letterSpacing;
        float wordSpacing;
    };

    struct GlyphCacheValue
    {
        QList<QGlyphRun> runs;
        QRectF rect;
        int usageCounter = 0;
    };

    QCDistanceFieldGlyphCache(QRhi *rhi);
    ~QCDistanceFieldGlyphCache();

    void generate(const QString &text, const QRectF &rect, const QFont &font, QCState *state, QCanvasPainter::TextAlign alignment,
                  QCRhiDistanceFieldGlyphCache::VertexList *verts, QCRhiDistanceFieldGlyphCache::IndexList *indices);

    void commitResourceUpdates(QRhiResourceUpdateBatch *batch);

    QRhiTexture *getCurrentTextures(const FontKey &key) const;
    QRhiTexture *getOldTextures(const FontKey &key) const;

    void setOldTexture(FontKey key, QRhiTexture *tex);
    void optimizeCache();

private:
    QList<QGlyphRun> generateGlyphRuns(const QString &text, const QRectF &rect,
                                       const QFont &font, const QFontMetrics &metrics,
                                       QCState *state, QCanvasPainter::TextAlign alignment);
    QHash<QCDistanceFieldGlyphCache::FontKey, FontKeyData> m_glyphCaches;
    QRhi *m_rhi;
    QTextLayout m_layout;
#ifdef QCPAINTER_CACHE_GLYPH_RUNS
    QHash<GlyphCacheKey, GlyphCacheValue> m_glyphRunCache;
#endif

private:
    friend bool operator==(const FontKey &f1, const FontKey &f2);
    friend size_t qHash(const FontKey &f, size_t seed);

};

inline bool operator==(
    const QCDistanceFieldGlyphCache::FontKey &f1, const QCDistanceFieldGlyphCache::FontKey &f2)
{
    return f1.faceId == f2.faceId && f1.style == f2.style && f1.weight == f2.weight
           && f1.familyName == f2.familyName && f1.styleName == f2.styleName;
}

inline size_t qHash(const QCDistanceFieldGlyphCache::FontKey &f, size_t seed = 0)
{
    return qHashMulti(seed, f.faceId, f.familyName, f.styleName, f.style, f.weight);
}

inline bool operator==(
    const QCDistanceFieldGlyphCache::GlyphCacheKey &a, const QCDistanceFieldGlyphCache::GlyphCacheKey &b)
{
    return a.fontKey == b.fontKey && qFuzzyCompare(a.pixelSize, b.pixelSize)
           && a.text == b.text && a.textAlign == b.textAlign && a.textDirection == b.textDirection
           && a.wrapMode == b.wrapMode && qFuzzyCompare(a.lineWidth, b.lineWidth)
           && qFuzzyCompare(a.lineHeight, b.lineHeight)
           && qFuzzyCompare(a.letterSpacing, b.letterSpacing)
           && qFuzzyCompare(a.wordSpacing, b.wordSpacing);
}

inline bool operator!=(
    const QCDistanceFieldGlyphCache::GlyphCacheKey &a, const QCDistanceFieldGlyphCache::GlyphCacheKey &b)
{
    return !(a == b);
}

inline size_t qHash(const QCDistanceFieldGlyphCache::GlyphCacheKey &k, size_t seed = 0)
{
    return qHashMulti(seed, k.fontKey, k.text, k.textAlign, k.textDirection, k.wrapMode, k.lineWidth,
                      k.lineHeight, k.letterSpacing, k.wordSpacing, k.pixelSize);
}

QT_END_NAMESPACE

#endif
