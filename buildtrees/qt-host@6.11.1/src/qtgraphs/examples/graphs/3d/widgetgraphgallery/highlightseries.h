// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef HIGHLIGHTSERIES_H
#define HIGHLIGHTSERIES_H

#include <QtGraphs/qsurface3dseries.h>

#include "topographicseries.h"

class HighlightSeries : public QSurface3DSeries
{
    Q_OBJECT
public:
    Q_DISABLE_COPY_MOVE(HighlightSeries)

    explicit HighlightSeries();
    ~HighlightSeries() override;

    void setTopographicSeries(TopographicSeries *series);
    void setMinHeight(float height) { m_minHeight = height; }
    void handleZoomChange(float zoom);

public Q_SLOTS:
    void handlePositionChange(const QPoint &position);
    void handleGradientChange(float value);

private:
    int m_width = 100;
    int m_height = 100;
    int m_srcWidth = 0;
    int m_srcHeight = 0;
    QPoint m_position;
    TopographicSeries *m_topographicSeries = nullptr;
    float m_minHeight = 0.f;
    float m_heightAdjustment = 5.f;
};

#endif // HIGHLIGHTSERIES_H
