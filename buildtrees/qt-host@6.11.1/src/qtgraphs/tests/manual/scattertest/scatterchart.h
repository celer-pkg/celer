// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef SCATTERDATAMODIFIER_H
#define SCATTERDATAMODIFIER_H

#include <QtGraphsWidgets/q3dscatterwidgetitem.h>
#include <QtGraphs/qscatter3dseries.h>

#include <QFont>
#include <QDebug>
#include <QTimer>
#include <QLabel>

class ScatterDataModifier : public QObject
{
    Q_OBJECT
public:
    explicit ScatterDataModifier(Q3DScatterWidgetItem *scatter);
    ~ScatterDataModifier();

    void addData();
    void changeStyle();
    void changePresetCamera();
    void changeTheme();
    void changeLabelStyle();
    void changeFont(const QFont &font);
    void changeFontSize(int fontSize);
    void changeLabelSize(int labelSize);
    void changePointSize(int pointSize);
    void setBackgroundVisible(int visible);
    void setGridVisible(int visible);
    void setMinX(int min);
    void setMinY(int min);
    void setMinZ(int min);
    void setMaxX(int max);
    void setMaxY(int max);
    void setMaxZ(int max);
    void setCutoffMargin(int margin);
    void setAspectRatio(int ratio);
    void setHorizontalAspectRatio(int ratio);
    void start();
    void massiveDataTest();
    void massiveTestScroll();
    void massiveTestAppendAndScroll();
    void setFpsMeasurement(int enable);
    void setFpsLabel(QLabel *fpsLabel) { m_fpsLabel = fpsLabel; }
    void testItemChanges();
    void testAxisReverse();
    void testNanSeries();

public Q_SLOTS:
    void changeShadowQuality(int quality);
    void shadowQualityUpdatedByVisual(QtGraphs3D::ShadowQuality shadowQuality);
    void changeLightingMode(int mode);
    void clear();
    void resetAxes();
    void addOne();
    void addBunch();
    void insertOne();
    void insertBunch();
    void changeOne();
    void changeBunch();
    void removeOne();
    void removeBunch();
    void timeout();
    void startStopTimer();
    void selectItem();
    void handleSelectionChange(qsizetype index);
    void setGradient();
    void clearSeriesData();
    void addSeries();
    void removeSeries();
    void toggleSeriesVisibility();
    void changeSeriesName();
    void handleAxisXChanged(QValue3DAxis *axis);
    void handleAxisYChanged(QValue3DAxis *axis);
    void handleAxisZChanged(QValue3DAxis *axis);
    void handleFpsChange(int fps);
    void changeLabelRotation(int rotation);
    void changeTitleOffset(int offset);
    void changeRadialLabelOffset(int offset);
    void toggleAxisTitleVisibility(int enabled);
    void toggleAxisTitleFixed(int enabled);
    void toggleLabelAdjustment(int enabled);
    void renderToImage();
    void togglePolar(int enable);
    void toggleLegacy(int enable);
    void toggleOrtho(int enable);
    void setCameraTargetX(int value);
    void setCameraTargetY(int value);
    void setCameraTargetZ(int value);
    void setGraphMargin(int value);
    void setXAxisSegemntCount(int count);
    void setYAxisSegemntCount(int count);
    void setZAxisSegemntCount(int count);
    void setXAxisSubsegemntCount(int count);
    void setYAxisSubsegemntCount(int count);
    void setZAxisSubsegemntCount(int count);

Q_SIGNALS:
    void shadowQualityChanged(int quality);

private:
    QVector3D randVector();
    QScatter3DSeries *createAndAddSeries();
    void populateFlatSeries(QScatter3DSeries *series, int rows, int columns, float value);
    void populateRisingSeries(QScatter3DSeries *series, int rows, int columns, float minValue,
                              float maxValue);

    void populateNanSeries(QScatter3DSeries *series, int rows, int columns, float value);
    void deleteAxis(QValue3DAxis *axis);

    Q3DScatterWidgetItem *m_chart;
    int m_fontSize;
    QTimer m_timer;
    int m_loopCounter;
    qsizetype m_selectedItem;
    QScatter3DSeries *m_targetSeries;
    QScatterDataArray m_massiveTestCacheArray;
    QLabel *m_fpsLabel;
    QVector3D m_cameraTarget;

};

#endif
