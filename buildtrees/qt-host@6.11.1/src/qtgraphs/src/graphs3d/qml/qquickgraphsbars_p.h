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

#ifndef QQUICKGRAPHSBARS_H
#define QQUICKGRAPHSBARS_H

#include "qabstract3daxis.h"
#include "qbar3dseries.h"
#include "qquickgraphsitem_p.h"
#include <private/barinstancing_p.h>

#include <QtQuick3D/private/qquick3dmaterial_p.h>

QT_BEGIN_NAMESPACE

class QCategory3DAxis;

struct Bars3DChangeBitField
{
    bool multiSeriesScalingChanged : 1;
    bool barSpecsChanged : 1;
    bool selectedBarChanged : 1;
    bool rowsChanged : 1;
    bool itemChanged : 1;
    bool floorLevelChanged : 1;
    bool barSeriesMarginChanged : 1;
    bool axisRangeChanged : 1;
    bool floorChanged : 1;

    Bars3DChangeBitField()
        : multiSeriesScalingChanged(true)
        , barSpecsChanged(true)
        , selectedBarChanged(true)
        , rowsChanged(false)
        , itemChanged(false)
        , floorLevelChanged(false)
        , barSeriesMarginChanged(false)
        , axisRangeChanged(false)
        , floorChanged(true)
    {}
};

class Q_GRAPHS_EXPORT QQuickGraphsBars : public QQuickGraphsItem
{
    Q_OBJECT
    Q_PROPERTY(QCategory3DAxis *rowAxis READ rowAxis WRITE setRowAxis NOTIFY rowAxisChanged)
    Q_PROPERTY(QValue3DAxis *valueAxis READ valueAxis WRITE setValueAxis NOTIFY valueAxisChanged)
    Q_PROPERTY(
        QCategory3DAxis *columnAxis READ columnAxis WRITE setColumnAxis NOTIFY columnAxisChanged)
    Q_PROPERTY(bool multiSeriesUniform READ isMultiSeriesUniform WRITE setMultiSeriesUniform NOTIFY
                   multiSeriesUniformChanged)
    Q_PROPERTY(float barThickness READ barThickness WRITE setBarThickness NOTIFY barThicknessChanged)
    Q_PROPERTY(QSizeF barSpacing READ barSpacing WRITE setBarSpacing NOTIFY barSpacingChanged)
    Q_PROPERTY(bool barSpacingRelative READ isBarSpacingRelative WRITE setBarSpacingRelative NOTIFY
                   barSpacingRelativeChanged)
    Q_PROPERTY(QSizeF barSeriesMargin READ barSeriesMargin WRITE setBarSeriesMargin NOTIFY
                   barSeriesMarginChanged)
    Q_PROPERTY(QQmlListProperty<QBar3DSeries> seriesList READ seriesList CONSTANT)
    Q_PROPERTY(QBar3DSeries *selectedSeries READ selectedSeries NOTIFY selectedSeriesChanged)
    Q_PROPERTY(QBar3DSeries *primarySeries READ primarySeries WRITE setPrimarySeries NOTIFY
                   primarySeriesChanged)
    Q_PROPERTY(float floorLevel READ floorLevel WRITE setFloorLevel NOTIFY floorLevelChanged)
    Q_CLASSINFO("DefaultProperty", "seriesList")

    QML_NAMED_ELEMENT(Bars3D)

public:
    explicit QQuickGraphsBars(QQuickItem *parent = 0);
    ~QQuickGraphsBars() override;

    struct ChangeItem
    {
        QBar3DSeries *series;
        QPoint point;
    };
    struct ChangeRow
    {
        QBar3DSeries *series;
        qsizetype row;
    };

    QCategory3DAxis *rowAxis() const;
    void setRowAxis(QCategory3DAxis *axis);
    QValue3DAxis *valueAxis() const;
    void setValueAxis(QValue3DAxis *axis);
    QCategory3DAxis *columnAxis() const;
    void setColumnAxis(QCategory3DAxis *axis);

    void setMultiSeriesUniform(bool uniform);
    bool isMultiSeriesUniform() const;

    void setBarThickness(float thicknessRatio);
    float barThickness() const;

    void setBarSpacing(QSizeF spacing);
    QSizeF barSpacing() const;

    void setBarSpacingRelative(bool relative);
    bool isBarSpacingRelative() const;

    void setBarSeriesMargin(QSizeF margin);
    QSizeF barSeriesMargin() const;

    void setMultiSeriesScaling(bool uniform);
    bool multiSeriesScaling() const;

    void setBarSpecs(float thicknessRatio = 1.0f,
                     QSizeF spacing = QSizeF(1.0, 1.0),
                     bool relative = true);

    void setFloorLevel(float level);
    float floorLevel() const;

    QQmlListProperty<QBar3DSeries> seriesList();
    static void appendSeriesFunc(QQmlListProperty<QBar3DSeries> *list, QBar3DSeries *series);
    static qsizetype countSeriesFunc(QQmlListProperty<QBar3DSeries> *list);
    static QBar3DSeries *atSeriesFunc(QQmlListProperty<QBar3DSeries> *list, qsizetype index);
    static void clearSeriesFunc(QQmlListProperty<QBar3DSeries> *list);
    Q_INVOKABLE void addSeries(QBar3DSeries *series);
    Q_INVOKABLE void removeSeries(QBar3DSeries *series);
    Q_INVOKABLE void insertSeries(qsizetype index, QBar3DSeries *series);
    Q_INVOKABLE void clearSelection() override;

    void setPrimarySeries(QBar3DSeries *series);
    QBar3DSeries *primarySeries() const;
    QBar3DSeries *selectedSeries() const;
    static QPoint invalidSelectionPosition() { return QPoint(-1, -1); }
    void setSelectionMode(QtGraphs3D::SelectionFlags mode) override;

    void handleAxisAutoAdjustRangeChangedInOrientation(QAbstract3DAxis::AxisOrientation orientation,
                                                       bool autoAdjust) override;
    void handleSeriesVisibilityChangedBySender(QObject *sender) override;
    void handleItemLabelVisibleChangedBySender(bool visible, QObject *sender) override;

    void handleAxisRangeChangedBySender(QObject *sender) override;
    void adjustAxisRanges() override;
    void handleLightingModeChanged() override;

    void handleMultiAxisChanged(QAbstract3DAxis *axis) override;

    void setSelectedBar(QPoint coord, QBar3DSeries *series, bool enterSlice);

    QList<QBar3DSeries *> barSeriesList();

    bool isSeriesVisualsDirty() const { return m_isSeriesVisualsDirty; }
    void setSeriesVisualsDirty(bool dirty) { m_isSeriesVisualsDirty = dirty; }
    bool isDataDirty() const { return m_isDataDirty; }
    void setDataDirty(bool dirty) { m_isDataDirty = dirty; }

    void renderSliceToImage(int requestedIndex, QtGraphs3D::SliceCaptureType sliceType);
    Q_REVISION(6, 10)
    Q_INVOKABLE void renderSliceToImage(int requestedIndex, QtGraphs3D::SliceCaptureType sliceType,
                                        const QUrl &filePath);

protected:
    void componentComplete() override;
    void synchData() override;
    void updateFloor();
    void updateParameters();
    void updateFloorLevel(float level);
    void updateGraph() override;
    void updateAxisRange(float min, float max) override;
    void updateAxisReversed(bool enable) override;
    void updateLightStrength() override;
    void calculateSceneScalingFactors() override;
    QVector3D calculateCategoryLabelPosition(QAbstract3DAxis *axis,
                                             QVector3D labelPosition,
                                             int index) override;
    float calculateCategoryGridLinePosition(QAbstract3DAxis *axis, int index) override;
    void createSliceView() override;
    void toggleSliceGraph() override;
    void handleLabelCountChanged(QQuick3DRepeater *repeater, QColor axisLabelColor) override;
    void updateSelectionMode(QtGraphs3D::SelectionFlags mode) override;
    bool doPicking(QPointF position) override;
    bool doRayPicking(QVector3D origin, QVector3D direction) override;
    QAbstract3DAxis *createDefaultAxis(QAbstract3DAxis::AxisOrientation orientation) override;
    void updateSliceItemLabel(const QString &label, QVector3D position) override;
    QAbstract3DAxis *getSeriesMultiAxis(QAbstract3DSeries *series,
                       QAbstract3DAxis::AxisOrientation orientation) override;

    QQuick3DViewport* createOffscreenSliceView(int requestedIndex,
                                              QtGraphs3D::SliceCaptureType sliceType);

public Q_SLOTS:
    void handleAxisXChanged(QAbstract3DAxis *axis) override;
    void handleAxisYChanged(QAbstract3DAxis *axis) override;
    void handleAxisZChanged(QAbstract3DAxis *axis) override;
    void handleSeriesMeshChanged(QAbstract3DSeries::Mesh mesh);
    void handleMeshSmoothChanged(bool enable);
    void handleCameraRotationChanged();
    void handleArrayReset();
    void handleRowsAdded(qsizetype startIndex, qsizetype count);
    void handleRowsChanged(qsizetype startIndex, qsizetype count);
    void handleRowsRemoved(qsizetype startIndex, qsizetype count);
    void handleRowsInserted(qsizetype startIndex, qsizetype count);
    void handleItemChanged(qsizetype rowIndex, qsizetype columnIndex);
    void handleDataRowLabelsChanged();
    void handleDataColumnLabelsChanged();
    void handleRowColorsChanged();
    void handleValueColoringChanged();

Q_SIGNALS:
    void rowAxisChanged(QCategory3DAxis *axis);
    void valueAxisChanged(QValue3DAxis *axis);
    void columnAxisChanged(QCategory3DAxis *axis);
    void multiSeriesUniformChanged(bool uniform);
    void barThicknessChanged(float thicknessRatio);
    void barSpacingChanged(QSizeF spacing);
    void barSpacingRelativeChanged(bool relative);
    void barSeriesMarginChanged(QSizeF margin);
    void meshFileNameChanged(const QString &filename);
    void primarySeriesChanged(QBar3DSeries *series);
    void selectedSeriesChanged(QBar3DSeries *series);
    void floorLevelChanged(float level);
    Q_REVISION(6, 10)
    void sliceImageChanged(const QImage &image);

private:
    enum SelectionType {
        SelectionNone = 0,
        SelectionItem,
        SelectionRow,
        SelectionColumn,
    };

    Bars3DChangeBitField m_changeTracker;
    QList<ChangeItem> m_changedItems;
    QList<ChangeRow> m_changedRows;

    QBar3DSeries *m_primarySeries = nullptr;

    bool m_isMultiSeriesUniform = false;
    bool m_isBarSpecRelative = true;
    float m_barThicknessRatio = 1.0f;
    QSizeF m_barSpacing = QSizeF(1.0f, 1.0f);
    float m_floorLevel = 0.0f;
    QSizeF m_barSeriesMargin = QSizeF(0.0f, 0.0f);

    qsizetype m_cachedRowCount = 0;
    qsizetype m_cachedColumnCount = 0;
    qsizetype m_minRow = 0;
    qsizetype m_maxRow = 0;
    qsizetype m_minCol = 0;
    qsizetype m_maxCol = 0;
    qsizetype m_newRows = 0;
    qsizetype m_newCols = 0;

    float m_maxSceneSize = 40.f;
    float m_rowWidth = 0.0f;
    float m_columnDepth = 0.0f;
    float m_maxDimension = 0.0f;
    float m_scaleFactor = 0.0f;
    float m_xScaleFactor = 1.f;
    float m_zScaleFactor = 1.f;

    QSizeF m_cachedBarSeriesMargin = QSizeF(0.0f, 0.0f);
    QSizeF m_cachedBarThickness = QSizeF(1.0f, 1.0f);
    QSizeF m_cachedBarSpacing = QSizeF(1.0f, 1.0f);

    float m_xScale = scale().x();
    float m_yScale = scale().y();
    float m_zScale = scale().z();

    float m_vBackgroundMargin = 0.1f;
    float m_hBackgroundMargin = 0.1f;

    bool m_hasNegativeValues = false;
    bool m_noZeroInRange = false;
    float m_actualFloorLevel = 0.0f;
    float m_heightNormalizer = 1.0f;
    float m_backgroundAdjustment = 0.0f;

    QQuick3DModel *m_floorBackground = nullptr;
    QQuick3DNode *m_floorBackgroundScale = nullptr;
    QQuick3DNode *m_floorBackgroundRotation = nullptr;

    QBar3DSeries *m_selectedBarSeries = nullptr;
    QPoint m_selectedBar = invalidSelectionPosition(); // Points to row & column in data window.
    QVector3D m_selectedBarPos = QVector3D(0.0f, 0.0f, 0.0f);

    struct BarModel
    {
        QQuick3DModel *model = nullptr;
        QBarDataItem *barItem;
        QPoint coord;
        int visualIndex;
        float heightValue;
        QQuick3DTexture *texture;
        BarInstancing *instancing = nullptr;
        BarInstancing *selectionInstancing = nullptr;
        QQuick3DModel *selectedModel = nullptr;
        BarInstancing *multiSelectionInstancing = nullptr;
        QQuick3DModel *multiSelectedModel = nullptr;
    };

    QHash<QBar3DSeries *, QList<BarModel *> *> m_barModelsMap;
    QAbstract3DSeries::Mesh m_meshType = QAbstract3DSeries::Mesh::Sphere;
    bool m_smooth = false;
    bool m_keepSeriesUniform = false;
    bool m_hasHighlightTexture = false;
    float m_seriesScaleX = 0.0f;
    float m_seriesScaleZ = 0.0f;
    float m_seriesStep = 0.0f;
    float m_seriesStart = 0.0f;
    float m_zeroPosition = 0.0f;
    int m_visibleSeriesCount = 0;
    QQuick3DTexture *m_highlightTexture = nullptr;
    QQuick3DTexture *m_multiHighlightTexture = nullptr;
    QHash<QBar3DSeries *, QList<BarModel *>> m_slicedBarModels;
    bool m_selectionDirty = false;
    QImage *m_grabresult = nullptr;

    void calculateHeightAdjustment();
    void calculateSeriesStartPosition();
    void connectSeries(QBar3DSeries *series);
    void disconnectSeries(QBar3DSeries *series);
    void generateBars(QList<QBar3DSeries *> &barSeriesList);
    QQuick3DModel *createDataItem(QQuick3DNode *scene, QAbstract3DSeries *series);
    QString getMeshFileName();
    void fixMeshFileName(QString &fileName, QAbstract3DSeries::Mesh meshType);
    void updateBarVisuality(QBar3DSeries *series, int visualIndex);
    void updateBarPositions(QBar3DSeries *series);
    float updateBarHeightParameters(const QBarDataItem *item, QValue3DAxis *axis = nullptr);
    void updateBarVisuals(QBar3DSeries *series);
    void updateItemMaterial(QQuick3DModel *item,
                            bool useGradient,
                            bool rangeGradient,
                            const QString &materialName);
    void updateMaterialProperties(QQuick3DModel *item,
                                  const bool isHighlight,
                                  const bool isMultiHighlight,
                                  QQuick3DTexture *texture,
                                  QColor color,
                                  const bool transparency = false);
    void removeBarModels();
    void deleteBarModels(QQuick3DModel *model);
    void deleteBarItemHolders(BarInstancing *instancing);
    QQuick3DTexture *createTexture();
    void updateSelectedBar();
    SelectionType isSelected(int row, int bar, QBar3DSeries *series);
    void resetClickedStatus();
    void removeSlicedBarModels();
    void createBarItemHolders(QBar3DSeries *series, QList<BarModel *> barList, bool slice);

    void updateBarSpecs(float thicknessRatio, QSizeF spacing, bool relative);
    void updateBarSeriesMargin(QSizeF margin);

    void adjustSelectionPosition(QPoint &pos, const QBar3DSeries *series);

    friend class Q3DBarsWidgetItem;
    friend class QQuickGraphsBarsNode;
};

QT_END_NAMESPACE
#endif // QQUICKGRAPHSBARS_H
