// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
#ifndef SURFACEGRAPHWIDGET_H
#define SURFACEGRAPHWIDGET_H

#include <QtGraphsWidgets/q3dsurfacewidgetitem.h>
#include <QtQuickWidgets/qquickwidget.h>

class SurfaceGraphWidget : public QQuickWidget
{
    Q_OBJECT
public:
    Q_DISABLE_COPY_MOVE(SurfaceGraphWidget)

    SurfaceGraphWidget();
    ~SurfaceGraphWidget() override;

    void initialize();
    Q3DSurfaceWidgetItem *surfaceGraph() const;

private:
    Q3DSurfaceWidgetItem *m_surfaceGraph = nullptr;
};

#endif // SURFACEGRAPHWIDGET_H
