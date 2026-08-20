// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef GRAPHMODIFIER_H
#define GRAPHMODIFIER_H

#include <QtDataVisualization/q3dbars.h>
#include <QtDataVisualization/qbardataproxy.h>
#include <QtDataVisualization/qabstract3dseries.h>
#include <QtCore/qpropertyanimation.h>

class RainfallData;

class GraphModifier : public QObject
{
    Q_OBJECT
public:
    explicit GraphModifier(Q3DBars *bargraph, QObject *parent);
    ~GraphModifier();

    void resetTemperatureData();
    void changePresetCamera();
    void changeLabelBackground();
    void changeFont(const QFont &font);
    void changeFontSize(int fontsize);
    void rotateX(int rotation);
    void rotateY(int rotation);
    void setBackgroundEnabled(int enabled);
    void setGridEnabled(int enabled);
    void setSmoothBars(int smooth);
    void setSeriesVisibility(int enabled);
    void setReverseValueAxis(int enabled);
    void setReflection(bool enabled);
    void changeDataMode(bool customData);

public Q_SLOTS:
    void changeRange(int range);
    void changeStyle(int style);
    void changeSelectionMode(int selectionMode);
    void changeTheme(int theme);
    void changeShadowQuality(int quality);
    void shadowQualityUpdatedByVisual(QAbstract3DGraph::ShadowQuality shadowQuality);
    void changeLabelRotation(int rotation);
    void setAxisTitleVisibility(bool enabled);
    void setAxisTitleFixed(bool enabled);
    void zoomToSelectedBar();
    void setDataModeToWeather(bool enabled);
    void setDataModeToCustom(bool enabled);

Q_SIGNALS:
    void shadowQualityChanged(int quality);
    void backgroundEnabledChanged(bool enabled);
    void gridEnabledChanged(bool enabled);
    void fontChanged(const QFont &font);
    void fontSizeChanged(int size);

private:
    Q3DBars *m_graph = nullptr;
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
    QAbstract3DSeries::Mesh m_barMesh = QAbstract3DSeries::MeshBevelBar;
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
