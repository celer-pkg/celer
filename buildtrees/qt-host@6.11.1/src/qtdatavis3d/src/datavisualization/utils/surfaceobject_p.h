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

#ifndef SURFACEOBJECT_P_H
#define SURFACEOBJECT_P_H

#include "abstractobjecthelper_p.h"
#include "qsurfacedataproxy.h"

#include <QtCore/QRect>
#include <QtGui/QColor>

QT_BEGIN_NAMESPACE

class Surface3DRenderer;
class AxisRenderCache;

class SurfaceObject : public AbstractObjectHelper
{
public:
    enum SurfaceType {
        SurfaceSmooth,
        SurfaceFlat,
        Undefined
    };

    enum DataDimension {
        BothAscending = 0,
        XDescending = 1,
        ZDescending = 2,
        BothDescending = XDescending | ZDescending
    };
    Q_DECLARE_FLAGS(DataDimensions, DataDimension)

public:
    SurfaceObject(Surface3DRenderer *renderer);
    virtual ~SurfaceObject();

    void setUpData(const QSurfaceDataArray &dataArray, const QRect &space,
                   bool changeGeometry, bool polar, bool flipXZ = false);
    void setUpSmoothData(const QSurfaceDataArray &dataArray, const QRect &space,
                         bool changeGeometry, bool polar, bool flipXZ = false);
    void smoothUVs(const QSurfaceDataArray &dataArray, const QSurfaceDataArray &modelArray);
    void coarseUVs(const QSurfaceDataArray &dataArray, const QSurfaceDataArray &modelArray);
    void updateCoarseRow(const QSurfaceDataArray &dataArray, int rowIndex, bool polar);
    void updateSmoothRow(const QSurfaceDataArray &dataArray, int startRow, bool polar);
    void updateSmoothItem(const QSurfaceDataArray &dataArray, int row, int column, bool polar);
    void updateCoarseItem(const QSurfaceDataArray &dataArray, int row, int column, bool polar);
    void createSmoothIndices(int x, int y, int endX, int endY);
    void createCoarseSubSection(int x, int y, int columns, int rows);
    void createSmoothGridlineIndices(int x, int y, int endX, int endY);
    void createCoarseGridlineIndices(int x, int y, int endX, int endY);
    void uploadBuffers();
    GLuint gridElementBuf();
    GLuint uvBuf() override;
    GLuint gridIndexCount();
    QVector3D vertexAt(int column, int row);
    void clear();
    float minYValue() const { return m_minY; }
    float maxYValue() const { return m_maxY; }
    inline void activateSurfaceTexture(bool value) { m_returnTextureBuffer = value; }
    inline void setLineColor(const QColor &color) { m_wireframeColor = color; }
    inline const QColor &wireframeColor() const { return m_wireframeColor; }

private:
    void createCoarseIndices(GLint *indices, int &p, int row, int upperRow, int j);
    void createNormals(int &p, int row, int upperRow, int j);
    void createSmoothNormalBodyLine(int &totalIndex, int column);
    void createSmoothNormalUpperLine(int &totalIndex);
    QVector3D createSmoothNormalBodyLineItem(int x, int y);
    QVector3D createSmoothNormalUpperLineItem(int x, int y);
    QVector3D normal(const QVector3D &a, const QVector3D &b, const QVector3D &c);
    void createBuffers(const QList<QVector3D> &vertices, const QList<QVector2D> &uvs,
                       const QList<QVector3D> &normals, const GLint *indices);
    void checkDirections(const QSurfaceDataArray &array);
    inline void getNormalizedVertex(const QSurfaceDataItem &data, QVector3D &vertex, bool polar,
                                    bool flipXZ);

private:
    SurfaceType m_surfaceType = Undefined;
    int m_columns = 0;
    int m_rows = 0;
    GLuint m_gridElementbuffer;
    GLuint m_gridIndexCount = 0;
    QList<QVector3D> m_vertices;
    QList<QVector3D> m_normals;
    // Caches are not owned
    AxisRenderCache &m_axisCacheX;
    AxisRenderCache &m_axisCacheY;
    AxisRenderCache &m_axisCacheZ;
    Surface3DRenderer *m_renderer;
    float m_minY;
    float m_maxY;
    GLuint m_uvTextureBuffer;
    bool m_returnTextureBuffer = false;
    SurfaceObject::DataDimensions m_dataDimension;
    SurfaceObject::DataDimensions m_oldDataDimension = DataDimensions(-1);
    QColor m_wireframeColor;
};

QT_END_NAMESPACE

#endif
