// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef SCATTERDATAMODIFIER_H
#define SCATTERDATAMODIFIER_H

#include "axesinputhandler.h"

#include <QtDataVisualization/q3dscatter.h>
#include <QtDataVisualization/qabstract3dseries.h>

class ScatterDataModifier : public QObject
{
    Q_OBJECT
public:
    explicit ScatterDataModifier(Q3DScatter *scatter, QObject *parent);

    void addData();

public Q_SLOTS:
    void setBackgroundEnabled(int enabled);
    void setGridEnabled(int enabled);
    void setSmoothDots(int smooth);
    void changePresetCamera();
    void toggleItemCount();
    void toggleRanges();
    void changeStyle(int style);
    void changeTheme(int theme);
    void changeShadowQuality(int quality);
    void shadowQualityUpdatedByVisual(QAbstract3DGraph::ShadowQuality shadowQuality);

Q_SIGNALS:
    void backgroundEnabledChanged(bool enabled);
    void gridEnabledChanged(bool enabled);
    void shadowQualityChanged(int quality);

private:
    QVector3D randVector();
    Q3DScatter *m_graph = nullptr;
    QAbstract3DSeries::Mesh m_style = QAbstract3DSeries::MeshSphere;
    bool m_smooth = true;
    int m_itemCount;
    float m_curveDivider;

    AxesInputHandler *m_inputHandler;
    bool m_autoAdjust = true;
};

#endif
