// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef SURFACEGRAPHWIDGET_H
#define SURFACEGRAPHWIDGET_H

#include <QtQuickWidgets/qquickwidget.h>

class Q3DSurfaceWidgetItem;

class SurfaceGraphWidget : public QQuickWidget
{
    Q_OBJECT
public:
    SurfaceGraphWidget();
    ~SurfaceGraphWidget() override;

    void initialize();
    Q3DSurfaceWidgetItem *surfaceGraph() const;

private:
    Q3DSurfaceWidgetItem *m_surfaceGraph = nullptr;
};

#endif // SURFACEGRAPHWIDGET_H
