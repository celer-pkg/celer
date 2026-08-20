// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef GRAPHMODIFIER_H
#define GRAPHMODIFIER_H

#include <QtCore/qpropertyanimation.h>
#include <QtGraphs/qabstract3dseries.h>
#include <QtGraphs/qbardataproxy.h>
#include <QtGraphsWidgets/q3dbarswidgetitem.h>

class RainfallData;

class GraphModifier : public QObject
{
    Q_OBJECT
public:
    Q_DISABLE_COPY_MOVE(GraphModifier)

    explicit GraphModifier(Q3DBarsWidgetItem *bargraph, QObject *parent);
    ~GraphModifier() override;

    void resetTemperatureData();
    void changePresetCamera();
    void changeLabelBackground();
    void changeFont(const QFont &font);
    void changeFontSize(int fontsize);
    void rotateX(int angle);
    void rotateY(int angle);
    void setBackgroundVisible(int visible);
    void setGridVisible(int visible);
    void setSmoothBars(int smooth);
    void setSeriesVisibility(int visible);
    void setReverseValueAxis(int enabled);
    void changeDataMode(bool modelData);

public Q_SLOTS:
    void changeRange(int range);
    void changeStyle(int style);
    void changeSelectionMode(int selectionMode);
    void changeTheme(int theme);
    void changeShadowQuality(int quality);
    void shadowQualityUpdatedByVisual(QtGraphs3D::ShadowQuality shadowQuality);
    void changeLabelRotation(int rotation);
    void setAxisTitleVisibility(bool visible);
    void setAxisTitleFixed(bool enabled);
    void zoomToSelectedBar();
    void setDataModeToWeather(bool enabled);
    void setDataModeToModel(bool enabled);

Q_SIGNALS:
    void shadowQualityChanged(int quality);
    void backgroundVisibleChanged(bool visible);
    void gridVisibleChanged(bool visible);
    void fontChanged(const QFont &font);
    void fontSizeChanged(int size);

private:
    Q3DBarsWidgetItem *m_graph = nullptr;
    float m_xRotation = 0.f;
    float m_yRotation = 0.f;
    int m_fontSize = 30;
    int m_segments = 4;
    int m_subSegments = 3;
    float m_minval = -20.f;
    float m_maxval = 20.f;
    QStringList m_months = {};
    QStringList m_years = {};
    QValue3DAxis *m_temperatureAxis = nullptr;
    QCategory3DAxis *m_yearAxis = nullptr;
    QCategory3DAxis *m_monthAxis = nullptr;
    QBar3DSeries *m_primarySeries = nullptr;
    QBar3DSeries *m_secondarySeries = nullptr;
    QAbstract3DSeries::Mesh m_barMesh = QAbstract3DSeries::Mesh::BevelBar;
    bool m_smooth = false;
    QPropertyAnimation m_animationCameraX = {};
    QPropertyAnimation m_animationCameraY = {};
    QPropertyAnimation m_animationCameraZoom = {};
    QPropertyAnimation m_animationCameraTarget = {};
    float m_defaultAngleX = 0.f;
    float m_defaultAngleY = 0.f;
    float m_defaultZoom = 0.f;
    QVector3D m_defaultTarget = {};
    const QString m_celsiusString;
    RainfallData *m_customData = nullptr;
};

#endif
