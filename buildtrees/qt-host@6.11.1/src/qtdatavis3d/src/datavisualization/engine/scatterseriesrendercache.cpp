// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "scatterseriesrendercache_p.h"
#include "scatterobjectbufferhelper_p.h"
#include "scatterpointbufferhelper_p.h"

QT_BEGIN_NAMESPACE

ScatterSeriesRenderCache::ScatterSeriesRenderCache(QAbstract3DSeries *series,
                                                   Abstract3DRenderer *renderer)
    : SeriesRenderCache(series, renderer),
      m_itemSize(0.0f),
      m_selectionIndexOffset(0),
      m_staticBufferDirty(false),
      m_oldRenderArraySize(0),
      m_oldMeshFileName(QString()),
      m_scatterBufferObj(0),
      m_scatterBufferPoints(0),
      m_visibilityChanged(false)
{
}

ScatterSeriesRenderCache::~ScatterSeriesRenderCache()
{
    delete m_scatterBufferObj;
    delete m_scatterBufferPoints;
}

void ScatterSeriesRenderCache::cleanup(TextureHelper *texHelper)
{
    m_renderArray.clear();

    SeriesRenderCache::cleanup(texHelper);
}

QT_END_NAMESPACE
