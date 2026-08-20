// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


//
//  W A R N I N G
//  -------------
//
// This file is not part of the QtGraphs API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.

#ifndef QQUICKGRAPHSSURFACE_P_H
#define QQUICKGRAPHSSURFACE_P_H

#include "qquickgraphsitem_p.h"
#include "qsurface3dseries.h"

#include <private/qgraphsglobal_p.h>

QT_BEGIN_NAMESPACE

class QValue3DAxis;
class QSurface3DSeries;
class QQuickGraphsSurface;

struct Surface3DChangeBitField
{
    bool selectedPointChanged : 1;
    bool rowsChanged : 1;
    bool itemChanged : 1;
    bool flipHorizontalGridChanged : 1;
    bool surfaceTextureChanged : 1;

    Surface3DChangeBitField()
        : selectedPointChanged(true)
        , rowsChanged(false)
        , itemChanged(false)
        , flipHorizontalGridChanged(true)
        , surfaceTextureChanged(true)
    {}
};

class Q_GRAPHS_EXPORT QQuickGraphsSurface : public QQuickGraphsItem
{
    Q_OBJECT
    Q_PROPERTY(QValue3DAxis *axisX READ axisX WRITE setAxisX NOTIFY axisXChanged)
    Q_PROPERTY(QValue3DAxis *axisY READ axisY WRITE setAxisY NOTIFY axisYChanged)
    Q_PROPERTY(QValue3DAxis *axisZ READ axisZ WRITE setAxisZ NOTIFY axisZChanged)
    Q_PROPERTY(QSurface3DSeries *selectedSeries READ selectedSeries NOTIFY selectedSeriesChanged)
    Q_PROPERTY(QQmlListProperty<QSurface3DSeries> seriesList READ seriesList CONSTANT)
    Q_PROPERTY(bool flipHorizontalGrid READ flipHorizontalGrid WRITE setFlipHorizontalGrid NOTIFY
                   flipHorizontalGridChanged)
    Q_CLASSINFO("DefaultProperty", "seriesList")

    QML_NAMED_ELEMENT(Surface3D)

public:
    explicit QQuickGraphsSurface(QQuickItem *parent = 0);
    ~QQuickGraphsSurface() override;

    struct ChangeItem
    {
        QSurface3DSeries *series;
        QPoint point;
    };
    struct ChangeRow
    {
        QSurface3DSeries *series;
        qsizetype row;
    };
    enum DataDimension {
        BothAscending = 0,
        XDescending = 1,
        ZDescending = 2,
        BothDescending = XDescending | ZDescending,
    };
    Q_DECLARE_FLAGS(DataDimensions, DataDimension)

    void setAxisX(QValue3DAxis *axis);
    QValue3DAxis *axisX() const;
    void setAxisY(QValue3DAxis *axis);
    QValue3DAxis *axisY() const;
    void setAxisZ(QValue3DAxis *axis);
    QValue3DAxis *axisZ() const;

    QQmlListProperty<QSurface3DSeries> seriesList();
    static void appendSeriesFunc(QQmlListProperty<QSurface3DSeries> *list, QSurface3DSeries *series);
    static qsizetype countSeriesFunc(QQmlListProperty<QSurface3DSeries> *list);
    static QSurface3DSeries *atSeriesFunc(QQmlListProperty<QSurface3DSeries> *list, qsizetype index);
    static void clearSeriesFunc(QQmlListProperty<QSurface3DSeries> *list);
    Q_INVOKABLE void addSeries(QSurface3DSeries *series);
    Q_INVOKABLE void removeSeries(QSurface3DSeries *series);
    Q_INVOKABLE void clearSelection() override;

    void setFlipHorizontalGrid(bool flip);
    bool flipHorizontalGrid() const;
    bool isFlatShadingSupported();
    QList<QSurface3DSeries *> surfaceSeriesList();
    void updateSurfaceTexture(QSurface3DSeries *series);

    static QPoint invalidSelectionPosition();
    void setSelectedPoint(QPoint position, QSurface3DSeries *series, bool enterSlice);

    QSurface3DSeries *selectedSeries() const { return m_selectedSeries; }
    void setSelectionMode(QtGraphs3D::SelectionFlags mode) override;

    void setDataDimensions(DataDimensions dimension) { m_dataDimensions = dimension; }
    DataDimensions dataDimensions() { return m_dataDimensions; }

    bool hasChangedSeriesList() const { return !m_changedSeriesList.isEmpty(); }
    bool isSeriesVisibilityDirty() const { return m_isSeriesVisibilityDirty; }
    void setSeriesVisibilityDirty(bool dirty) { m_isSeriesVisibilityDirty = dirty; }
    bool isDataDirty() const { return m_isDataDirty; }
    void setDataDirty(bool dirty) { m_isDataDirty = dirty; }
    bool isSeriesVisualsDirty() const { return m_isSeriesVisualsDirty; }
    void setSeriesVisualsDirty(bool dirty) { m_isSeriesVisualsDirty = dirty; }
    bool isIndexDirty() const { return m_isIndexDirty; }
    void setIndexDirty(bool dirty) { m_isIndexDirty = dirty; }

    QList<QAbstract3DSeries *> changedSeriesList() { return m_changedSeriesList; }

    bool isSelectedPointChanged() const { return m_changeTracker.selectedPointChanged; }
    void setSelectedPointChanged(bool changed) { m_changeTracker.selectedPointChanged = changed; }

    bool isFlipHorizontalGridChanged() const { return m_changeTracker.flipHorizontalGridChanged; }
    void setFlipHorizontalGridChanged(bool changed)
    {
        m_changeTracker.flipHorizontalGridChanged = changed;
    }

    bool isSurfaceTextureChanged() const { return m_changeTracker.surfaceTextureChanged; }
    void setSurfaceTextureChanged(bool changed) { m_changeTracker.surfaceTextureChanged = changed; }
    bool isChangedTexturesEmpty() const { return m_changedTextures.empty(); }
    bool hasSeriesToChangeTexture(QSurface3DSeries *series) const
    {
        return m_changedTextures.contains(series);
    }

    void handleAxisAutoAdjustRangeChangedInOrientation(QAbstract3DAxis::AxisOrientation orientation,
                                                       bool autoAdjust) override;
    void handleAxisRangeChangedBySender(QObject *sender) override;
    void handleSeriesVisibilityChangedBySender(QObject *sender) override;
    void handleItemLabelVisibleChangedBySender(bool visible, QObject *sender) override;
    void adjustAxisRanges() override;

    void handleMultiAxisChanged(QAbstract3DAxis *axis) override;

    void handleLightingModeChanged() override;

    void renderSliceToImage(int index,
                            int requestedIndex,
                            QtGraphs3D::SliceCaptureType sliceType);
    Q_REVISION(6, 10)
    Q_INVOKABLE void renderSliceToImage(int index, int requestedIndex,
                                        QtGraphs3D::SliceCaptureType sliceType,
                                        const QUrl &filePath);

protected:
    void componentComplete() override;
    void synchData() override;
    void updateGraph() override;
    void calculateSceneScalingFactors() override;
    void toggleSliceGraph() override;
    void updateSingleHighlightColor() override;
    void updateLightStrength() override;
    void handleThemeTypeChange() override;
    bool doPicking(QPointF position) override;
    bool doRayPicking(QVector3D origin, QVector3D direction) override;
    QAbstract3DAxis *getSeriesMultiAxis(QAbstract3DSeries *series,
                       QAbstract3DAxis::AxisOrientation orientation) override;

    void createSliceView() override;
    void updateSliceItemLabel(const QString &label, QVector3D position) override;
    void updateSelectionMode(QtGraphs3D::SelectionFlags mode) override;

    QQuick3DViewport *createOffscreenSliceView(int index, int requestedIndex,
                                               QtGraphs3D::SliceCaptureType sliceType);

public Q_SLOTS:
    void handleAxisXChanged(QAbstract3DAxis *axis) override;
    void handleAxisYChanged(QAbstract3DAxis *axis) override;
    void handleAxisZChanged(QAbstract3DAxis *axis) override;

    void handleShadingChanged();
    void handleWireframeColorChanged();
    void handleFlipHorizontalGridChanged(bool flip);

    void handleArrayReset();
    void handleRowsAdded(qsizetype startIndex, qsizetype count);
    void handleRowsChanged(qsizetype startIndex, qsizetype count);
    void handleRowsRemoved(qsizetype startIndex, qsizetype count);
    void handleRowsInserted(qsizetype startIndex, qsizetype count);
    void handleItemChanged(qsizetype rowIndex, qsizetype columnIndex);

    void handleFlatShadingSupportedChange(bool supported);

Q_SIGNALS:
    void axisXChanged(QValue3DAxis *axis);
    void axisYChanged(QValue3DAxis *axis);
    void axisZChanged(QValue3DAxis *axis);
    void selectedSeriesChanged(QSurface3DSeries *series);
    void flipHorizontalGridChanged(bool flip);
    Q_REVISION(6, 10)
    void sliceImageChanged(const QImage &image);

private:
    struct SurfaceVertex
    {
        QVector3D position;
        QVector2D uv;
        QPoint coord;
    };

    struct SurfaceModel
    {
        QQuick3DModel *model;
        QQuick3DModel *gridModel;
        QQuick3DModel *sliceModel;
        QQuick3DModel *sliceGridModel;
        QQuick3DModel *fillModel;
        QList<SurfaceVertex> vertices;
        QList<QVector4D> heights;
        QList<quint32> indices;
        QList<quint32> gridIndices;
        QSurface3DSeries *series;
        QQuick3DTexture *gradientTexture;
        QQuick3DTexture *texture;
        QQuick3DTexture *heightTexture;
        QQuick3DCustomMaterial *customMaterial;
        qsizetype columnCount;
        qsizetype rowCount;
        SurfaceVertex selectedVertex;
        bool picked = false;
        QVector3D boundsMin;
        QVector3D boundsMax;
        QRect sampleSpace;
        bool ascendingX;
        bool ascendingZ;
    };

    QVector3D getNormalizedVertex(const QSurfaceDataItem &data, bool polar, bool flipXZ, QSurface3DSeries *series = nullptr);
    QRect calculateSampleSpace(SurfaceModel *model, const QSurfaceDataArray &array);
    QPointF mapCoordsToWorldSpace(SurfaceModel *model, QPointF coords);
    QPoint mapCoordsToSampleSpace(SurfaceModel *model, QPointF coords);
    void createIndices(SurfaceModel *model, qsizetype columnCount, qsizetype rowCount);
    void createLineIndices(SurfaceModel *model, qsizetype pointCount);
    void createGridlineIndices(SurfaceModel *model, qsizetype x, qsizetype y, qsizetype endX, qsizetype endY);
    void handleChangedSeries();
    void updateModel(SurfaceModel *model);
    void updateFill(SurfaceModel *model);
    void updateLineFill(SurfaceModel *model);
    void updateMaterial(SurfaceModel *model);
    void updateSelectedPoint();
    void addModel(QSurface3DSeries *series);
    void addFillModel(SurfaceModel *model);
    void addSliceModel(SurfaceModel *model);
    QSurfaceDataArray removeNaNRows(const QSurfaceDataArray &array);

    void handleMeshTypeChanged(QAbstract3DSeries::Mesh mesh);
    void handlePointerChanged(const QString &filename);
    void changePointerMeshTypeForSeries(QAbstract3DSeries::Mesh mesh, QSurface3DSeries *series);
    void changePointerForSeries(const QString &filename, QSurface3DSeries *series);
    void changeSlicePointerMeshTypeForSeries(QAbstract3DSeries::Mesh mesh, QSurface3DSeries *series);
    void changeSlicePointerForSeries(const QString &filename, QSurface3DSeries *series);
    QString getMeshFileName(QAbstract3DSeries::Mesh mesh, QSurface3DSeries *series) const;

    QVector3D pickSurfaces(QVector3D rayOrigin, QVector3D rayDir, SurfaceModel *&pickedModel);
    QVector3D triangleIntersection(QVector3D origin,
                                   QVector3D dir,
                                   const std::array<QVector3D, 3> &triangle);
    bool intersectWithAABB(QVector3D boundMin, QVector3D boundsMax, QVector3D origin, QVector3D dir);

    QList<SurfaceModel *> m_model;
    QMap<QSurface3DSeries *, QQuick3DModel *> m_selectionPointers = {};
    QMap<QSurface3DSeries *, QQuick3DModel *> m_sliceSelectionPointers = {};

    bool m_isIndexDirty = true;
    bool m_selectionDirty = false;
    QHash<SurfaceModel *, bool> m_fillDirty;

    Surface3DChangeBitField m_changeTracker;
    QPoint m_selectedPoint;
    QSurface3DSeries *m_selectedSeries
        = nullptr; // Points to the series for which the point is selected in
                   // single series selection cases.
    bool m_flatShadingSupported = true;
    QList<ChangeItem> m_changedItems;
    QList<ChangeRow> m_changedRows;
    bool m_flipHorizontalGrid = false;
    QList<QSurface3DSeries *> m_changedTextures;
    bool m_isSeriesVisibilityDirty = false;

    QQuick3DModel *m_topGrid = nullptr;
    QQuick3DNode *m_topGridScale = nullptr;
    QQuick3DNode *m_topGridRotation = nullptr;

    DataDimensions m_dataDimensions;

    QImage *m_grabresult = nullptr;

    friend class Q3DSurfaceWidgetItem;
    friend class QQuickGraphsSurfaceNode;
};

QT_END_NAMESPACE

#endif // QQUICKGRAPHSSURFACE_P_H
