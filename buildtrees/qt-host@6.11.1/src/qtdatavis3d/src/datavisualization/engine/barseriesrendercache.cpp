// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "barseriesrendercache_p.h"

QT_BEGIN_NAMESPACE

BarSeriesRenderCache::BarSeriesRenderCache(QAbstract3DSeries *series,
                                           Abstract3DRenderer *renderer)
    : SeriesRenderCache(series, renderer),
      m_visualIndex(-1)
{
}

BarSeriesRenderCache::~BarSeriesRenderCache()
{
}

void BarSeriesRenderCache::cleanup(TextureHelper *texHelper)
{
    m_renderArray.clear();
    m_sliceArray.clear();

    SeriesRenderCache::cleanup(texHelper);
}

QT_END_NAMESPACE
