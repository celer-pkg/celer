// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "customrenderitem_p.h"

QT_BEGIN_NAMESPACE

CustomRenderItem::CustomRenderItem()
    : AbstractRenderItem(),
      m_texture(0),
      m_positionAbsolute(false),
      m_scalingAbsolute(true),
      m_object(0),
      m_needBlend(true),
      m_visible(true),
      m_valid(true),
      m_index(0),
      m_shadowCasting(false),
      m_isFacingCamera(false),
      m_item(0),
      m_renderer(0),
      m_labelItem(false),
      m_textureWidth(0),
      m_textureHeight(0),
      m_textureDepth(0),
      m_isVolume(false),
      m_textureFormat(QImage::Format_ARGB32),
      m_sliceIndexX(-1),
      m_sliceIndexY(-1),
      m_sliceIndexZ(-1),
      m_alphaMultiplier(1.0f),
      m_preserveOpacity(true),
      m_useHighDefShader(true),
      m_drawSlices(false),
      m_drawSliceFrames(false)

{
}

CustomRenderItem::~CustomRenderItem()
{
    ObjectHelper::releaseObjectHelper(m_renderer, m_object);
}

bool CustomRenderItem::setMesh(const QString &meshFile)
{
    ObjectHelper::resetObjectHelper(m_renderer, m_object, meshFile);
    return m_object ? true : false;
}

void CustomRenderItem::setColorTable(const QList<QRgb> &colors)
{
    m_colorTable.resize(256);
    for (int i = 0; i < 256; i++) {
        if (i < colors.size()) {
            const QRgb &rgb = colors.at(i);
            m_colorTable[i] = QVector4D(float(qRed(rgb)) / 255.0f,
                                        float(qGreen(rgb)) / 255.0f,
                                        float(qBlue(rgb)) / 255.0f,
                                        float(qAlpha(rgb)) / 255.0f);
        } else {
            m_colorTable[i] = QVector4D(0.0f, 0.0f, 0.0f, 0.0f);
        }
    }
}

void CustomRenderItem::setMinBounds(const QVector3D &bounds)
{
    m_minBounds = bounds;
    m_minBoundsNormal = m_minBounds;
    m_minBoundsNormal.setY(-m_minBoundsNormal.y());
    m_minBoundsNormal.setZ(-m_minBoundsNormal.z());
    m_minBoundsNormal = 0.5f * (m_minBoundsNormal + oneVector);
}

void CustomRenderItem::setMaxBounds(const QVector3D &bounds)
{
    m_maxBounds = bounds;
    m_maxBoundsNormal = m_maxBounds;
    m_maxBoundsNormal.setY(-m_maxBoundsNormal.y());
    m_maxBoundsNormal.setZ(-m_maxBoundsNormal.z());
    m_maxBoundsNormal = 0.5f * (m_maxBoundsNormal + oneVector);
}

void CustomRenderItem::setSliceFrameColor(const QColor &color)
{
    const QRgb &rgb = color.rgba();
    m_sliceFrameColor = QVector4D(float(qRed(rgb)) / 255.0f,
                                  float(qGreen(rgb)) / 255.0f,
                                  float(qBlue(rgb)) / 255.0f,
                                  float(1.0f)); // Alpha not supported for frames
}

QT_END_NAMESPACE
