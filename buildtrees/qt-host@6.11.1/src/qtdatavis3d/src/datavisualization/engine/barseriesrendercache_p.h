// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

//
//  W A R N I N G
//  -------------
//
// This file is not part of the QtDataVisualization API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.

#ifndef BARSERIESRENDERCACHE_P_H
#define BARSERIESRENDERCACHE_P_H

#include "datavisualizationglobal_p.h"
#include "seriesrendercache_p.h"
#include "qbar3dseries_p.h"
#include "barrenderitem_p.h"

QT_BEGIN_NAMESPACE

class BarSeriesRenderCache : public SeriesRenderCache
{
public:
    BarSeriesRenderCache(QAbstract3DSeries *series, Abstract3DRenderer *renderer);
    virtual ~BarSeriesRenderCache();

    void cleanup(TextureHelper *texHelper) override;

    inline BarRenderItemArray &renderArray() { return m_renderArray; }
    inline QBar3DSeries *series() const { return static_cast<QBar3DSeries *>(m_series); }
    inline QList<BarRenderSliceItem> &sliceArray() { return m_sliceArray; }
    inline void setVisualIndex(int index) { m_visualIndex = index; }
    inline int visualIndex() {return m_visualIndex; }

protected:
    BarRenderItemArray m_renderArray;
    QList<BarRenderSliceItem> m_sliceArray;
    int m_visualIndex; // order of the series is relevant
};

QT_END_NAMESPACE

#endif
