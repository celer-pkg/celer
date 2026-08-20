// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef BARGRAPHWIDGET_H
#define BARGRAPHWIDGET_H

#include <QtQuickWidgets/qquickwidget.h>

class Q3DBarsWidgetItem;

class BarGraphWidget : public QQuickWidget
{
    Q_OBJECT
public:
    BarGraphWidget();
    ~BarGraphWidget() override;

    void initialize();
    Q3DBarsWidgetItem *barGraph() const;

private:
    Q3DBarsWidgetItem *m_barGraph = nullptr;
};

#endif // BARGRAPHWIDGET_H
