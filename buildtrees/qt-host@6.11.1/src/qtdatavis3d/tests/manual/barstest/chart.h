// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef CHARTMODIFIER_H
#define CHARTMODIFIER_H

#include <QtDataVisualization/q3dbars.h>
#include <QtDataVisualization/q3dinputhandler.h>
#include <QtDataVisualization/qbar3dseries.h>
#include <QtDataVisualization/q3dtheme.h>
#include <QFont>
#include <QDebug>
#include <QStringList>
#include <QPointer>
#include <QColorDialog>
#include <QTimer>
#include <QLabel>

class GraphModifier : public QObject
{
    Q_OBJECT
public:
    explicit GraphModifier(Q3DBars *barchart, QColorDialog *colorDialog);
    ~GraphModifier();

    void resetTemperatureData();
    void addRow();
    void addRows();
    void insertRow();
    void insertRows();
    void changeItem();
    void changeRow();
    void changeRows();
    void removeRow();
    void removeRows();
    void changeStyle();
    void changePresetCamera();
    void changeTheme();
    void changeLabelStyle();
    void changeSelectionMode();
    void changeFont(const QFont &font);
    void changeFontSize(int fontsize);
    void rotateX(int rotation);
    void rotateY(int rotation);
    void setFpsMeasurement(int state);
    void setBackgroundEnabled(int enabled);
    void setGridEnabled(int enabled);
    void setSpecsRatio(int barwidth);
    void setSpecsZ(int bardepth);
    void setSpacingSpecsX(int spacing);
    void setSpacingSpecsZ(int spacing);
    void setMarginX(int margin);
    void setMarginZ(int margin);
    void setSampleCountX(int samples);
    void setSampleCountZ(int samples);
    void setMinX(int min);
    void setMinZ(int min);
    void setMinY(int min);
    void setMaxY(int max);
    void start();
    void restart(int dynamicData);
    void selectBar();
    void swapAxis();
    void releaseAxes();
    void releaseSeries();
    void createMassiveArray();
    void useOwnTheme(bool checked);
    void changeBaseColor(const QColor &color);
    void changeColorStyle(bool checked);
    void showFiveSeries();
    QBarDataArray *makeDummyData();
    void primarySeriesTest(bool checked);
    void insertRemoveTestToggle();
    void toggleRotation(bool checked);
    void useLogAxis(bool checked);
    void changeValueAxisFormat(const QString & text);
    void changeLogBase(const QString & text);
    void setFpsLabel(QLabel *fpsLabel) { m_fpsLabel = fpsLabel; }
    void addRemoveSeries();
    void testItemAndRowChanges(bool checked);
    void reverseValueAxis(int enabled);
    void setInputHandlerRotationEnabled(int enabled);
    void setInputHandlerZoomEnabled(int enabled);
    void setInputHandlerSelectionEnabled(int enabled);
    void setInputHandlerZoomAtTargetEnabled(int enabled);
    void setReflection(int enabled);
    void setReflectivity(int value);
    void toggleCustomItem();

public Q_SLOTS:
    void flipViews(bool checked);
    void setGradient(bool checked);
    void toggleMultiseriesScaling();
    void changeShadowQuality(int quality);
    void shadowQualityUpdatedByVisual(QAbstract3DGraph::ShadowQuality shadowQuality);
    void handleSelectionChange(const QPoint &position);
    void setUseNullInputHandler(int useNull);
    void changeValueAxisSegments(int value);

    void handleRowAxisChanged(QCategory3DAxis *axis);
    void handleColumnAxisChanged(QCategory3DAxis *axis);
    void handleValueAxisChanged(QValue3DAxis *axis);
    void handlePrimarySeriesChanged(QBar3DSeries *series);

    void insertRemoveTimerTimeout();
    void triggerSelection();
    void triggerRotation();
    void handleValueAxisLabelsChanged();
    void handleFpsChange(qreal fps);
    void setCameraTargetX(int value);
    void setCameraTargetY(int value);
    void setCameraTargetZ(int value);
    void setFloorLevel(int value);
    void setGraphMargin(int value);

Q_SIGNALS:
    void shadowQualityChanged(int quality);

private:
    void populateFlatSeries(QBar3DSeries *series, int rows, int columns, float value);
    QBarDataRow *createFlatRow(int columns, float value);

    Q3DBars *m_graph;
    QColorDialog *m_colorDialog;
    int m_columnCount;
    int m_rowCount;
    float m_xRotation;
    float m_yRotation;
    int m_static;
    float m_barSpacingX;
    float m_barSpacingZ;
    float m_barSeriesMarginX;
    float m_barSeriesMarginZ;
    int m_fontSize;
    int m_segments;
    int m_subSegments;
    float m_minval;
    float m_maxval;
    QStringList m_months;
    QStringList m_years;
    QPoint m_selectedBar;
    QBar3DSeries *m_selectedSeries;
    QValue3DAxis *m_autoAdjustingAxis;
    QValue3DAxis *m_fixedRangeAxis;
    QValue3DAxis *m_temperatureAxis;
    QCategory3DAxis *m_yearAxis;
    QCategory3DAxis *m_monthAxis;
    QCategory3DAxis *m_genericRowAxis;
    QCategory3DAxis *m_genericColumnAxis;
    QBar3DSeries *m_temperatureData;
    QBar3DSeries *m_temperatureData2;
    QBar3DSeries *m_genericData;
    QBar3DSeries *m_dummyData;
    QBar3DSeries *m_dummyData2;
    QBar3DSeries *m_dummyData3;
    QBar3DSeries *m_dummyData4;
    QBar3DSeries *m_dummyData5;
    QValue3DAxis *m_currentAxis;
    bool m_negativeValuesOn;
    bool m_useNullInputHandler;
    Q3DInputHandler *m_defaultInputHandler;
    Q3DTheme *m_ownTheme;
    Q3DTheme *m_builtinTheme;
    QTimer m_insertRemoveTimer;
    int m_insertRemoveStep;
    QAbstract3DInputHandler *m_customInputHandler;
    QTimer m_selectionTimer;
    QTimer m_rotationTimer;
    QLabel *m_fpsLabel;
    QBar3DSeries *m_extraSeries;
    QVector3D m_cameraTarget;
};

#endif
