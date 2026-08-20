// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef SURFACEGRAPHMODIFIER_H
#define SURFACEGRAPHMODIFIER_H

#include <QtDataVisualization/q3dsurface.h>
#include <QtDataVisualization/qsurfacedataproxy.h>
#include <QtDataVisualization/qheightmapsurfacedataproxy.h>
#include <QtDataVisualization/qsurface3dseries.h>
#include <QtDataVisualization/qcustom3ditem.h>
#include <QtDataVisualization/qcustom3dlabel.h>
#include <QtDataVisualization/q3dinputhandler.h>
#include <QtWidgets/qslider.h>
#include <QtWidgets/qlabel.h>
#include <QtCore/qpropertyanimation.h>

class TopographicSeries;
class HighlightSeries;
class CustomInputHandler;

class SurfaceGraphModifier : public QObject
{
    Q_OBJECT
public:
    explicit SurfaceGraphModifier(Q3DSurface *surface, QLabel *label, QObject *parent);

    //! [0]
    void toggleModeNone() { m_graph->setSelectionMode(QAbstract3DGraph::SelectionNone); }
    void toggleModeItem() { m_graph->setSelectionMode(QAbstract3DGraph::SelectionItem); }
    void toggleModeSliceRow() { m_graph->setSelectionMode(QAbstract3DGraph::SelectionItemAndRow
                                                          | QAbstract3DGraph::SelectionSlice
                                                          | QAbstract3DGraph::SelectionMultiSeries); }
    void toggleModeSliceColumn() { m_graph->setSelectionMode(QAbstract3DGraph::SelectionItemAndColumn
                                                             | QAbstract3DGraph::SelectionSlice
                                                             | QAbstract3DGraph::SelectionMultiSeries); }
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

private:
    void setAxisXRange(float min, float max);
    void setAxisZRange(float min, float max);
    void fillSqrtSinProxy();
    void handleElementSelected(QAbstract3DGraph::ElementType type);
    void resetSelection();

private:
    Q3DSurface *m_graph = nullptr;
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
    QVector3D m_previousScaling = {};

    TopographicSeries *m_topography = nullptr;
    HighlightSeries *m_highlight = nullptr;
    int m_highlightWidth = 0;
    int m_highlightHeight = 0;

    CustomInputHandler *m_customInputHandler = nullptr;
    Q3DInputHandler *m_defaultInputHandler = new Q3DInputHandler();
};

#endif // SURFACEGRAPHMODIFIER_H
