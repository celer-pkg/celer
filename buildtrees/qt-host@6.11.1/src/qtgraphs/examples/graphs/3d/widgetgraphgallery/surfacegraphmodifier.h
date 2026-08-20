// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef SURFACEGRAPHMODIFIER_H
#define SURFACEGRAPHMODIFIER_H

#include <QtCore/qpropertyanimation.h>
#include <QtGraphs/qcustom3ditem.h>
#include <QtGraphs/qcustom3dlabel.h>
#include <QtGraphs/qheightmapsurfacedataproxy.h>
#include <QtGraphs/qsurface3dseries.h>
#include <QtGraphs/qsurfacedataproxy.h>
#include <QtGraphsWidgets/q3dsurfacewidgetitem.h>
#include <QtWidgets/qlabel.h>
#include <QtWidgets/qslider.h>

class TopographicSeries;
class HighlightSeries;

class SurfaceGraphModifier : public QObject
{
    Q_OBJECT

    enum InputState : unsigned short { StateNormal = 0, StateDraggingX, StateDraggingZ, StateDraggingY };

public:
    Q_DISABLE_COPY_MOVE(SurfaceGraphModifier)

    explicit SurfaceGraphModifier(Q3DSurfaceWidgetItem *surface, QLabel *label, QObject *parent);
    ~SurfaceGraphModifier() override;

    //! [0]
    void toggleModeNone() { m_graph->setSelectionMode(QtGraphs3D::SelectionFlag::None); }
    void toggleModeItem() { m_graph->setSelectionMode(QtGraphs3D::SelectionFlag::Item); }
    void toggleModeSliceRow()
    {
        m_graph->setSelectionMode(QtGraphs3D::SelectionFlag::ItemAndRow
                                  | QtGraphs3D::SelectionFlag::Slice
                                  | QtGraphs3D::SelectionFlag::MultiSeries);
    }
    void toggleModeSliceColumn()
    {
        m_graph->setSelectionMode(QtGraphs3D::SelectionFlag::ItemAndColumn
                                  | QtGraphs3D::SelectionFlag::Slice
                                  | QtGraphs3D::SelectionFlag::MultiSeries);
    }
    //! [0]

    void setBlackToYellowGradient();
    void setGreenToRedGradient();

    void setAxisMinSliderX(QSlider *slider) { m_axisMinSliderX = slider; }
    void setAxisMaxSliderX(QSlider *slider) { m_axisMaxSliderX = slider; }
    void setAxisMinSliderZ(QSlider *slider) { m_axisMinSliderZ = slider; }
    void setAxisMaxSliderZ(QSlider *slider) { m_axisMaxSliderZ = slider; }

    void adjustXMin(int min);
    void adjustXMax(int max);
    void adjustZMin(int min);
    void adjustZMax(int max);

public Q_SLOTS:
    void enableSqrtSinModel(bool enable);
    void enableHeightMapModel(bool enable);
    void enableTopographyModel(bool enable);

    void toggleItemOne(bool show);
    void toggleItemTwo(bool show);
    void toggleItemThree(bool show);
    void toggleSeeThrough(bool seethrough);
    void toggleOilHighlight(bool highlight);
    void toggleShadows(bool shadows);
    void toggleSurfaceTexture(bool enable);

    void handleAxisDragging(QVector2D delta);
    void onWheel(QWheelEvent *event);

private:
    void setAxisXRange(float min, float max);
    void setAxisZRange(float min, float max);
    void fillSqrtSinProxy();
    void handleElementSelected(QtGraphs3D::ElementType type);
    void resetSelection();

private:
    Q3DSurfaceWidgetItem *m_graph = nullptr;
    QSurfaceDataProxy *m_sqrtSinProxy = nullptr;
    QSurface3DSeries *m_sqrtSinSeries = nullptr;
    QHeightMapSurfaceDataProxy *m_heightMapProxyOne = nullptr;
    QHeightMapSurfaceDataProxy *m_heightMapProxyTwo = nullptr;
    QHeightMapSurfaceDataProxy *m_heightMapProxyThree = nullptr;
    QSurface3DSeries *m_heightMapSeriesOne = nullptr;
    QSurface3DSeries *m_heightMapSeriesTwo = nullptr;
    QSurface3DSeries *m_heightMapSeriesThree = nullptr;

    QSlider *m_axisMinSliderX = nullptr;
    QSlider *m_axisMaxSliderX = nullptr;
    QSlider *m_axisMinSliderZ = nullptr;
    QSlider *m_axisMaxSliderZ = nullptr;
    float m_rangeMinX = 0.f;
    float m_rangeMinZ = 0.f;
    float m_stepX = 0.f;
    float m_stepZ = 0.f;
    int m_heightMapWidth = 0;
    int m_heightMapHeight = 0;

    QLabel *m_textField = nullptr;
    QPropertyAnimation *m_selectionAnimation = nullptr;
    QCustom3DLabel *m_titleLabel = nullptr;
    QCustom3DItem *m_previouslyAnimatedItem = nullptr;
    QVector3D m_previousScaling;

    TopographicSeries *m_topography = nullptr;
    HighlightSeries *m_highlight = nullptr;
    int m_highlightWidth = 0;
    int m_highlightHeight = 0;

    bool m_mousePressed = false;
    InputState m_state = StateNormal;
    float m_speedModifier = 20.f;
    float m_aspectRatio = 0.f;
    float m_axisXMinValue = 0.f;
    float m_axisXMaxValue = 0.f;
    float m_axisXMinRange = 0.f;
    float m_axisZMinValue = 0.f;
    float m_axisZMaxValue = 0.f;
    float m_axisZMinRange = 0.f;
    float m_areaMinValue = 0.f;
    float m_areaMaxValue = 0.f;

    void checkConstraints();
};

#endif // SURFACEGRAPHMODIFIER_H
