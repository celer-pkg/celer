// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef GRAPHMODIFIER_H
#define GRAPHMODIFIER_H

#include <QtGraphsWidgets/Q3DSurfaceWidgetItem>
#include <QtGraphs/QSurfaceDataProxy>
#include <QtGraphs/QSurface3DSeries>
#include <QSlider>
#include <QTimer>
#include <QLabel>
#include <QCheckBox>

#define MULTI_SERIES

class GraphModifier : public QObject
{
    Q_OBJECT
public:
    enum Samples {
        SqrtSin = 1,
        Plane,
        Map
    };

    explicit GraphModifier(Q3DSurfaceWidgetItem *graph);
    ~GraphModifier();

    void toggleSeries1(int enabled);
    void toggleSeries2(int enabled);
    void toggleSeries3(int enabled);
    void toggleSeries4(int enabled);
    void toggleLineSeries(int enabled);
    void toggleSmooth(int enabled);
    void toggleSurfaceGrid(int enable);
    void toggleSurface(int enable);
    void toggleSeriesVisible(int enable);
    void toggleFill(int enable);
    void toggleSmoothS2(int visible);
    void toggleSurfaceGridS2(int enable);
    void toggleSurfaceS2(int enable);
    void toggleSeries2Visible(int enable);
    void toggleFillS2(int enable);
    void toggleSmoothS3(int visible);
    void toggleSurfaceGridS3(int enable);
    void toggleSurfaceS3(int enable);
    void toggleSeries3Visible(int enable);
    void toggleFillS3(int enable);
    void toggleSmoothS4(int visible);
    void toggleSurfaceGridS4(int enable);
    void toggleSurfaceS4(int enable);
    void toggleSeries4Visible(int enable);
    void toggleFillS4(int enable);
    void toggleLineSurfaceGrid(int enable);
    void toggleLineSurface(int enable);
    void toggleFillLine(int enable);

    void toggleSqrtSin(int enable);
    void togglePlane(int enable);
    void setHeightMapData(bool enable);
    void toggleGridSliderLock(bool enable);
    void setGridSliderX(QSlider *slider) { m_gridSliderX = slider; }
    void setGridSliderZ(QSlider *slider) { m_gridSliderZ = slider; }
    void setAxisRangeSliderX(QSlider *slider) { m_axisRangeSliderX = slider; }
    void setAxisRangeSliderZ(QSlider *slider) { m_axisRangeSliderZ = slider; }
    void setAxisMinSliderX(QSlider *slider) { m_axisMinSliderX = slider; }
    void setAxisMinSliderZ(QSlider *slider) { m_axisMinSliderZ = slider; }
    void setSeries1CB(QCheckBox *cb) { m_series1CB = cb; }
    void setSeries2CB(QCheckBox *cb) { m_series2CB = cb; }
    void setSeries3CB(QCheckBox *cb) { m_series3CB = cb; }
    void setSeries4CB(QCheckBox *cb) { m_series4CB = cb; }
    void adjustXCount(int count);
    void adjustZCount(int count);
    void adjustXRange(int range);
    void adjustYRange(int range);
    void adjustZRange(int range);
    void adjustXMin(int min);
    void adjustYMin(int min);
    void adjustZMin(int min);
    void adjustCutoffMargin(int margin);
    void updateSamples();
    void gradientPressed();
    void changeFont(const QFont &font);
    void changeStyle();
    void selectButtonClicked();
    void setSelectionInfoLabel(QLabel *label) {m_selectionInfoLabel = label; }
    void selectedPointChanged(const QPoint &point);
    void changeRow();
    void changeRows();
    void changeMesh();
    void changeItem();
    void changeMultipleItem();
    void changeMultipleRows();
    void addRow();
    void addRows();
    void insertRow();
    void insertRows();
    void removeRow();
    void resetArray();
    void resetArrayEmpty();
    void massiveDataTest();
    void massiveTestScroll();
    void massiveTestAppendAndScroll();
    void testAxisReverse();
    void testDataOrdering();
    void testNanSeries();
    void setRowSanitization(int enabled);
    void setAspectRatio(int ratio);
    void setHorizontalAspectRatio(int ratio);
    void setSurfaceTexture(int enabled);
    void setSurfaceAlphaTexture(int enabled);
    void setSurfaceAlphaTextureFile(int enabled);

public Q_SLOTS:
    void changeShadowQuality(int quality);
    void changeTransparencyTechnique(int technique);
    void changeColorScheme(int colorScheme);
    void changeTheme(int theme);
    void flipViews();
    void changeSubView();
    void changeSelectionMode(int mode);
    void timeout();
    void graphQueryTimeout();

    void handleAxisXChanged(QValue3DAxis *axis);
    void handleAxisYChanged(QValue3DAxis *axis);
    void handleAxisZChanged(QValue3DAxis *axis);
    void handleFpsChange(int fps);
    void changeLabelRotation(int rotation);
    void toggleAxisTitleVisibility(int visible);
    void toggleXAxisLabelsVisibility(int visible);
    void toggleYAxisLabelsVisibility(int visible);
    void toggleZAxisLabelsVisibility(int visible);
    void toggleAxisTitleFixed(int enabled);
    void toggleXAscending(int enabled);
    void toggleZAscending(int enabled);
    void togglePolar(int enabled);
    void setCameraTargetX(int value);
    void setCameraTargetY(int value);
    void setCameraTargetZ(int value);
    void setGraphMargin(int value);
    void setLabelMargin(int offset);

    void setXAxisSegemntCount(int count);
    void setYAxisSegemntCount(int count);
    void setZAxisSegemntCount(int count);
    void setXAxisSubsegemntCount(int count);
    void setYAxisSubsegemntCount(int count);
    void setZAxisSubsegemntCount(int count);

private:
    void fillSeries();
    void resetArrayAndSliders(QSurfaceDataArray array, float minZ, float maxZ, float minX,
                              float maxX);
    QSurfaceDataRow createMultiRow(int row, int series, bool change);
    void populateRisingSeries(QSurface3DSeries *series, int rows, int columns, float minValue,
                              float maxValue, bool ascendingX, bool ascendingZ);
    void populateNanSeries(QSurface3DSeries *series, int rows, int columns);

    Q3DSurfaceWidgetItem *m_graph;
    QSurface3DSeries *m_multiseries[4];
    QSurface3DSeries *m_series1;
    QSurface3DSeries *m_series2;
    QSurface3DSeries *m_series3;
    QSurface3DSeries *m_series4;
    QSurface3DSeries *m_lineSeries;
    QSurface3DSeries *m_nanSeries;
    QSlider *m_gridSliderX;
    QSlider *m_gridSliderZ;
    QSlider *m_axisRangeSliderX;
    QSlider *m_axisRangeSliderZ;
    QSlider *m_axisMinSliderX;
    QSlider *m_axisMinSliderZ;
    QCheckBox *m_series1CB;
    QCheckBox *m_series2CB;
    QCheckBox *m_series3CB;
    QCheckBox *m_series4CB;
    QCheckBox *m_lineSeriesCB;
    bool m_gridSlidersLocked;
    int m_xCount;
    int m_zCount;
    int m_activeSample;
    int m_fontSize;
    float m_rangeX;
    float m_rangeY;
    float m_rangeZ;
    float m_minX;
    float m_minY;
    float m_minZ;
    int m_addRowCounter;
    int m_insertTestZPos;
    int m_insertTestIndexPos;
    QTimer m_timer;
    QSurfaceDataArray m_planeArray;
    QLabel *m_selectionInfoLabel;
    QSurface3DSeries *m_theSeries;
    QSurface3DSeries::DrawFlags m_drawMode;
    QSurface3DSeries::DrawFlags m_drawMode2;
    QSurface3DSeries::DrawFlags m_drawMode3;
    QSurface3DSeries::DrawFlags m_drawMode4;
    QSurface3DSeries::DrawFlags m_drawMode5;
    float m_limitX;
    float m_limitZ;
    float m_offset;
    float m_multiSampleOffsetX[4];
    float m_multiSampleOffsetZ[4];
    QSurfaceDataArray m_massiveTestCacheArray;
    QVector3D m_cameraTarget;
    QTimer m_graphPositionQueryTimer;
    bool m_ascendingX;
    bool m_ascendingZ;
    bool m_customSubviews = false;
};

#endif
