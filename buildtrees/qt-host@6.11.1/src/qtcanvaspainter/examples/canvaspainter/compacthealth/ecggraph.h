// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef ECGGRAPH_H
#define ECGGRAPH_H

#include <QList>
#include "theme.h"

QT_FORWARD_DECLARE_CLASS(QCanvasPainter)
class MainWindow;

class ECGGraph
{

public:
    explicit ECGGraph();

    void paintWaves(QCanvasPainter *painter, float x, float y, float w, float h);
    void initialize();
    void updateData(float hr);

private:
    friend class MainWindow;
    void paintWave(float x, float y, float w, float h);
    void paintSpo2Wave(float x, float y, float w, float h);

    void fillEmptyData();
    void fillRandomData();
    void fillBeatData();

    MainWindow *m_mainWindow;
    QCanvasPainter *m_painter;
    Theme *m_theme;
    // Wave data of each graph. Amount of graphs is m_waveAmount.
    QList<float> m_waveData;
    // Index of new data arriving
    int m_heartDataIndex = 0;
    // Beat index for creating dummy data. Values > -1 mean that beat is ongoing.
    int m_beatDataIndex = -1;
    // Amount of data points per graph
    int m_dataLength = 250;
    // How big is the space between fresh data & old data, in items
    int m_clearAmount = 20;
    // Index of old data disappearing
    int m_clearDataIndex = 0;
    // How often a heart pulse comes, every nth frame
    // On 60Hz screen, 40 means HR = 60 * (60/40) = 90
    int m_pulseRate = 40;
    // Currently rendering frame
    int m_currentFrame = 0;
    // Frame where current pulse has started
    int m_beatFrame = 0;
    // True when the sensors are attached
    bool m_attached = true;
    // True when defibrillator is attached, causing random spikes
    bool m_defibrillator = false;
    float m_lineWidth = 2.0;
    QList<float> m_spo2Data;

};

#endif // ECGGRAPH_H
