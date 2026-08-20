// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef BARGRAPHMODIFIER_H
#define BARGRAPHMODIFIER_H

#include <QtCore/qpropertyanimation.h>
#include <QtGraphs/qabstract3dseries.h>
#include <QtGraphs/qbardataproxy.h>
#include <QtGraphsWidgets/q3dbarswidgetitem.h>

class RainfallData;

class BarGraphModifier : public QObject
{
    Q_OBJECT
public:
    explicit BarGraphModifier(Q3DBarsWidgetItem *bargraph, QObject *parent);
    virtual ~BarGraphModifier() = default;

    void resetTemperatureData();
    void changePresetCamera();

    void renderSliceToImage(QtGraphs3D::SliceCaptureType sliceType, int requestedIndex);
    void changeSelectionMode(bool checked, bool nopick = true);

Q_SIGNALS:
    void sliceImageChanged(const QImage &image);

private:
    Q3DBarsWidgetItem *m_graph = nullptr;
    int m_fontSize = 30;
    int m_segments = 4;
    int m_subSegments = 3;
    float m_minval = -20.f;
    float m_maxval = 20.f;
    QStringList m_months = {};
    QStringList m_years = {};
    QValue3DAxis *m_temperatureAxis = nullptr;
    QCategory3DAxis *m_yearAxis = nullptr;
    QCategory3DAxis *m_monthAxis = nullptr;
    QBar3DSeries *m_primarySeries = nullptr;
    QBar3DSeries *m_secondarySeries = nullptr;
    QAbstract3DSeries::Mesh m_barMesh = QAbstract3DSeries::Mesh::BevelBar;
    const QString m_celsiusString;
};

#endif
