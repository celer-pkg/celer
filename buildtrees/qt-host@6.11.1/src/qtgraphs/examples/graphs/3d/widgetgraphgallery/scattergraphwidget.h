// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
#ifndef SCATTERGRAPHWIDGET_H
#define SCATTERGRAPHWIDGET_H

#include <QtGraphsWidgets/q3dscatterwidgetitem.h>
#include <QtQuickWidgets/qquickwidget.h>

class ScatterGraphWidget : public QQuickWidget
{
    Q_OBJECT
public:
    Q_DISABLE_COPY_MOVE(ScatterGraphWidget)

    ScatterGraphWidget();
    ~ScatterGraphWidget();
    void initialize();

    Q3DScatterWidgetItem *scatterGraph() const;

private:
    Q3DScatterWidgetItem *m_scatterGraph = nullptr;
};

#endif // SCATTERGRAPHWIDGET_H
