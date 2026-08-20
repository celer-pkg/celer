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

#ifndef SCATTERPOINTBUFFERHELPER_P_H
#define SCATTERPOINTBUFFERHELPER_P_H

#include "datavisualizationglobal_p.h"
#include "abstractobjecthelper_p.h"
#include "scatterseriesrendercache_p.h"

QT_BEGIN_NAMESPACE

class ScatterPointBufferHelper : public AbstractObjectHelper
{
public:
    ScatterPointBufferHelper();
    virtual ~ScatterPointBufferHelper();

    GLuint pointBuf();

    void pushPoint(uint pointIndex);
    void popPoint();
    void load(ScatterSeriesRenderCache *cache);
    void update(ScatterSeriesRenderCache *cache);
    void setScaleY(float scale) { m_scaleY = scale; }
    void updateUVs(ScatterSeriesRenderCache *cache);

public:
    GLuint m_pointbuffer;

private:
    void createRangeGradientUVs(ScatterSeriesRenderCache *cache, QList<QVector2D> &buffered_uvs);

private:
    QList<QVector3D> m_bufferedPoints;
    int m_oldRemoveIndex;
    float m_scaleY;
};

QT_END_NAMESPACE

#endif
