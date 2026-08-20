// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qcrhidistancefieldglyphcache_p.h"
#include <QtGui/private/qrawfont_p.h>

QT_BEGIN_NAMESPACE

#define RHI_DISTANCEFIELD_GLYPH_CACHE_PADDING 2

QCRhiDistanceFieldGlyphCache::TextureInfo QCRhiDistanceFieldGlyphCache::s_emptyTexture;

QCRhiDistanceFieldGlyphCache::QCRhiDistanceFieldGlyphCache(QRhi *rhi)
    : m_rhi(rhi)
    , m_pendingGlyphs(64)
{
    m_batch = rhi->nextResourceUpdateBatch();
}

QCRhiDistanceFieldGlyphCache::~QCRhiDistanceFieldGlyphCache()
{
    m_batch->release();

    for (auto i = 0; i < m_textures.size(); ++i)
        m_textures[i].texture->deleteLater();

    if (m_areaAllocator != nullptr)
        delete m_areaAllocator;
}

bool QCRhiDistanceFieldGlyphCache::addGlyphs(
    QPointF position, const QGlyphRun &glyphs)
{
    return setGlyphs(position + QPointF(0, glyphs.rawFont().ascent()), glyphs);
}

void QCRhiDistanceFieldGlyphCache::createTexture(TextureInfo *texInfo, int width, int height)
{
    QByteArray zeroBuf(width * height, 0);
    createTexture(texInfo, width, height, zeroBuf.constData());
}

void QCRhiDistanceFieldGlyphCache::createTexture(
    TextureInfo *texInfo, int width, int height, const void *pixels)
{
    if (useTextureResizeWorkaround() && texInfo->image.isNull()) {
        texInfo->image = QDistanceField(width, height);
        memcpy(texInfo->image.bits(), pixels, width * height);
    }

    texInfo->texture = m_rhi->newTexture(
        QRhiTexture::RED_OR_ALPHA8, QSize(width, height), 1, QRhiTexture::UsedAsTransferSource);
    texInfo->texture->setName("QCDistanceFieldAtlas");

    if (texInfo->texture->create()) {
        QRhiTextureSubresourceUploadDescription subresDesc(pixels, width * height);
        subresDesc.setSourceSize(QSize(width, height));
        m_batch->uploadTexture(texInfo->texture, QRhiTextureUploadEntry(0, 0, subresDesc));
    } else {
        qWarning("Failed to create distance field glyph cache");
    }

    texInfo->size = QSize(width, height);
}

void QCRhiDistanceFieldGlyphCache::resizeTexture(TextureInfo *texInfo, int width, int height)
{
    int oldWidth = texInfo->size.width();
    int oldHeight = texInfo->size.height();
    if (width == oldWidth && height == oldHeight)
        return;

    QRhiTexture *oldTexture = texInfo->texture;
    createTexture(texInfo, width, height);

    if (!oldTexture)
        return;

    updateRhiTexture(oldTexture, texInfo->texture, texInfo->size);

    if (useTextureResizeWorkaround()) {
        QRhiTextureSubresourceUploadDescription
            subresDesc(texInfo->image.constBits(), oldWidth * oldHeight);
        subresDesc.setSourceSize(QSize(oldWidth, oldHeight));
        m_batch->uploadTexture(texInfo->texture, QRhiTextureUploadEntry(0, 0, subresDesc));
        texInfo->image = texInfo->image.copy(0, 0, width, height);
    } else {
        m_batch->copyTexture(texInfo->texture, oldTexture);
    }

    oldTexture->deleteLater();
}

bool QCRhiDistanceFieldGlyphCache::setGlyphs(QPointF position, const QGlyphRun &glyphs)
{
    QRawFont font = glyphs.rawFont();
    m_position = QPointF(position.x(), position.y() - font.ascent());
    m_glyphs = glyphs;

    auto populated = populate(glyphs.glyphIndexes());

    const QList<quint32> glyphIndexes = m_glyphs.glyphIndexes();

    return populated;
}

void QCRhiDistanceFieldGlyphCache::setGlyphTexture(
    const QList<glyph_t> &glyphs, const TextureInfo &tex)
{
    int i = m_textures.indexOf(tex);
    if (i == -1) {
        m_textures.append(tex);
        i = m_textures.size() - 1;
    } else {
        m_textures[i].size = tex.size;
    }
    TextureInfo *texture = &(m_textures[i]);

    int count = glyphs.size();

    for (int j = 0; j < count; ++j) {
        glyph_t glyphIndex = glyphs.at(j);
        GlyphData &gd = glyphData(glyphIndex);
        gd.texture = texture;
    }
}

void QCRhiDistanceFieldGlyphCache::generateVertices(
    VertexList *verts,
    IndexList *indices,
    const QTransform &transform,
    QRectF *boundingRect)
{
    if (!verts || !indices)
        return;

    const QList<quint32> indexes = m_glyphs.glyphIndexes();
    const QList<QPointF> positions = m_glyphs.positions();
    qreal fontPixelSize = m_glyphs.rawFont().pixelSize();

    const qsizetype maxIndexCount = (std::numeric_limits<quint16>::max() - 1)
                                    / 4; // 16383 (see below: 0xFFFF is not allowed)
    const auto likelyGlyphCount = qMin(indexes.size(), maxIndexCount);
    verts->reserve(likelyGlyphCount * 4);
    indices->reserve(likelyGlyphCount * 6);

    qreal maxTexMargin = distanceFieldRadius();
    qreal fScale = fontScale(fontPixelSize);
    qreal margin = 2;
    qreal texMargin = margin / fScale;
    if (texMargin > maxTexMargin) {
        texMargin = maxTexMargin;
        margin = maxTexMargin * fScale;
    }

    for (int i = 0; i < indexes.size(); ++i) {
        const int glyphIndex = indexes.at(i);
        TexCoord c = glyphTexCoord(glyphIndex);

        if (c.isNull())
            continue;

        const QPointF position = positions.at(i);
        Metrics metrics = glyphMetrics(glyphIndex, fontPixelSize);

        if (!metrics.isNull() && !c.isNull()) {
            metrics.width += margin * 2;
            metrics.height += margin * 2;
            metrics.baselineX -= margin;
            metrics.baselineY += margin;
            c.xMargin -= texMargin;
            c.yMargin -= texMargin;
            c.width += texMargin * 2;
            c.height += texMargin * 2;
        }

        qreal x = position.x() + metrics.baselineX + m_position.x();
        qreal y = position.y() - metrics.baselineY + m_position.y();

        *boundingRect |= QRectF(x, y, metrics.width, metrics.height);

        float cx1 = x;
        float cx2 = x + metrics.width;
        float cy1 = y;
        float cy2 = y + metrics.height;

        float tx1 = c.x + c.xMargin;
        float tx2 = tx1 + c.width;
        float ty1 = c.y + c.yMargin;
        float ty2 = ty1 + c.height;

        qsizetype baseIndex = verts->size();

        // transform
        auto p1 = QPointF{cx1, cy1};
        auto p2 = QPointF{cx2, cy1};
        auto p3 = QPointF{cx1, cy2};
        auto p4 = QPointF{cx2, cy2};

        if (!transform.isIdentity()) {
            p1 = transform.map(p1);
            p2 = transform.map(p2);
            p3 = transform.map(p3);
            p4 = transform.map(p4);
        }

        TexturedPoint2D v1;
        v1.set(p1.x(), p1.y(), tx1, ty1);
        TexturedPoint2D v2;
        v2.set(p2.x(), p2.y(), tx2, ty1);
        TexturedPoint2D v3;
        v3.set(p3.x(), p3.y(), tx1, ty2);
        TexturedPoint2D v4;
        v4.set(p4.x(), p4.y(), tx2, ty2);

        verts->append(v1);
        verts->append(v2);
        verts->append(v3);
        verts->append(v4);

        indices->append(baseIndex + 0);
        indices->append(baseIndex + 2);
        indices->append(baseIndex + 3);
        indices->append(baseIndex + 3);
        indices->append(baseIndex + 1);
        indices->append(baseIndex + 0);
    }
}

void QCRhiDistanceFieldGlyphCache::commitResourceUpdate(QRhiResourceUpdateBatch *batch)
{
    batch->merge(m_batch);
    m_batch->release();
    m_batch = m_rhi->nextResourceUpdateBatch();
}

int QCRhiDistanceFieldGlyphCache::maxTextureSize() const
{
    return m_maxTextureSize;
}

QCRhiDistanceFieldGlyphCache::GlyphData &QCRhiDistanceFieldGlyphCache::emptyData(glyph_t glyph)
{
    GlyphData gd;
    gd.texture = &s_emptyTexture;
    auto it = m_glyphsData.insert(glyph, gd);
    return it.value();
}

void QCRhiDistanceFieldGlyphCache::markGlyphsToRender(const QList<glyph_t> &glyphs)
{
    int count = glyphs.size();
    for (int i = 0; i < count; ++i)
        m_pendingGlyphs.add(glyphs.at(i));
}

bool QCRhiDistanceFieldGlyphCache::useTextureResizeWorkaround() const
{
    static bool set = false;
    static bool useWorkaround = false;
    if (!set) {
        useWorkaround = m_rhi->backend() == QRhi::OpenGLES2;
        set = true;
    }
    return useWorkaround;
}

void QCRhiDistanceFieldGlyphCache::updateRhiTexture(
    QRhiTexture *oldTex, QRhiTexture *newTex, QSize newTexSize)
{
    int count = m_textures.size();
    for (int i = 0; i < count; ++i) {
        TextureInfo &tex = m_textures[i];
        if (tex.texture == oldTex) {
            tex.texture = newTex;
            tex.size = newTexSize;
            return;
        }
    }
}

QCRhiDistanceFieldGlyphCache::Metrics QCRhiDistanceFieldGlyphCache::glyphMetrics(
    glyph_t glyph, qreal pixelSize)
{
    GlyphData &gd = glyphData(glyph);
    qreal scale = fontScale(pixelSize);

    Metrics m;
    m.width = gd.boundingRect.width() * scale;
    m.height = gd.boundingRect.height() * scale;
    m.baselineX = gd.boundingRect.x() * scale;
    m.baselineY = -gd.boundingRect.y() * scale;

    return m;
}

qreal QCRhiDistanceFieldGlyphCache::fontScale(qreal pixelSize) const
{
    return pixelSize
           / QT_DISTANCEFIELD_BASEFONTSIZE(m_rawFontCache[m_referenceFont].doubleGlyphResolution);
}

bool QCRhiDistanceFieldGlyphCache::populate(const QList<glyph_t> &glyphs)
{
    QSet<glyph_t> referencedGlyphs;
    QSet<glyph_t> newGlyphs;
    int count = glyphs.size();

    const auto glyphCount = m_rawFontCache[m_referenceFont].glyphCount;

    for (int i = 0; i < count; ++i) {
        glyph_t glyphIndex = glyphs.at(i);
        if ((int) glyphIndex >= glyphCount && glyphCount > 0) {
            qWarning("Warning: distance-field glyph is not available with index %d", glyphIndex);
            continue;
        }

        GlyphData &gd = glyphData(glyphIndex);
        ++gd.ref;
        referencedGlyphs.insert(glyphIndex);

        if (gd.texCoord.isValid() || m_populatingGlyphs.contains(glyphIndex))
            continue;

        m_populatingGlyphs.insert(glyphIndex);

        if (gd.boundingRect.isEmpty()) {
            gd.texCoord.width = 0;
            gd.texCoord.height = 0;
        } else {
            newGlyphs.insert(glyphIndex);
        }
    }

    referenceGlyphs(referencedGlyphs);
    if (!newGlyphs.isEmpty())
        requestGlyphs(newGlyphs);

    return newGlyphs.size() > 0;
}

void QCRhiDistanceFieldGlyphCache::update()
{
    m_populatingGlyphs.clear();

    if (m_pendingGlyphs.isEmpty())
        return;

    QList<QDistanceField> distanceFields;
    const int pendingGlyphsSize = m_pendingGlyphs.size();
    distanceFields.reserve(pendingGlyphsSize);
    for (int i = 0; i < pendingGlyphsSize; ++i) {
        GlyphData &gd = glyphData(m_pendingGlyphs.at(i));

        QSize size = QSize(
            qCeil(gd.texCoord.width + gd.texCoord.xMargin * 2),
            qCeil(gd.texCoord.height + gd.texCoord.yMargin * 2));

        distanceFields.append(QDistanceField(
            size,
            gd.path,
            m_pendingGlyphs.at(i),
            m_rawFontCache[m_referenceFont].doubleGlyphResolution));
        gd.path = QPainterPath(); // no longer needed, so release memory used by the painter path
    }

    m_pendingGlyphs.reset();

    storeGlyphs(distanceFields);
}

void QCRhiDistanceFieldGlyphCache::setRawFont(const QRawFont &font)
{
    if (m_referenceFont == font)
        return;

    Q_ASSERT(font.isValid());

    if (!m_rawFontCache.contains(font)) {
        auto refFont = ReferenceFont{};
        refFont.mutatedFont = font;
        QRawFontPrivate *fontD = QRawFontPrivate::get(font);
        refFont.glyphCount = fontD->fontEngine->glyphCount();

        refFont.doubleGlyphResolution = qt_fontHasNarrowOutlines(font)
                                        && refFont.glyphCount < QT_DISTANCEFIELD_HIGHGLYPHCOUNT();

        refFont.pixelSize = QT_DISTANCEFIELD_BASEFONTSIZE(refFont.doubleGlyphResolution)
                            * QT_DISTANCEFIELD_SCALE(refFont.doubleGlyphResolution);

        // we set the same pixel size as used by the distance field internally.
        // this allows us to call pathForGlyph once and reuse the result.
        refFont.mutatedFont.setPixelSize(refFont.pixelSize);
        m_rawFontCache.insert(font, refFont);
    }

    m_referenceFont = font;

    Q_ASSERT(m_referenceFont.isValid());
}

void QCRhiDistanceFieldGlyphCache::requestGlyphs(const QSet<glyph_t> &glyphs)
{
    QList<GlyphPosition> glyphPositions;
    QList<glyph_t> glyphsToRender;

    if (m_areaAllocator == nullptr)
        m_areaAllocator = new QCAreaAllocator(
            QSize(maxTextureSize(), m_maxTextureCount * maxTextureSize()));

    for (QSet<glyph_t>::const_iterator it = glyphs.constBegin(); it != glyphs.constEnd(); ++it) {
        glyph_t glyphIndex = *it;

        int padding = RHI_DISTANCEFIELD_GLYPH_CACHE_PADDING;
        QRectF boundingRect = glyphData(glyphIndex).boundingRect;
        int glyphWidth = qCeil(boundingRect.width() + distanceFieldRadius() * 2);
        int glyphHeight = qCeil(boundingRect.height() + distanceFieldRadius() * 2);
        QSize glyphSize(glyphWidth + padding * 2, glyphHeight + padding * 2);
        QRect alloc = m_areaAllocator->allocate(glyphSize);

        if (alloc.isNull()) {
            // Unallocate unused glyphs until we can allocated the new glyph
            while (alloc.isNull() && !m_unusedGlyphs.isEmpty()) {
                glyph_t unusedGlyph = *m_unusedGlyphs.constBegin();

                TexCoord unusedCoord = glyphTexCoord(unusedGlyph);
                QRectF unusedGlyphBoundingRect = glyphData(unusedGlyph).boundingRect;
                int unusedGlyphWidth = qCeil(
                    unusedGlyphBoundingRect.width() + distanceFieldRadius() * 2);
                int unusedGlyphHeight = qCeil(
                    unusedGlyphBoundingRect.height() + distanceFieldRadius() * 2);
                m_areaAllocator->deallocate(QRect(
                    unusedCoord.x - padding,
                    unusedCoord.y - padding,
                    padding * 2 + unusedGlyphWidth,
                    padding * 2 + unusedGlyphHeight));

                m_unusedGlyphs.remove(unusedGlyph);
                m_glyphsTexture.remove(unusedGlyph);
                removeGlyph(unusedGlyph);

                alloc = m_areaAllocator->allocate(glyphSize);
            }

            // Not enough space left for this glyph... skip to the next one
            if (alloc.isNull())
                continue;
        }

        TextureInfo *tex = textureInfo(alloc.y() / maxTextureSize());
        alloc = QRect(alloc.x(), alloc.y() % maxTextureSize(), alloc.width(), alloc.height());

        tex->allocatedArea |= alloc;
        Q_ASSERT(tex->padding == padding || tex->padding < 0);
        tex->padding = padding;

        GlyphPosition p;
        p.glyph = glyphIndex;
        p.position = alloc.topLeft() + QPoint(padding, padding);

        glyphPositions.append(p);
        glyphsToRender.append(glyphIndex);
        m_glyphsTexture.insert(glyphIndex, tex);
    }

    setGlyphsPosition(glyphPositions);
    markGlyphsToRender(glyphsToRender);
}

void QCRhiDistanceFieldGlyphCache::removeGlyph(glyph_t glyph)
{
    GlyphData &gd = glyphData(glyph);
    gd.texCoord = TexCoord();
    gd.texture = &s_emptyTexture;
}

void QCRhiDistanceFieldGlyphCache::setGlyphsPosition(const QList<GlyphPosition> &glyphs)
{
    QList<quint32> invalidatedGlyphs;

    int count = glyphs.size();

    const auto dgResolution = m_rawFontCache[m_referenceFont].doubleGlyphResolution;

    for (int i = 0; i < count; ++i) {
        GlyphPosition glyph = glyphs.at(i);
        GlyphData &gd = glyphData(glyph.glyph);

        if (!gd.texCoord.isNull())
            invalidatedGlyphs.append(glyph.glyph);

        gd.texCoord.xMargin = QT_DISTANCEFIELD_RADIUS(dgResolution)
                              / qreal(QT_DISTANCEFIELD_SCALE(dgResolution));
        gd.texCoord.yMargin = QT_DISTANCEFIELD_RADIUS(dgResolution)
                              / qreal(QT_DISTANCEFIELD_SCALE(dgResolution));
        gd.texCoord.x = glyph.position.x();
        gd.texCoord.y = glyph.position.y();
        gd.texCoord.width = gd.boundingRect.width();
        gd.texCoord.height = gd.boundingRect.height();
    }
}

void QCRhiDistanceFieldGlyphCache::referenceGlyphs(const QSet<glyph_t> &glyphs)
{
    m_unusedGlyphs -= glyphs;
}

void QCRhiDistanceFieldGlyphCache::storeGlyphs(const QList<QDistanceField> &glyphs)
{
    typedef QHash<TextureInfo *, QList<glyph_t>> GlyphTextureHash;

    GlyphTextureHash glyphTextures;

    QVarLengthArray<QRhiTextureUploadEntry, 32> uploads;
    for (int i = 0; i < glyphs.size(); ++i) {
        QDistanceField glyph = glyphs.at(i);
        glyph_t glyphIndex = glyph.glyph();
        TexCoord c = glyphData(glyphIndex).texCoord;
        TextureInfo *texInfo = m_glyphsTexture.value(glyphIndex);

        resizeTexture(texInfo, texInfo->allocatedArea.width(), texInfo->allocatedArea.height());

        glyphTextures[texInfo].append(glyphIndex);

        int padding = texInfo->padding;
        int expectedWidth = qCeil(c.width + c.xMargin * 2);
        glyph = glyph.copy(
            -padding, -padding, expectedWidth + padding * 2, glyph.height() + padding * 2);

        if (useTextureResizeWorkaround()) {
            uchar *inBits = glyph.scanLine(0);
            uchar *outBits = texInfo->image.scanLine(int(c.y) - padding) + int(c.x) - padding;
            for (int y = 0; y < glyph.height(); ++y) {
                memcpy(outBits, inBits, glyph.width());
                inBits += glyph.width();
                outBits += texInfo->image.width();
            }
        }

        QRhiTextureSubresourceUploadDescription
            subresDesc(glyph.constBits(), glyph.width() * glyph.height());
        subresDesc.setSourceSize(QSize(glyph.width(), glyph.height()));
        subresDesc.setDestinationTopLeft(QPoint(c.x - padding, c.y - padding));
        texInfo->uploads.append(QRhiTextureUploadEntry(0, 0, subresDesc));
    }

    for (int i = 0; i < glyphs.size(); ++i) {
        TextureInfo *texInfo = m_glyphsTexture.value(glyphs.at(i).glyph());
        if (!texInfo->uploads.isEmpty()) {
            QRhiTextureUploadDescription desc;
            desc.setEntries(texInfo->uploads.cbegin(), texInfo->uploads.cend());
            m_batch->uploadTexture(texInfo->texture, desc);
            texInfo->uploads.clear();
        }
    }

    for (auto i = glyphTextures.constBegin(), cend = glyphTextures.constEnd(); i != cend; ++i) {
        TextureInfo t;
        t.texture = i.key()->texture;
        t.size = i.key()->size;
        setGlyphTexture(i.value(), t);
    }
}

QCRhiDistanceFieldGlyphCache::GlyphData &QCRhiDistanceFieldGlyphCache::glyphData(glyph_t glyph)
{
    auto data = m_glyphsData.find(glyph);
    if (data == m_glyphsData.end()) {
        GlyphData &gd = emptyData(glyph);
        gd.path = m_rawFontCache[m_referenceFont].mutatedFont.pathForGlyph(glyph);
        // need bounding rect in base font size scale
        qreal scaleFactor = qreal(1) / QT_DISTANCEFIELD_SCALE(m_rawFontCache[m_referenceFont].doubleGlyphResolution);
        QTransform scaleDown;
        scaleDown.scale(scaleFactor, scaleFactor);
        gd.boundingRect = scaleDown.mapRect(gd.path.boundingRect());
        return gd;
    }
    return data.value();
}

QT_END_NAMESPACE
