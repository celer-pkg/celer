// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QQUICKGRAPHSITEM_H
#define QQUICKGRAPHSITEM_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the QtGraphs API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.

#include <QtGraphs/qgraphs3dnamespace.h>
#include "qabstract3daxis.h"
#include "qabstract3dseries.h"
#include "qcategory3daxis.h"
#include "qqmlcomponent.h"
#include "qvalue3daxis.h"

#include <QtQuick3D/private/qquick3dviewport_p.h>

QT_BEGIN_NAMESPACE

class Q3DScene;

class QAbstract3DAxis;
class QAbstract3DSeries;
class QCustom3DItem;
class QCustom3DVolume;
class QCustom3DLabel;
class QGraphsInputHandler;
class QGraphsTheme;
class QQuaternion;
class QQuick3DCustomMaterial;
class QQuick3DDirectionalLight;
class QQuick3DPrincipledMaterial;
class QQuick3DRepeater;
class QQuick3DPerspectiveCamera;
class QQuick3DOrthographicCamera;

struct Abstract3DChangeBitField
{
    bool themeChanged : 1;
    bool shadowQualityChanged : 1;
    bool selectionModeChanged : 1;
    bool optimizationHintChanged : 1;
    bool axisXTypeChanged : 1;
    bool axisYTypeChanged : 1;
    bool axisZTypeChanged : 1;
    bool axisXTitleChanged : 1;
    bool axisYTitleChanged : 1;
    bool axisZTitleChanged : 1;
    bool axisXLabelsChanged : 1;
    bool axisYLabelsChanged : 1;
    bool axisZLabelsChanged : 1;
    bool axisXRangeChanged : 1;
    bool axisYRangeChanged : 1;
    bool axisZRangeChanged : 1;
    bool axisXSegmentCountChanged : 1;
    bool axisYSegmentCountChanged : 1;
    bool axisZSegmentCountChanged : 1;
    bool axisXSubSegmentCountChanged : 1;
    bool axisYSubSegmentCountChanged : 1;
    bool axisZSubSegmentCountChanged : 1;
    bool axisXLabelFormatChanged : 1;
    bool axisYLabelFormatChanged : 1;
    bool axisZLabelFormatChanged : 1;
    bool axisXReversedChanged : 1;
    bool axisYReversedChanged : 1;
    bool axisZReversedChanged : 1;
    bool axisXFormatterChanged : 1;
    bool axisYFormatterChanged : 1;
    bool axisZFormatterChanged : 1;
    bool projectionChanged : 1;
    bool axisXLabelAutoRotationChanged : 1;
    bool axisYLabelAutoRotationChanged : 1;
    bool axisZLabelAutoRotationChanged : 1;
    bool axisXScaleLabelsByCountChanged : 1;
    bool axisYScaleLabelsByCountChanged : 1;
    bool axisZScaleLabelsByCountChanged : 1;
    bool axisXLabelSizeChanged : 1;
    bool axisYLabelSizeChanged : 1;
    bool axisZLabelSizeChanged : 1;
    bool aspectRatioChanged : 1;
    bool horizontalAspectRatioChanged : 1;
    bool axisXTitleVisibilityChanged : 1;
    bool axisYTitleVisibilityChanged : 1;
    bool axisZTitleVisibilityChanged : 1;
    bool axisXLabelVisibilityChanged : 1;
    bool axisYLabelVisibilityChanged : 1;
    bool axisZLabelVisibilityChanged : 1;
    bool axisXTitleFixedChanged : 1;
    bool axisYTitleFixedChanged : 1;
    bool axisZTitleFixedChanged : 1;
    bool axisXTitleOffsetChanged : 1;
    bool axisYTitleOffsetChanged : 1;
    bool axisZTitleOffsetChanged : 1;
    bool multiAxisChanged : 1;
    bool polarChanged : 1;
    bool labelMarginChanged : 1;
    bool radialLabelOffsetChanged : 1;
    bool marginChanged : 1;
    bool cameraChanged : 1;
    bool themeBackgroundColorChanged : 1;
    bool themeBackgroundVisibilityChanged : 1;
    bool themeAxisXChanged : 1;
    bool themeAxisYChanged : 1;
    bool themeAxisZChanged : 1;
    bool themeGridChanged : 1;
    bool themeGridVisibilityChanged : 1;
    bool themeLabelBackgroundColorChanged : 1;
    bool themeLabelBackgroundVisibilityChanged : 1;
    bool themeLabelBorderVisibilityChanged : 1;
    bool themeLabelFontChanged : 1;
    bool themeLabelTextColorChanged : 1;
    bool themeLabelsVisibilityChanged : 1;
    bool themePlotAreaBackgroundColorChanged : 1;
    bool themePlotAreaBackgroundVisibilityChanged : 1;

    Abstract3DChangeBitField()
        : themeChanged(true)
        , shadowQualityChanged(true)
        , selectionModeChanged(true)
        , optimizationHintChanged(true)
        , axisXTypeChanged(true)
        , axisYTypeChanged(true)
        , axisZTypeChanged(true)
        , axisXTitleChanged(true)
        , axisYTitleChanged(true)
        , axisZTitleChanged(true)
        , axisXLabelsChanged(true)
        , axisYLabelsChanged(true)
        , axisZLabelsChanged(true)
        , axisXRangeChanged(true)
        , axisYRangeChanged(true)
        , axisZRangeChanged(true)
        , axisXSegmentCountChanged(true)
        , axisYSegmentCountChanged(true)
        , axisZSegmentCountChanged(true)
        , axisXSubSegmentCountChanged(true)
        , axisYSubSegmentCountChanged(true)
        , axisZSubSegmentCountChanged(true)
        , axisXLabelFormatChanged(true)
        , axisYLabelFormatChanged(true)
        , axisZLabelFormatChanged(true)
        , axisXReversedChanged(true)
        , axisYReversedChanged(true)
        , axisZReversedChanged(true)
        , axisXFormatterChanged(true)
        , axisYFormatterChanged(true)
        , axisZFormatterChanged(true)
        , projectionChanged(true)
        , axisXLabelAutoRotationChanged(true)
        , axisYLabelAutoRotationChanged(true)
        , axisZLabelAutoRotationChanged(true)
        , aspectRatioChanged(true)
        , horizontalAspectRatioChanged(true)
        , axisXTitleVisibilityChanged(true)
        , axisYTitleVisibilityChanged(true)
        , axisZTitleVisibilityChanged(true)
        , axisXLabelVisibilityChanged(true)
        , axisYLabelVisibilityChanged(true)
        , axisZLabelVisibilityChanged(true)
        , axisXTitleFixedChanged(true)
        , axisYTitleFixedChanged(true)
        , axisZTitleFixedChanged(true)
        , axisXTitleOffsetChanged(true)
        , axisYTitleOffsetChanged(true)
        , axisZTitleOffsetChanged(true)
        , polarChanged(true)
        , labelMarginChanged(true)
        , radialLabelOffsetChanged(true)
        , marginChanged(true)
        , cameraChanged(true)
        , themeBackgroundColorChanged(true)
        , themeBackgroundVisibilityChanged(true)
        , themeAxisXChanged(true)
        , themeAxisYChanged(true)
        , themeAxisZChanged(true)
        , themeGridChanged(true)
        , themeGridVisibilityChanged(true)
        , themeLabelBackgroundColorChanged(true)
        , themeLabelBackgroundVisibilityChanged(true)
        , themeLabelBorderVisibilityChanged(true)
        , themeLabelFontChanged(true)
        , themeLabelTextColorChanged(true)
        , themeLabelsVisibilityChanged(true)
        , themePlotAreaBackgroundColorChanged(true)
        , themePlotAreaBackgroundVisibilityChanged(true)
    {}
};

class Q_GRAPHS_EXPORT QQuickGraphsItem : public QQuick3DViewport
{
    Q_OBJECT
    Q_PROPERTY(QQuick3DNode *rootNode READ rootNode CONSTANT REVISION(6, 9))
    Q_PROPERTY(QtGraphs3D::SelectionFlags selectionMode READ selectionMode WRITE setSelectionMode
                   NOTIFY selectionModeChanged)
    Q_PROPERTY(QtGraphs3D::ShadowQuality shadowQuality READ shadowQuality WRITE setShadowQuality
                   NOTIFY shadowQualityChanged)
    Q_PROPERTY(int msaaSamples READ msaaSamples WRITE setMsaaSamples NOTIFY msaaSamplesChanged)
    Q_PROPERTY(Q3DScene *scene READ scene NOTIFY sceneChanged)
    Q_PROPERTY(QGraphsTheme *theme READ theme WRITE setTheme NOTIFY themeChanged)
    Q_PROPERTY(QtGraphs3D::RenderingMode renderingMode READ renderingMode WRITE setRenderingMode
                   NOTIFY renderingModeChanged)
    Q_PROPERTY(QtGraphs3D::TransparencyTechnique transparencyTechnique READ transparencyTechnique
                   WRITE setTransparencyTechnique NOTIFY transparencyTechniqueChanged REVISION(6, 9))
    Q_PROPERTY(bool measureFps READ measureFps WRITE setMeasureFps NOTIFY measureFpsChanged)
    Q_PROPERTY(int currentFps READ currentFps NOTIFY currentFpsChanged)
    Q_PROPERTY(QQmlListProperty<QCustom3DItem> customItemList READ customItemList CONSTANT)
    Q_PROPERTY(bool orthoProjection READ isOrthoProjection WRITE setOrthoProjection NOTIFY
                   orthoProjectionChanged)
    Q_PROPERTY(
        QtGraphs3D::ElementType selectedElement READ selectedElement NOTIFY selectedElementChanged)
    Q_PROPERTY(qreal aspectRatio READ aspectRatio WRITE setAspectRatio NOTIFY aspectRatioChanged)
    Q_PROPERTY(QtGraphs3D::OptimizationHint optimizationHint READ optimizationHint WRITE
                   setOptimizationHint NOTIFY optimizationHintChanged)
    Q_PROPERTY(bool polar READ isPolar WRITE setPolar NOTIFY polarChanged)
    Q_PROPERTY(float labelMargin READ labelMargin WRITE setLabelMargin NOTIFY labelMarginChanged)
    Q_PROPERTY(float radialLabelOffset READ radialLabelOffset WRITE setRadialLabelOffset NOTIFY
                   radialLabelOffsetChanged)
    Q_PROPERTY(qreal horizontalAspectRatio READ horizontalAspectRatio WRITE setHorizontalAspectRatio
                   NOTIFY horizontalAspectRatioChanged)
    Q_PROPERTY(QLocale locale READ locale WRITE setLocale NOTIFY localeChanged)
    Q_PROPERTY(
        QVector3D queriedGraphPosition READ queriedGraphPosition NOTIFY queriedGraphPositionChanged)
    Q_PROPERTY(qreal margin READ margin WRITE setMargin NOTIFY marginChanged)
    Q_PROPERTY(qreal cutoffMargin READ cutoffMargin WRITE setCutoffMargin NOTIFY
                   cutoffMarginChanged REVISION (6, 11))
    Q_PROPERTY(float cameraXRotation READ cameraXRotation WRITE setCameraXRotation NOTIFY
                   cameraXRotationChanged)
    Q_PROPERTY(float cameraYRotation READ cameraYRotation WRITE setCameraYRotation NOTIFY
                   cameraYRotationChanged)
    Q_PROPERTY(float minCameraXRotation READ minCameraXRotation WRITE setMinCameraXRotation NOTIFY
                   minCameraXRotationChanged REVISION(6, 9))
    Q_PROPERTY(float maxCameraXRotation READ maxCameraXRotation WRITE setMaxCameraXRotation NOTIFY
                   maxCameraXRotationChanged REVISION(6, 9))
    Q_PROPERTY(float minCameraYRotation READ minCameraYRotation WRITE setMinCameraYRotation NOTIFY
                   minCameraYRotationChanged REVISION(6, 9))
    Q_PROPERTY(float maxCameraYRotation READ maxCameraYRotation WRITE setMaxCameraYRotation NOTIFY
                   maxCameraYRotationChanged REVISION(6, 9))
    Q_PROPERTY(float cameraZoomLevel READ cameraZoomLevel WRITE setCameraZoomLevel NOTIFY
                   cameraZoomLevelChanged)
    Q_PROPERTY(QtGraphs3D::CameraPreset cameraPreset READ cameraPreset WRITE setCameraPreset NOTIFY
                   cameraPresetChanged)
    Q_PROPERTY(QVector3D cameraTargetPosition READ cameraTargetPosition WRITE
                   setCameraTargetPosition NOTIFY cameraTargetPositionChanged)
    Q_PROPERTY(float minCameraZoomLevel READ minCameraZoomLevel WRITE setMinCameraZoomLevel NOTIFY
                   minCameraZoomLevelChanged)
    Q_PROPERTY(float maxCameraZoomLevel READ maxCameraZoomLevel WRITE setMaxCameraZoomLevel NOTIFY
                   maxCameraZoomLevelChanged)
    Q_PROPERTY(bool wrapCameraXRotation READ wrapCameraXRotation WRITE setWrapCameraXRotation NOTIFY
                   wrapCameraXRotationChanged)
    Q_PROPERTY(bool wrapCameraYRotation READ wrapCameraYRotation WRITE setWrapCameraYRotation NOTIFY
                   wrapCameraYRotationChanged)
    Q_PROPERTY(bool rotationEnabled READ rotationEnabled WRITE setRotationEnabled NOTIFY
                   rotationEnabledChanged)
    Q_PROPERTY(bool zoomAtTargetEnabled READ zoomAtTargetEnabled WRITE setZoomAtTargetEnabled NOTIFY
                   zoomAtTargetEnabledChanged)
    Q_PROPERTY(bool selectionEnabled READ selectionEnabled WRITE setSelectionEnabled NOTIFY
                   selectionEnabledChanged)
    Q_PROPERTY(bool zoomEnabled READ zoomEnabled WRITE setZoomEnabled NOTIFY zoomEnabledChanged)

    Q_PROPERTY(QColor lightColor READ lightColor WRITE setLightColor NOTIFY lightColorChanged)
    Q_PROPERTY(float ambientLightStrength READ ambientLightStrength WRITE setAmbientLightStrength
                   NOTIFY ambientLightStrengthChanged)
    Q_PROPERTY(
        float lightStrength READ lightStrength WRITE setLightStrength NOTIFY lightStrengthChanged)
    Q_PROPERTY(float shadowStrength READ shadowStrength WRITE setShadowStrength NOTIFY
                   shadowStrengthChanged)
    Q_PROPERTY(QtGraphs3D::GridLineType gridLineType READ gridLineType WRITE setGridLineType NOTIFY
                   gridLineTypeChanged FINAL)

    QML_NAMED_ELEMENT(GraphsItem3D)
    QML_UNCREATABLE("")

public:
    explicit QQuickGraphsItem(QQuickItem *parent = 0);
    ~QQuickGraphsItem() override;

    void markDataDirty();
    void markSeriesVisualsDirty();
    void markSeriesItemLabelsDirty();
    void emitNeedRender();

    void setQueriedGraphPosition(QVector3D position) { m_queriedGraphPosition = position; }

    virtual void handleAxisTitleChangedBySender(QObject *sender);
    virtual void handleAxisLabelsChangedBySender(QObject *sender);
    virtual void handleAxisRangeChangedBySender(QObject *sender);
    virtual void handleAxisSegmentCountChangedBySender(QObject *sender);
    virtual void handleAxisSubSegmentCountChangedBySender(QObject *sender);
    virtual void handleAxisAutoAdjustRangeChangedInOrientation(
        QAbstract3DAxis::AxisOrientation orientation, bool autoAdjust)
        = 0;
    virtual void handleAxisLabelFormatChangedBySender(QObject *sender);
    virtual void handleAxisReversedChangedBySender(QObject *sender);
    virtual void handleAxisFormatterDirtyBySender(QObject *sender);
    virtual void handleAxisLabelAutoRotationChangedBySender(QObject *sender);
    virtual void handleAxisScaleLabelsByCountChangedBySender(QObject *sender);
    virtual void handleAxisLabelSizeChangedBySender(QObject *sender);
    virtual void handleAxisTitleVisibilityChangedBySender(QObject *sender);
    virtual void handleAxisLabelVisibilityChangedBySender(QObject *sender);
    virtual void handleAxisTitleFixedChangedBySender(QObject *sender);
    virtual void handleAxisTitleOffsetChangedBySender(QObject *sender);
    virtual void handleSeriesVisibilityChangedBySender(QObject *sender);
    virtual void handleLightingModeChanged() = 0;
    virtual void adjustAxisRanges() = 0;

    virtual void handleMultiAxisChanged(QAbstract3DAxis *axis);
    void handleMultiAxisDirty();

    bool graphPositionQueryPending() const { return m_graphPositionQueryPending; }
    void setGraphPositionQueryPending(const bool &pending)
    {
        m_graphPositionQueryPending = pending;
    }

    virtual void addSeriesInternal(QAbstract3DSeries *series);
    void insertSeries(qsizetype index, QAbstract3DSeries *series);
    virtual void removeSeriesInternal(QAbstract3DSeries *series);
    QList<QAbstract3DSeries *> seriesList();

    void setAxisX(QAbstract3DAxis *axis);
    QAbstract3DAxis *axisX() const;
    void setAxisY(QAbstract3DAxis *axis);
    QAbstract3DAxis *axisY() const;
    void setAxisZ(QAbstract3DAxis *axis);
    QAbstract3DAxis *axisZ() const;
    virtual void addAxis(QAbstract3DAxis *axis);
    virtual void releaseAxis(QAbstract3DAxis *axis);
    virtual QList<QAbstract3DAxis *> axes() const; // Omits default axes

    virtual void setRenderingMode(QtGraphs3D::RenderingMode mode);
    virtual QtGraphs3D::RenderingMode renderingMode() const;

    virtual void setTransparencyTechnique(QtGraphs3D::TransparencyTechnique technique);
    virtual QtGraphs3D::TransparencyTechnique transparencyTechnique() const;

    virtual void setSelectionMode(QtGraphs3D::SelectionFlags mode);
    virtual QtGraphs3D::SelectionFlags selectionMode() const;

    void doSetShadowQuality(QtGraphs3D::ShadowQuality quality);
    virtual void setShadowQuality(QtGraphs3D::ShadowQuality quality);
    virtual QtGraphs3D::ShadowQuality shadowQuality() const;

    virtual QtGraphs3D::ElementType selectedElement() const;

    virtual void setMsaaSamples(int samples);
    virtual int msaaSamples() const;

    void addTheme(QGraphsTheme *theme);
    void releaseTheme(QGraphsTheme *theme);
    void setTheme(QGraphsTheme *theme);
    QGraphsTheme *theme() const;
    QList<QGraphsTheme *> themes() const;

    bool isSlicingActive() const;
    void setSlicingActive(bool isSlicing);

    bool isCustomDataDirty() const { return m_isCustomDataDirty; }
    void setCustomDataDirty(bool dirty) { m_isCustomDataDirty = dirty; }
    bool isCustomItemDirty() const { return m_isCustomItemDirty; }
    void setCustomItemDirty(bool dirty) { m_isCustomItemDirty = dirty; }
    bool isCustomLabelItem(QCustom3DItem *item) const;
    bool isCustomVolumeItem(QCustom3DItem *item) const;
    QImage customTextureImage(QCustom3DItem *item);
    Q3DScene *scene();

    Q_INVOKABLE virtual bool hasSeries(QAbstract3DSeries *series);
    Q_INVOKABLE virtual void clearSelection() = 0;

    void deleteCustomItems();
    void deleteCustomItem(QCustom3DItem *item);
    void deleteCustomItem(QVector3D position);
    QList<QCustom3DItem *> customItems() const;

    Q_INVOKABLE virtual qsizetype addCustomItem(QCustom3DItem *item);
    Q_INVOKABLE virtual void removeCustomItems();
    Q_INVOKABLE virtual void removeCustomItem(QCustom3DItem *item);
    Q_INVOKABLE virtual void removeCustomItemAt(QVector3D position);
    Q_INVOKABLE virtual void releaseCustomItem(QCustom3DItem *item);

    Q_INVOKABLE virtual int selectedLabelIndex() const;
    Q_INVOKABLE virtual QAbstract3DAxis *selectedAxis() const;

    Q_INVOKABLE virtual qsizetype selectedCustomItemIndex() const;
    Q_INVOKABLE virtual QCustom3DItem *selectedCustomItem() const;

    QQmlListProperty<QCustom3DItem> customItemList();
    static void appendCustomItemFunc(QQmlListProperty<QCustom3DItem> *list, QCustom3DItem *item);
    static qsizetype countCustomItemFunc(QQmlListProperty<QCustom3DItem> *list);
    static QCustom3DItem *atCustomItemFunc(QQmlListProperty<QCustom3DItem> *list, qsizetype index);
    static void clearCustomItemFunc(QQmlListProperty<QCustom3DItem> *list);

    void geometryChange(const QRectF &newGeometry, const QRectF &oldGeometry) override;

    void resizeViewports(QSizeF viewportSize);

    void checkWindowList(QQuickWindow *window);

    void setMeasureFps(bool enable);
    bool measureFps() const;
    int currentFps() const;

    void setOrthoProjection(bool enable);
    bool isOrthoProjection() const;

    void setAspectRatio(qreal ratio);
    qreal aspectRatio() const;

    void setOptimizationHint(QtGraphs3D::OptimizationHint hint);
    QtGraphs3D::OptimizationHint optimizationHint() const;

    void setPolar(bool enable);
    bool isPolar() const;

    void setLabelMargin(float offset);
    float labelMargin() const;

    void setRadialLabelOffset(float offset);
    float radialLabelOffset() const;

    void setHorizontalAspectRatio(qreal ratio);
    qreal horizontalAspectRatio() const;

    void setLocale(const QLocale &locale);
    QLocale locale() const;

    QVector3D queriedGraphPosition() const;

    void setMargin(qreal margin);
    qreal margin() const;

    void setCutoffMargin(qreal margin);
    qreal cutoffMargin() const;

    bool isReady() { return isComponentComplete(); }
    QQuick3DNode *rootNode() const;

    QQuick3DNode *cameraTarget() { return m_cameraTarget; }
    void setCameraTarget(QQuick3DNode *target) { m_cameraTarget = target; }

    QQuick3DModel *background() const { return m_background; }
    void setBackground(QQuick3DModel *newBackground) { m_background = newBackground; }
    QQuick3DModel *backgroundBB() const { return m_backgroundBB; }

    QQuick3DDirectionalLight *light() const;
    QQuick3DCustomMaterial *createQmlCustomMaterial(const QString &fileName);
    QQuick3DPrincipledMaterial *createPrincipledMaterial();

    QQuickItem *itemLabel() { return m_itemLabel; }
    QQuick3DNode *sliceItemLabel() { return m_sliceItemLabel; }

    QQuick3DModel *m_targetVisualizer;

    QQuick3DRepeater *repeaterX() const { return m_repeaterX; }
    QQuick3DRepeater *repeaterY() const { return m_repeaterY; }
    QQuick3DRepeater *repeaterZ() const { return m_repeaterZ; }

    QQuick3DNode *titleLabelX() const { return m_titleLabelX; }
    QQuick3DNode *titleLabelY() const { return m_titleLabelY; }
    QQuick3DNode *titleLabelZ() const { return m_titleLabelZ; }

    bool isXFlipped() const { return m_xFlipped; }
    void setXFlipped(bool xFlipped) { m_xFlipped = xFlipped; }
    bool isYFlipped() const { return m_yFlipped; }
    void setYFlipped(bool yFlipped) { m_yFlipped = yFlipped; }
    bool isZFlipped() const { return m_zFlipped; }
    void setZFlipped(bool zFlipped) { m_zFlipped = zFlipped; }
    QVector3D scaleWithBackground() const { return m_scaleWithBackground; }
    QVector3D backgroundScaleMargin() const { return m_backgroundScaleMargin; }
    void setScaleWithBackground(QVector3D scale) { m_scaleWithBackground = scale; }
    void setBackgroundScaleMargin(QVector3D margin) { m_backgroundScaleMargin = margin; }
    QVector3D rotation() const { return m_rot; }
    void setRotation(QVector3D rotation) { m_rot = rotation; }
    QVector3D scale() const { return m_scale; }
    void setScale(QVector3D scale) { m_scale = scale; }
    QVector3D translate() const { return m_translate; }
    void setTranslate(QVector3D translate) { m_translate = translate; }

    float lineLengthScaleFactor() const { return m_lineLengthScaleFactor; }
    void setLineLengthScaleFactor(float scaleFactor) { m_lineLengthScaleFactor = scaleFactor; }
    float lineWidthScaleFactor() const { return m_lineWidthScaleFactor; }
    void setLineWidthScaleFactor(float scaleFactor) { m_lineWidthScaleFactor = scaleFactor; }
    float gridOffset() const { return m_gridOffset; }

    QtGraphs3D::CameraPreset cameraPreset() const;
    void setCameraPreset(QtGraphs3D::CameraPreset preset);

    float cameraXRotation() const { return m_xRotation; }
    void setCameraXRotation(float rotation);
    float cameraYRotation() const { return m_yRotation; }
    void setCameraYRotation(float rotation);

    float minCameraXRotation() const { return m_minXRotation; }
    void setMinCameraXRotation(float rotation);
    float maxCameraXRotation() const { return m_maxXRotation; }
    void setMaxCameraXRotation(float rotation);

    float minCameraYRotation() const { return m_minYRotation; }
    void setMinCameraYRotation(float rotation);
    float maxCameraYRotation() const { return m_maxYRotation; }
    void setMaxCameraYRotation(float rotation);

    void setZoomAtTargetEnabled(bool enable);
    bool zoomAtTargetEnabled();
    void setZoomEnabled(bool enable);
    bool zoomEnabled();
    void setSelectionEnabled(bool enable);
    bool selectionEnabled();
    void setRotationEnabled(bool enable);
    bool rotationEnabled();

    Q_INVOKABLE void setDefaultInputHandler();
    Q_INVOKABLE void unsetDefaultInputHandler();
    Q_INVOKABLE void unsetDefaultTapHandler();
    Q_INVOKABLE void unsetDefaultDragHandler();
    Q_INVOKABLE void unsetDefaultWheelHandler();
    Q_INVOKABLE void unsetDefaultPinchHandler();
    Q_INVOKABLE void setDragButton(Qt::MouseButtons button);

    float cameraZoomLevel() const { return m_zoomLevel; }
    void setCameraZoomLevel(float level);

    float minCameraZoomLevel() const { return m_minZoomLevel; }
    void setMinCameraZoomLevel(float level);

    float maxCameraZoomLevel() const { return m_maxZoomLevel; }
    void setMaxCameraZoomLevel(float level);

    void setCameraTargetPosition(QVector3D target);
    QVector3D cameraTargetPosition() const { return m_requestedTarget; }

    bool wrapCameraXRotation() const { return m_wrapXRotation; }
    void setWrapCameraXRotation(bool wrap);

    bool wrapCameraYRotation() const { return m_wrapYRotation; }
    void setWrapCameraYRotation(bool wrap);

    QVector3D graphPositionAt(QPoint point);
    void setCameraPosition(float horizontal, float vertical, float zoom = 100.0f);

    void changeLabelBackgroundColor(QQuick3DRepeater *repeater, QColor color);
    void changeLabelBackgroundVisible(QQuick3DRepeater *repeater, const bool &visible);
    void changeLabelBorderVisible(QQuick3DRepeater *repeater, const bool &visible);
    void changeLabelTextColor(QQuick3DRepeater *repeater, QColor color);
    void changeLabelFont(QQuick3DRepeater *repeater, const QFont &font);
    void changeLabelsVisible(QQuick3DRepeater *repeater, const bool &visible);
    void changeGridLineColor(QQuick3DRepeater *repeater, QColor color);
    void updateTitleLabels();
    virtual void updateSelectionMode(QtGraphs3D::SelectionFlags newMode);

    void setSliceActivatedChanged(bool changed) { m_sliceActivatedChanged = changed; }

    Q_INVOKABLE virtual bool doPicking(QPointF point);

    Q_REVISION(6, 9) Q_INVOKABLE virtual bool doRayPicking(QVector3D origin,
                                          QVector3D direction);

    void minimizeMainGraph();

    int horizontalFlipFactor() const;
    void setHorizontalFlipFactor(int newHorizontalFlipFactor);

    float ambientLightStrength() const;
    void setAmbientLightStrength(float newAmbientLightStrength);
    float lightStrength() const;
    void setLightStrength(float newLightStrength);
    float shadowStrength() const;
    void setShadowStrength(float newShadowStrength);
    QColor lightColor() const;
    void setLightColor(QColor newLightColor);
    QtGraphs3D::GridLineType gridLineType() const;
    void setGridLineType(const QtGraphs3D::GridLineType &gridLineType);

public Q_SLOTS:
    virtual void handleAxisXChanged(QAbstract3DAxis *axis) = 0;
    virtual void handleAxisYChanged(QAbstract3DAxis *axis) = 0;
    virtual void handleAxisZChanged(QAbstract3DAxis *axis) = 0;
    void handleFpsChanged();
    void windowDestroyed(QObject *obj);

    void handleAxisTitleChanged(const QString &title);
    void handleAxisLabelsChanged();
    void handleAxisRangeChanged(float min, float max);
    void handleAxisSegmentCountChanged(qsizetype count);
    void handleAxisSubSegmentCountChanged(qsizetype count);
    void handleAxisAutoAdjustRangeChanged(bool autoAdjust);
    void handleAxisScaleLabelsByCountChanged(bool adjust);
    void handleAxisLabelSizeChanged(qreal size);
    void handleAxisLabelFormatChanged(const QString &format);
    void handleAxisReversedChanged(bool enable);
    void handleAxisFormatterDirty();
    void handleAxisLabelAutoRotationChanged(float angle);
    void handleAxisTitleVisibilityChanged(bool visible);
    void handleAxisLabelVisibilityChanged(bool visible);
    void handleAxisTitleFixedChanged(bool fixed);
    void handleAxisTitleOffsetChanged(float offset);
    void handleInputPositionChanged(QPoint position);
    void handleSeriesVisibilityChanged(bool visible);
    void handleItemLabelVisibleChanged(bool visible);
    virtual void handleItemLabelVisibleChangedBySender(bool visible, QObject *sender) = 0;

    void handleThemeColorStyleChanged(QGraphsTheme::ColorStyle style);
    void handleThemeBaseColorsChanged(const QList<QColor> &color);
    void handleThemeBaseGradientsChanged(const QList<QLinearGradient> &gradient);
    void handleThemeSingleHighlightColorChanged(QColor color);
    void handleThemeSingleHighlightGradientChanged(const QLinearGradient &gradient);
    void handleThemeMultiHighlightColorChanged(QColor color);
    void handleThemeMultiHighlightGradientChanged(const QLinearGradient &gradient);
    void handleThemeTypeChanged(QGraphsTheme::Theme theme);

    void handleRequestShadowQuality(QtGraphs3D::ShadowQuality quality);

    void updateCustomItem();

Q_SIGNALS:
    void selectionModeChanged(QtGraphs3D::SelectionFlags mode);
    void shadowQualityChanged(QtGraphs3D::ShadowQuality quality);
    void shadowsSupportedChanged(bool supported);
    void msaaSamplesChanged(int samples);
    void themeChanged(QGraphsTheme *theme);
    void renderingModeChanged(QtGraphs3D::RenderingMode mode);
    Q_REVISION(6, 9) void transparencyTechniqueChanged(QtGraphs3D::TransparencyTechnique technique);
    void measureFpsChanged(bool enabled);
    void currentFpsChanged(int fps);
    void selectedElementChanged(QtGraphs3D::ElementType type);
    void orthoProjectionChanged(bool enabled);
    void aspectRatioChanged(qreal ratio);
    void optimizationHintChanged(QtGraphs3D::OptimizationHint hint);
    void polarChanged(bool enabled);
    void labelMarginChanged(float margin);
    void radialLabelOffsetChanged(float offset);
    void horizontalAspectRatioChanged(qreal ratio);
    void localeChanged(const QLocale &locale);
    void queriedGraphPositionChanged(QVector3D data);
    void marginChanged(qreal margin);
    Q_REVISION(6, 11) void cutoffMarginChanged(qreal newMargin);
    void cameraPresetChanged(QtGraphs3D::CameraPreset preset);
    void cameraXRotationChanged(float rotation);
    void cameraYRotationChanged(float rotation);
    void cameraZoomLevelChanged(float zoomLevel);
    void cameraTargetPositionChanged(QVector3D target);
    void minCameraZoomLevelChanged(float zoomLevel);
    void maxCameraZoomLevelChanged(float zoomLevel);
    Q_REVISION(6, 9) void minCameraXRotationChanged(float rotation);
    Q_REVISION(6, 9) void minCameraYRotationChanged(float rotation);
    Q_REVISION(6, 9) void maxCameraXRotationChanged(float rotation);
    Q_REVISION(6, 9) void maxCameraYRotationChanged(float rotation);
    void wrapCameraXRotationChanged(bool wrap);
    void wrapCameraYRotationChanged(bool wrap);
    void needRender();
    void themeTypeChanged();
    void axisXChanged(QAbstract3DAxis *axis);
    void axisYChanged(QAbstract3DAxis *axis);
    void axisZChanged(QAbstract3DAxis *axis);
    void activeThemeChanged(QGraphsTheme *activeTheme);

    void tapped(QEventPoint eventPoint, Qt::MouseButton button);
    void doubleTapped(QEventPoint eventPoint, Qt::MouseButton button);
    void longPressed();
    void dragged(QVector2D delta);
    void wheel(QQuickWheelEvent *event);
    void pinch(qreal delta);
    void mouseMove(QPoint mousePos);

    void zoomEnabledChanged(bool enable);
    void zoomAtTargetEnabledChanged(bool enable);
    void rotationEnabledChanged(bool enable);
    void selectionEnabledChanged(bool enable);

    void ambientLightStrengthChanged();
    void lightStrengthChanged();
    void shadowStrengthChanged();
    void lightColorChanged();
    void gridLineTypeChanged();

protected:
    bool event(QEvent *event) override;

    void setParentNode(QQuick3DNode *node);
    virtual void handleWindowChanged(/*QQuickWindow *win*/);
    void itemChange(ItemChange change, const ItemChangeData &value) override;
    virtual void updateWindowParameters();
    virtual void handleSelectionModeChange(QtGraphs3D::SelectionFlags mode);
    virtual void handleShadowQualityChange(QtGraphs3D::ShadowQuality quality);
    virtual void handleSelectedElementChange(QtGraphs3D::ElementType type);
    virtual void handleOptimizationHintChange(QtGraphs3D::OptimizationHint hint);
    void keyPressEvent(QKeyEvent *ev) override;
    virtual void handleThemeTypeChange();
    virtual void handleParentWidthChange();
    virtual void handleParentHeightChange();
    void componentComplete() override;
    void checkSliceEnabled();
    bool isUserCameraRotationRange() { return m_userRotationRange; };
    void setUserCameraRotationRange(bool userRotationRange)
    {
        m_userRotationRange = userRotationRange;
    };

    virtual void createSliceView();
    QQuick3DViewport *createOffscreenSliceView(QtGraphs3D::SliceCaptureType sliceType);

    void handleGraphQueryPositionChanged(QPoint position);
    void handleSelectionQueryPositionChanged(QPoint position);

    void handlePrimarySubViewportChanged(const QRect rect);
    void handleSecondarySubViewportChanged(const QRect rect);

    QQuick3DNode *graphNode() { return m_graphNode; }
    QQuick3DViewport *sliceView() { return m_sliceView; }

    QQmlComponent *createRepeaterDelegateComponent(const QString &fileName);
    QQuick3DRepeater *createRepeater(QQuick3DNode *parent = nullptr);
    QQuick3DNode *createTitleLabel(QQuick3DNode *parent = nullptr);
    void createItemLabel();
    QAbstract3DSeries::SeriesType m_graphType = QAbstract3DSeries::SeriesType::None;

    void updateXTitle(QVector3D labelRotation,
                      QVector3D labelTrans,
                      const QQuaternion &totalRotation,
                      float labelsMaxWidth,
                      QVector3D scale,
                      QAbstract3DAxis *axis = nullptr,
                      QQuick3DNode *label = nullptr);
    void updateYTitle(QVector3D sideLabelRotation,
                      QVector3D backLabelRotation,
                      QVector3D sideLabelTrans,
                      QVector3D backLabelTrans,
                      const QQuaternion &totalSideRotation,
                      const QQuaternion &totalBackRotation,
                      float labelsMaxWidth,
                      QVector3D scale,
                      QAbstract3DAxis *axis = nullptr,
                      QQuick3DNode *label = nullptr);
    void updateZTitle(QVector3D labelRotation,
                      QVector3D labelTrans,
                      const QQuaternion &totalRotation,
                      float labelsMaxWidth,
                      QVector3D scale,
                      QAbstract3DAxis *axis = nullptr,
                      QQuick3DNode *label = nullptr);

    virtual void calculateSceneScalingFactors() = 0;
    void positionAndScaleLine(QQuick3DNode *lineNode, QVector3D scale, QVector3D position);
    int findLabelsMaxWidth(const QStringList &labels);
    virtual QVector3D calculateCategoryLabelPosition(QAbstract3DAxis *axis,
                                                     QVector3D labelPosition,
                                                     int index);
    virtual float calculateCategoryGridLinePosition(QAbstract3DAxis *axis, int index);
    float calculatePolarBackgroundMargin();
    void setFloorGridInRange(bool inRange) { m_isFloorGridInRange = inRange; }
    void setVerticalSegmentLine(bool hasVerticalLine)
    {
        m_hasVerticalSegmentLine = hasVerticalLine;
    }
    void updateGrid();
    void updateGridLineType();
    void updateLabels();
    QVector3D calculateLabelRotation(float labelAutoAngle,
                                     float fractionCamX,
                                     float fractionCamY,
                                     Qt::Axis axis);
    void updateSliceGrid(
        QQuick3DModel *sliceGrid = nullptr,
        QtGraphs3D::SliceCaptureType selectedFlag = QtGraphs3D::SliceCaptureType::NoImage);
    void updateSliceLabels(
        QQuick3DRepeater *horizontalLabel = nullptr,
        QQuick3DRepeater *verticalLabel = nullptr,
        QQuick3DNode *horizontalTitle = nullptr,
        QQuick3DNode *verticalTitle = nullptr,
        QQuick3DNode *itemLabel = nullptr,
        QtGraphs3D::SliceCaptureType selectedFlag = QtGraphs3D::SliceCaptureType::NoImage);
    void updateBackgroundColor();
    void setItemSelected(bool selected);
    virtual void updateShadowQuality(QtGraphs3D::ShadowQuality quality);
    void updateItemLabel(QVector3D position);
    virtual void updateSliceItemLabel(const QString &label, QVector3D position);

    struct Volume
    {
        QQuick3DModel *model = nullptr;
        QQuick3DTexture *texture = nullptr;
        QQuick3DTextureData *textureData = nullptr;
        QQuick3DTexture *colorTexture = nullptr;
        QQuick3DTextureData *colorTextureData = nullptr;
        bool updateTextureData = false;
        bool updateColorTextureData = false;
        bool useHighDefShader = false;
        bool drawSlices = false;
        bool drawSliceFrames = false;
        QQuick3DModel *sliceFrameX = nullptr;
        QQuick3DModel *sliceFrameY = nullptr;
        QQuick3DModel *sliceFrameZ = nullptr;
        QQuick3DTexture *sliceFrameTexture = nullptr;
    };

    virtual void synchData();
    virtual void updateGraph() {}

    bool isSliceEnabled() const { return m_sliceEnabled; }
    void setSliceEnabled(bool enabled) { m_sliceEnabled = enabled; }
    bool isSliceActivatedChanged() const { return m_sliceActivatedChanged; }
    virtual void toggleSliceGraph();
    void createSliceCamera(QQuick3DViewport *sliceView);
    bool isSliceOrthoProjection() const { return m_sliceUseOrthoProjection; }
    void setSliceOrthoProjection(bool enable) { m_sliceUseOrthoProjection = enable; }

    virtual void updateAxisRange(float min, float max);
    virtual void updateAxisReversed(bool enable);
    virtual void updateSingleHighlightColor() {}
    virtual void updateLightStrength() {}

    virtual void handleLabelCountChanged(QQuick3DRepeater *repeater, QColor axisLabelColor);

    struct MultiAxis
    {
        QQuick3DRepeater *repeater = nullptr;
        QQuick3DNode *titleLabel = nullptr;
        QQmlComponent *delegateModel = nullptr;
        QQuick3DModel *grid = nullptr;
        qsizetype seriesIndex = -1;
    };

    void updateMultiAxis();
    void updateMultiAxisLabels(qsizetype axisIndex, QAbstract3DAxis *axis);
    MultiAxis getMultiAxis(qsizetype axisIndex, QAbstract3DAxis *axis);
    MultiAxis createMultiAxis(qsizetype axisIndex);
    void updateMultiAxisGrid(qsizetype axisIndex, QAbstract3DAxis *axis);
    void releaseMultiAxis(QAbstract3DAxis::AxisOrientation orientation, qsizetype index);
    void connectMultiAxis(QAbstract3DAxis *axis);
    virtual QAbstract3DAxis *getSeriesMultiAxis(QAbstract3DSeries *series,
                       QAbstract3DAxis::AxisOrientation orientation) = 0;

    bool isGridUpdated() { return m_gridUpdated; }
    void setGridUpdated(bool updated) { m_gridUpdated = updated; }

    QGraphsInputHandler *graphsInputHandler() const { return m_inputHandler; }

    virtual QAbstract3DAxis *createDefaultAxis(QAbstract3DAxis::AxisOrientation orientation);
    QValue3DAxis *createDefaultValueAxis();
    QCategory3DAxis *createDefaultCategoryAxis();
    void setAxisHelper(QAbstract3DAxis::AxisOrientation orientation,
                       QAbstract3DAxis *axis,
                       QAbstract3DAxis **axisPtr);
    virtual void startRecordingRemovesAndInserts();

    QMap<QCustom3DVolume *, Volume> m_customVolumes;

    Q3DScene *m_scene = nullptr;
    // Active axes
    QAbstract3DAxis *m_axisX = nullptr;
    QAbstract3DAxis *m_axisY = nullptr;
    QAbstract3DAxis *m_axisZ = nullptr;

    QList<QAbstract3DAxis *> m_axes; // List of all added axes

    bool m_isDataDirty = true;
    bool m_isCustomDataDirty = true;
    bool m_isCustomItemDirty = true;
    bool m_isSeriesVisualsDirty = true;
    bool m_renderPending = false;
    bool m_isPolar = false;
    float m_radialLabelOffset = 1.0f;
    float m_polarRadius = 2.0f;
    bool m_userRotationRange = false;

    QList<QAbstract3DSeries *> m_seriesList;

    QList<QAbstract3DSeries *> m_changedSeriesList;

    QList<QCustom3DItem *> m_customItems;

    QtGraphs3D::ElementType m_clickedType = QtGraphs3D::ElementType::None;
    int m_selectedLabelIndex = -1;
    qsizetype m_selectedCustomItemIndex = -1;
    qreal m_margin = -1.0;
    qreal m_cutoffMargin = 0.0;

    QQuickGraphsItem *m_qml = nullptr;

    QQuick3DViewport *m_customView = nullptr;
    QQuick3DNode *m_parentNode = nullptr;

private:
    // This is the same as the minimum bound of GridLine model.
    const float angularLineOffset = -49.98f;
    const float rotationOffset = 90.0f;

    QQuick3DModel *m_gridGeometryModel = nullptr;
    QQuick3DModel *m_subgridGeometryModel = nullptr;
    QQuick3DModel *m_sliceGridGeometryModel = nullptr;
    Abstract3DChangeBitField m_changeTracker;
    QtGraphs3D::SelectionFlags m_selectionMode = QtGraphs3D::SelectionFlag::Item;
    QtGraphs3D::ShadowQuality m_shadowQuality = QtGraphs3D::ShadowQuality::Medium;
    bool m_useOrthoProjection = false;
    qreal m_aspectRatio = 2.0;
    qreal m_horizontalAspectRatio = 0.0;
    QtGraphs3D::OptimizationHint m_optimizationHint = QtGraphs3D::OptimizationHint::Default;
    QLocale m_locale;
    QVector3D m_queriedGraphPosition;
    bool m_graphPositionQueryPending = false;

    QQuick3DNode *m_graphNode = nullptr;
    QQuick3DModel *m_background = nullptr;
    QQuick3DModel *m_backgroundBB = nullptr;
    QQuick3DNode *m_backgroundScale = nullptr;
    QQuick3DNode *m_backgroundRotation = nullptr;

    QQuick3DRepeater *m_repeaterX = nullptr;
    QQuick3DRepeater *m_repeaterY = nullptr;
    QQuick3DRepeater *m_repeaterZ = nullptr;
    std::unique_ptr<QQmlComponent> m_delegateModelX;
    std::unique_ptr<QQmlComponent> m_delegateModelY;
    std::unique_ptr<QQmlComponent> m_delegateModelZ;

    QQuick3DNode *m_titleLabelX = nullptr;
    QQuick3DNode *m_titleLabelY = nullptr;
    QQuick3DNode *m_titleLabelZ = nullptr;

    QList<MultiAxis> m_multiAxesX;
    QList<MultiAxis> m_multiAxesY;
    QList<MultiAxis> m_multiAxesZ;

    QQuickItem *m_itemLabel = nullptr;
    QQuick3DNode *m_sliceItemLabel = nullptr;

    QQuick3DViewport *m_sliceView = nullptr;

    QQuick3DRepeater *m_sliceHorizontalLabelRepeater = nullptr;
    QQuick3DRepeater *m_sliceVerticalLabelRepeater = nullptr;
    std::unique_ptr<QQmlComponent> m_labelDelegate;

    QQuick3DNode *m_sliceHorizontalTitleLabel = nullptr;
    QQuick3DNode *m_sliceVerticalTitleLabel = nullptr;

    QQuick3DNode *m_cameraTarget = nullptr;
    QQuick3DDirectionalLight *m_light = nullptr;
    QQuick3DPerspectiveCamera *m_pCamera = nullptr;
    QQuick3DOrthographicCamera *m_oCamera = nullptr;
    QRectF m_cachedGeometry;
    QtGraphs3D::RenderingMode m_renderMode = QtGraphs3D::RenderingMode::DirectToBackground;
    QtGraphs3D::TransparencyTechnique m_transparencyTechnique
        = QtGraphs3D::TransparencyTechnique::Default;
    int m_samples = 0;
    int m_windowSamples = 0;
    QSize m_initialisedSize = QSize(0, 0);
    bool m_runningInDesigner;

    bool m_xFlipped = false;
    bool m_yFlipped = false;
    bool m_zFlipped = false;

    bool m_flipScales;
    int m_horizontalFlipFactor = 1;

    bool m_isFloorGridInRange = false;
    bool m_hasVerticalSegmentLine = true;

    QVector3D m_scaleWithBackground = QVector3D(1.0f, 1.0f, 1.0f);
    QVector3D m_backgroundScaleMargin = QVector3D(0.0f, 0.0f, 0.0f);

    QVector3D m_rot = QVector3D(1.0f, 1.0f, 1.0f);

    QVector3D m_scale = QVector3D(1.0f, 1.0f, 1.0f);

    QVector3D m_translate = QVector3D(1.0f, 1.0f, 1.0f);

    QVector3D m_labelScale = QVector3D(0.01f, 0.01f, 0.0f);

    float m_gridOffset = 0.002f;
    float m_lineWidthScaleFactor = 0.0001f;
    float m_lineLengthScaleFactor = 0.02f;

    float m_labelMargin = .1f;

    bool m_itemSelected = false;
    bool m_sliceEnabled = false;
    bool m_sliceActivatedChanged = false;
    QRect m_primarySubView;
    QRect m_secondarySubView;

    bool m_gridUpdated = false;

    QtGraphs3D::GridLineType m_gridLineType = QtGraphs3D::GridLineType::Geometry;
    bool m_gridLineTypeDirty = false;

    bool m_validVolumeSlice = false;

    QVector3D m_labelPosition = QVector3D();
    QVector3D m_fontScaled = QVector3D();
    bool m_labelsNeedupdate = false;

    float m_initialZoomLevel = -1.0f;
    void setUpCamera();
    void setUpLight();
    void updateCamera();
    void updateRadialLabelOffset();
    QVector3D calculateLabelRotation(float labelAutoAngle);
    void updateCustomData();
    void updateCustomLabelsRotation();
    float fontScaleFactor(float pointSize);
    float labelAdjustment(float width);
    void gridLineCountHelper(QAbstract3DAxis *axis, qsizetype &lineCount, qsizetype &sublineCount);
    QVector3D graphPosToAbsolute(QVector3D position);

    void createVolumeMaterial(QCustom3DVolume *volume, Volume &volumeItem);
    QQuick3DModel *createSliceFrame(Volume &volumeItem);
    void updateSliceFrameMaterials(QCustom3DVolume *volume, Volume &volumeItem);
    void updateSubViews();
    void updateCustomVolumes();

    bool m_sliceUseOrthoProjection = false;

    QHash<QQuickGraphsItem *, QQuickWindow *> m_graphWindowList = {};
    QHash<QCustom3DLabel *, QQuick3DNode *> m_customLabelList = {};
    QHash<QCustom3DItem *, QQuick3DModel *> m_customItemList = {};
    QList<QCustom3DItem *> m_pendingCustomItemList = {};

    int m_currentFps = -1;
    bool m_measureFps = false;

    QtGraphs3D::CameraPreset m_activePreset = QtGraphs3D::CameraPreset::NoPreset;
    float m_xRotation = 0.0f;
    float m_yRotation = 0.0f;
    float m_minXRotation = -180.0f;
    float m_maxXRotation = 180.0f;
    float m_minYRotation = 0.0f;
    float m_maxYRotation = 90.0f;
    bool m_wrapXRotation = true;
    bool m_wrapYRotation = false;

    float m_zoomLevel = 100.0f;
    float m_minZoomLevel = 10.0f;
    float m_maxZoomLevel = 500.0f;

    QColor m_lightColor = QColor(Qt::white);
    float m_ambientLightStrength = 0.25f;
    float m_lightStrength = 5.0f;
    float m_shadowStrength = 25.0f;
    bool m_lightColorDirty = false;
    bool m_ambientLightStrengthDirty = false;
    bool m_lightStrengthDirty = false;
    bool m_shadowStrengthDirty = false;

    bool m_gridUpdate = false;

    QVector3D m_requestedTarget = QVector3D();

    QGraphsInputHandler *m_inputHandler = nullptr;

    QList<QGraphsTheme *> m_themes;
    QGraphsTheme *m_activeTheme = nullptr;

    friend class Q3DGraphsWidgetItem;
    friend class QQuickGraphsNode;
};

QT_END_NAMESPACE

#endif
