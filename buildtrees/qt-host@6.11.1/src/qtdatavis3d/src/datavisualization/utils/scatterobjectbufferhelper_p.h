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

#ifndef SCATTEROBJECTBUFFERHELPER_P_H
#define SCATTEROBJECTBUFFERHELPER_P_H

#include "datavisualizationglobal_p.h"
#include "abstractobjecthelper_p.h"
#include "scatterseriesrendercache_p.h"

QT_BEGIN_NAMESPACE

class ScatterObjectBufferHelper : public AbstractObjectHelper
{
public:
    ScatterObjectBufferHelper();
    virtual ~ScatterObjectBufferHelper();

    void fullLoad(ScatterSeriesRenderCache *cache, qreal dotScale);
    void update(ScatterSeriesRenderCache *cache, qreal dotScale);
    void updateUVs(ScatterSeriesRenderCache *cache);
    void setScaleY(float scale) { m_scaleY = scale; }

private:
    uint createRangeGradientUVs(ScatterSeriesRenderCache *cache, QList<QVector2D> &buffered_uvs);
    uint createObjectGradientUVs(ScatterSeriesRenderCache *cache, QList<QVector2D> &buffered_uvs,
                                 const QList<QVector3D> &indexed_vertices);

    float m_scaleY;
    static const GLfloat itemScaler;
};

QT_END_NAMESPACE

#endif
