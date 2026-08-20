// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef SCATTERDATAMODIFIER_H
#define SCATTERDATAMODIFIER_H

#include <QtGraphsWidgets/q3dscatterwidgetitem.h>
#include <QtGraphs/qabstract3dseries.h>
#include <QtGui/QFont>
#include <QtCore/QTimer>

class ScatterDataModifier : public QObject
{
    Q_OBJECT
public:
    explicit ScatterDataModifier(Q3DScatterWidgetItem *scatter);
    ~ScatterDataModifier();

    void fpsChanged(int fps);
    void addData();
    void changeStyle();
    void changePresetCamera();
    void changeLabelStyle();
    void changeFont(const QFont &font);
    void changeFontSize(int fontsize);
    void enableOptimization(int enabled);
    void setBackgroundVisible(int visible);
    void setGridVisible(int visible);
    void toggleRotation();
    void start();

public Q_SLOTS:
    void changeStyle(int style);
    void changeTheme(int theme);
    void changeShadowQuality(int quality);
    void shadowQualityUpdatedByVisual(QtGraphs3D::ShadowQuality shadowQuality);
    void triggerRotation();

Q_SIGNALS:
    void backgroundVisibleChanged(bool enabled);
    void gridVisibleChanged(bool enabled);
    void shadowQualityChanged(int quality);
    void fontChanged(const QFont &font);

private:
    Q3DScatterWidgetItem *m_graph;
    int m_fontSize;
    QAbstract3DSeries::Mesh m_style;
    bool m_smooth;
    QTimer m_rotationTimer;
};

#endif
