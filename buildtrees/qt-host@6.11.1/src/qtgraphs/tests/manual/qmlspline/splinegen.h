// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
#ifndef SPLINEGEN_H
#define SPLINEGEN_H

#include <QtGraphs>

enum class SplineType {
    Circle,
    Helix,
    Stitch,
};
class SplineGen : public QObject
{
    Q_OBJECT
    Q_ENUM(SplineType)
public:
    SplineGen(QObject *parent = 0);
    ~SplineGen() override;

public Q_SLOTS:
    void generateSpline(QScatter3DSeries *series, SplineType type, int points);
    void tickSpline(QScatter3DSeries *series);

private:
    void genCircle(int points);
    void genHelix(int points);
    void genStitch(int points);
    int m_cacheCount = 60;

    QScatter3DSeries *m_series = nullptr;
    QList<QScatterDataArray> m_splineCache;
};

#endif // SPLINEGEN_H
