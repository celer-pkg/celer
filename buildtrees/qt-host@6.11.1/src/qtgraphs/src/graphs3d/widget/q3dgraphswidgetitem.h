// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QTGRAPHS_Q3DGRAPHSWIDGETITEM_H
#define QTGRAPHS_Q3DGRAPHSWIDGETITEM_H

#include <QtCore/qlocale.h>
#include <QtGraphs/q3dscene.h>
#include <QtGraphs/qgraphs3dnamespace.h>
#include <QtGraphs/qgraphstheme.h>
#include <QtGraphsWidgets/qgraphswidgetsglobal.h>
#include <QtQuick/qquickitemgrabresult.h>
#include <QtQuickWidgets/qquickwidget.h>

QT_BEGIN_NAMESPACE

class QCustom3DItem;
class QAbstract3DAxis;
class QAbstract3DSeries;
class QQuickGraphsItem;
class QQuickItemGrabResult;
class QQuickWheelEvent;
class Q3DGraphsWidgetItemPrivate;

class Q_GRAPHSWIDGETS_EXPORT Q3DGraphsWidgetItem : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("RegisterEnumClassesUnscoped", "false")
    Q_PROPERTY(QGraphsTheme *activeTheme READ activeTheme WRITE setActiveTheme NOTIFY activeThemeChanged)
    Q_PROPERTY(QtGraphs3D::SelectionFlags selectionMode READ selectionMode WRITE setSelectionMode
                   NOTIFY selectionModeChanged)
    Q_PROPERTY(QtGraphs3D::ShadowQuality shadowQuality READ shadowQuality WRITE setShadowQuality
                   NOTIFY shadowQualityChanged)
    Q_PROPERTY(QtGraphs3D::TransparencyTechnique transparencyTechnique READ transparencyTechnique
                   WRITE setTransparencyTechnique NOTIFY transparencyTechniqueChanged REVISION(6, 9))
    Q_PROPERTY(Q3DScene *scene READ scene CONSTANT)
    Q_PROPERTY(bool measureFps READ measureFps WRITE setMeasureFps NOTIFY measureFpsChanged)
    Q_PROPERTY(int currentFps READ currentFps NOTIFY currentFpsChanged)
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
            cutoffMarginChanged REVISION(6, 11))
    Q_PROPERTY(QtGraphs3D::CameraPreset cameraPreset READ cameraPreset WRITE setCameraPreset NOTIFY
                   cameraPresetChanged)
    Q_PROPERTY(float cameraXRotation READ cameraXRotation WRITE setCameraXRotation NOTIFY
                   cameraXRotationChanged)
    Q_PROPERTY(float cameraYRotation READ cameraYRotation WRITE setCameraYRotation NOTIFY
                   cameraYRotationChanged)
    Q_PROPERTY(float cameraZoomLevel READ cameraZoomLevel WRITE setCameraZoomLevel NOTIFY
                   cameraZoomLevelChanged)
    Q_PROPERTY(float minCameraZoomLevel READ minCameraZoomLevel WRITE setMinCameraZoomLevel NOTIFY
                   minCameraZoomLevelChanged)
    Q_PROPERTY(float maxCameraZoomLevel READ maxCameraZoomLevel WRITE setMaxCameraZoomLevel NOTIFY
                   maxCameraZoomLevelChanged)
    Q_PROPERTY(bool wrapCameraXRotation READ wrapCameraXRotation WRITE setWrapCameraXRotation NOTIFY
                   wrapCameraXRotationChanged)
    Q_PROPERTY(bool wrapCameraYRotation READ wrapCameraYRotation WRITE setWrapCameraYRotation NOTIFY
                   wrapCameraYRotationChanged)
    Q_PROPERTY(float minCameraXRotation READ minCameraXRotation WRITE setMinCameraXRotation NOTIFY
                       minCameraXRotationChanged)
    Q_PROPERTY(float maxCameraXRotation READ maxCameraXRotation WRITE setMaxCameraXRotation NOTIFY
                       maxCameraXRotationChanged)
    Q_PROPERTY(float minCameraYRotation READ minCameraYRotation WRITE setMinCameraYRotation NOTIFY
                       minCameraYRotationChanged)
    Q_PROPERTY(float maxCameraYRotation READ maxCameraYRotation WRITE setMaxCameraYRotation NOTIFY
                       maxCameraYRotationChanged)
    Q_PROPERTY(QVector3D cameraTargetPosition READ cameraTargetPosition WRITE
                   setCameraTargetPosition NOTIFY cameraTargetPositionChanged)
    Q_PROPERTY(int msaaSamples READ msaaSamples WRITE setMsaaSamples NOTIFY msaaSamplesChanged)
    Q_PROPERTY(bool rotationEnabled READ isRotationEnabled WRITE setRotationEnabled NOTIFY
                   rotationEnabledChanged)
    Q_PROPERTY(bool zoomAtTargetEnabled READ isZoomAtTargetEnabled WRITE setZoomAtTargetEnabled NOTIFY
                   zoomAtTargetEnabledChanged)
    Q_PROPERTY(bool selectionEnabled READ isSelectionEnabled WRITE setSelectionEnabled NOTIFY
                   selectionEnabledChanged)
    Q_PROPERTY(bool zoomEnabled READ isZoomEnabled WRITE setZoomEnabled NOTIFY
                   zoomEnabledChanged)

    Q_PROPERTY(QColor lightColor READ lightColor WRITE setLightColor NOTIFY lightColorChanged)
    Q_PROPERTY(float ambientLightStrength READ ambientLightStrength WRITE setAmbientLightStrength
                   NOTIFY ambientLightStrengthChanged)
    Q_PROPERTY(
        float lightStrength READ lightStrength WRITE setLightStrength NOTIFY lightStrengthChanged)
    Q_PROPERTY(float shadowStrength READ shadowStrength WRITE setShadowStrength NOTIFY
                   shadowStrengthChanged)
    Q_PROPERTY(QtGraphs3D::GridLineType gridLineType READ gridLineType WRITE setGridLineType NOTIFY
                   gridLineTypeChanged FINAL)

public:
    void addTheme(QGraphsTheme *theme);
    void releaseTheme(QGraphsTheme *theme);
    QGraphsTheme *activeTheme() const;
    void setActiveTheme(QGraphsTheme *activeTheme);
    QList<QGraphsTheme *> themes() const;

    void setTransparencyTechnique(QtGraphs3D::TransparencyTechnique technique);
    QtGraphs3D::TransparencyTechnique transparencyTechnique() const;

    QtGraphs3D::ShadowQuality shadowQuality() const;
    void setShadowQuality(const QtGraphs3D::ShadowQuality &shadowQuality);

    QtGraphs3D::SelectionFlags selectionMode() const;
    void setSelectionMode(const QtGraphs3D::SelectionFlags &selectionMode);

    Q3DScene *scene() const;

    void setMeasureFps(bool enable);
    bool measureFps() const;
    int currentFps() const;

    void setOrthoProjection(bool enable);
    bool isOrthoProjection() const;

    QtGraphs3D::ElementType selectedElement() const;

    void setAspectRatio(qreal ratio);
    qreal aspectRatio() const;

    void setOptimizationHint(QtGraphs3D::OptimizationHint hint);
    QtGraphs3D::OptimizationHint optimizationHint() const;

    void setPolar(bool enable);
    bool isPolar() const;

    void setLabelMargin(float margin);
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

    void clearSelection();

    bool hasSeries(QAbstract3DSeries *series) const;

    qsizetype addCustomItem(QCustom3DItem *item);
    void removeCustomItems();
    void removeCustomItem(QCustom3DItem *item);
    void removeCustomItemAt(QVector3D position);
    void releaseCustomItem(QCustom3DItem *item);
    QList<QCustom3DItem *> customItems() const;

    int selectedLabelIndex() const;
    QAbstract3DAxis *selectedAxis() const;

    qsizetype selectedCustomItemIndex() const;
    QCustom3DItem *selectedCustomItem() const;

    QSharedPointer<QQuickItemGrabResult> renderToImage(QSize imageSize = QSize()) const;

    QtGraphs3D::CameraPreset cameraPreset() const;
    void setCameraPreset(QtGraphs3D::CameraPreset preset);

    float cameraXRotation() const;
    void setCameraXRotation(float rotation);
    float cameraYRotation() const;
    void setCameraYRotation(float rotation);

    float minCameraXRotation() const;
    void setMinCameraXRotation(float rotation);
    float maxCameraXRotation() const;
    void setMaxCameraXRotation(float rotation);

    float minCameraYRotation() const;
    void setMinCameraYRotation(float rotation);
    float maxCameraYRotation() const;
    void setMaxCameraYRotation(float rotation);

    void setZoomAtTargetEnabled(bool enable);
    bool isZoomAtTargetEnabled() const;
    void setZoomEnabled(bool enable);
    bool isZoomEnabled() const;
    void setSelectionEnabled(bool enable);
    bool isSelectionEnabled() const;
    void setRotationEnabled(bool enable);
    bool isRotationEnabled() const;

    void setDefaultInputHandler();
    void unsetDefaultInputHandler();
    void unsetDefaultTapHandler();
    void unsetDefaultDragHandler();
    void unsetDefaultWheelHandler();
    void unsetDefaultPinchHandler();
    void setDragButton(Qt::MouseButtons button);

    float cameraZoomLevel() const;
    void setCameraZoomLevel(float level);

    float minCameraZoomLevel() const;
    void setMinCameraZoomLevel(float level);

    float maxCameraZoomLevel() const;
    void setMaxCameraZoomLevel(float level);

    QVector3D cameraTargetPosition() const;
    void setCameraTargetPosition(QVector3D target);

    bool wrapCameraXRotation() const;
    void setWrapCameraXRotation(bool wrap);

    bool wrapCameraYRotation() const;
    void setWrapCameraYRotation(bool wrap);

    void setCameraPosition(float horizontal, float vertical, float zoom = 100.0f);

    int msaaSamples() const;
    void setMsaaSamples(int samples);

    void doPicking(QPoint point);
    void doRayPicking(QVector3D origin, QVector3D direction);

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

    void setWidget(QQuickWidget *widget);
    QQuickWidget *widget() const;

    ~Q3DGraphsWidgetItem() override;

protected:
    Q3DGraphsWidgetItem(Q3DGraphsWidgetItemPrivate &dd, QObject *parent, const QString &graph);

    bool event(QEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;

Q_SIGNALS:
    void activeThemeChanged(QGraphsTheme *activeTheme);
    void shadowQualityChanged(QtGraphs3D::ShadowQuality quality);
    Q_REVISION(6, 9) void transparencyTechniqueChanged(QtGraphs3D::TransparencyTechnique technique);
    void selectionModeChanged(const QtGraphs3D::SelectionFlags selectionMode);
    void selectedElementChanged(QtGraphs3D::ElementType type);
    void measureFpsChanged(bool enabled);
    void currentFpsChanged(int fps);
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
    void minCameraXRotationChanged(float rotation);
    void minCameraYRotationChanged(float rotation);
    void maxCameraXRotationChanged(float rotation);
    void maxCameraYRotationChanged(float rotation);
    void wrapCameraXRotationChanged(bool wrap);
    void wrapCameraYRotationChanged(bool wrap);
    void msaaSamplesChanged(int samples);

    void tapped(QEventPoint eventPoint, Qt::MouseButton button);
    void doubleTapped(QEventPoint eventPoint, Qt::MouseButton button);
    void longPressed();
    void dragged(QVector2D delta);
    void wheel(QWheelEvent *event);
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

private:
    Q_DISABLE_COPY_MOVE(Q3DGraphsWidgetItem)
    Q_DECLARE_PRIVATE(Q3DGraphsWidgetItem)
};

QT_END_NAMESPACE

#endif
