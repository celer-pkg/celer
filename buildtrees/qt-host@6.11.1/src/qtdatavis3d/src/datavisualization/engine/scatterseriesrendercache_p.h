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

#ifndef SCATTERSERIESRENDERCACHE_P_H
#define SCATTERSERIESRENDERCACHE_P_H

#include "datavisualizationglobal_p.h"
#include "seriesrendercache_p.h"
#include "qscatter3dseries_p.h"
#include "scatterrenderitem_p.h"

QT_BEGIN_NAMESPACE

class ScatterObjectBufferHelper;
class ScatterPointBufferHelper;

class ScatterSeriesRenderCache : public SeriesRenderCache
{
public:
    ScatterSeriesRenderCache(QAbstract3DSeries *series, Abstract3DRenderer *renderer);
    virtual ~ScatterSeriesRenderCache();

    void cleanup(TextureHelper *texHelper) override;

    inline ScatterRenderItemArray &renderArray() { return m_renderArray; }
    inline QScatter3DSeries *series() const { return static_cast<QScatter3DSeries *>(m_series); }
    inline void setItemSize(float size) { m_itemSize = size; }
    inline float itemSize() const { return m_itemSize; }
    inline void setSelectionIndexOffset(int offset) { m_selectionIndexOffset = offset; }
    inline int selectionIndexOffset() const { return m_selectionIndexOffset; }
    inline void setStaticBufferDirty(bool state) { m_staticBufferDirty = state; }
    inline bool staticBufferDirty() const { return m_staticBufferDirty; }
    inline int oldArraySize() const { return m_oldRenderArraySize; }
    inline void setOldArraySize(int size) { m_oldRenderArraySize = size; }
    inline const QString &oldMeshFileName() const { return m_oldMeshFileName; }
    inline void setOldMeshFileName(const QString &meshFileName) { m_oldMeshFileName = meshFileName; }
    inline void setBufferObject(ScatterObjectBufferHelper *object) { m_scatterBufferObj = object; }
    inline ScatterObjectBufferHelper *bufferObject() const { return m_scatterBufferObj; }
    inline void setBufferPoints(ScatterPointBufferHelper *object) { m_scatterBufferPoints = object; }
    inline ScatterPointBufferHelper *bufferPoints() const { return m_scatterBufferPoints; }
    inline QList<int> &updateIndices() { return m_updateIndices; }
    inline QList<int> &bufferIndices() { return m_bufferIndices; }
    inline void setVisibilityChanged(bool changed) { m_visibilityChanged = changed; }
    inline bool visibilityChanged() const { return m_visibilityChanged; }

protected:
    ScatterRenderItemArray m_renderArray;
    float m_itemSize;
    int m_selectionIndexOffset; // Temporarily cached value for selection color calculations
    bool m_staticBufferDirty;
    int m_oldRenderArraySize; // Used to detect if full buffer change needed
    QString m_oldMeshFileName; // Used to detect if full buffer change needed
    ScatterObjectBufferHelper *m_scatterBufferObj;
    ScatterPointBufferHelper *m_scatterBufferPoints;
    QList<int> m_updateIndices; // Used as temporary cache during item updates
    QList<int> m_bufferIndices; // Cache for mapping renderarray to mesh buffer
    bool m_visibilityChanged; // Used to detect if full buffer change needed
};

QT_END_NAMESPACE

#endif
