// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef TOPOGRAPHICSERIES_H
#define TOPOGRAPHICSERIES_H

#include <QtGraphs/qsurface3dseries.h>

class TopographicSeries : public QSurface3DSeries
{
    Q_OBJECT
public:
    Q_DISABLE_COPY_MOVE(TopographicSeries)

    TopographicSeries();
    ~TopographicSeries() override;

    void setTopographyFile(const QString &file, float width, float height);

    float sampleCountX() const { return m_sampleCountX; }
    float sampleCountZ() const { return m_sampleCountZ; }

private:
    float m_sampleCountX = 0.f;
    float m_sampleCountZ = 0.f;
};

#endif // TOPOGRAPHICSERIES_H
