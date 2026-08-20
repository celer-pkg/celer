// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef SCATTERDATAMODIFIER_H
#define SCATTERDATAMODIFIER_H

#include <QtGraphs/qabstract3dseries.h>
#include <QtGraphsWidgets/q3dscatterwidgetitem.h>

class ScatterDataModifier : public QObject
{
    Q_OBJECT

    enum InputState : unsigned short { StateNormal = 0, StateDraggingX, StateDraggingZ, StateDraggingY };

public:
    Q_DISABLE_COPY_MOVE(ScatterDataModifier)

    explicit ScatterDataModifier(Q3DScatterWidgetItem *scatter, QObject *parent);
    ~ScatterDataModifier() override;

    void addData();

public Q_SLOTS:
    void setBackgroundVisible(int visible);
    void setGridVisible(int visible);
    void setSmoothDots(int smooth);
    void changePresetCamera();
    void toggleItemCount();
    void toggleRanges();
    void adjustMinimumRange(float range);
    void adjustMaximumRange(float range);
    void changeStyle(int style);
    void changeTheme(int theme);
    void changeShadowQuality(int quality);
    void shadowQualityUpdatedByVisual(QtGraphs3D::ShadowQuality shadowQuality);
    void handleElementSelected(QtGraphs3D::ElementType type);
    void handleAxisDragging(QVector2D delta);

Q_SIGNALS:
    void backgroundVisibleChanged(bool visible);
    void gridVisibleChanged(bool visible);
    void shadowQualityChanged(int quality);

private:
    QVector3D randVector();
    Q3DScatterWidgetItem *m_graph = nullptr;
    QAbstract3DSeries::Mesh m_style = QAbstract3DSeries::Mesh::Sphere;
    bool m_smooth = true;
    int m_itemCount;
    float m_curveDivider;

    bool m_autoAdjust = true;

    InputState m_state = StateNormal;
    float m_dragSpeedModifier = 15.f;
};

#endif
