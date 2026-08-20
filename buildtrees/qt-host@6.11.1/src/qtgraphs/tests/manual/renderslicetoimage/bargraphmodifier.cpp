// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "bargraphmodifier.h"

#include <QtCore/qmath.h>
#include <QtGraphs/q3dscene.h>
#include <QtGraphs/qbar3dseries.h>
#include <QtGraphs/qbardataproxy.h>
#include <QtGraphs/qcategory3daxis.h>
#include <QtGraphs/qgraphstheme.h>
#include <QtGraphs/qvalue3daxis.h>
#include <QtWidgets/qcombobox.h>

using namespace Qt::StringLiterals;

// TODO: Many of the values do not affect custom proxy series now - should be fixed

//! [set up the graph]
BarGraphModifier::BarGraphModifier(Q3DBarsWidgetItem *bargraph, QObject *parent)
    : QObject(parent)
    , m_graph(bargraph)
    , m_temperatureAxis(new QValue3DAxis)
    , m_yearAxis(new QCategory3DAxis)
    , m_monthAxis(new QCategory3DAxis)
    , m_primarySeries(new QBar3DSeries)
    , m_secondarySeries(new QBar3DSeries)
    , m_celsiusString(u"°C"_s)
{
    m_graph->setShadowQuality(QtGraphs3D::ShadowQuality::SoftMedium);
    m_graph->setMultiSeriesUniform(true);
    m_graph->activeTheme()->setPlotAreaBackgroundVisible(false);
    m_graph->activeTheme()->setLabelFont(QFont("Times New Roman", m_fontSize));
    m_graph->activeTheme()->setLabelBackgroundVisible(true);
    m_graph->setSelectionMode(QtGraphs3D::SelectionFlag::None);

    m_months = {"January",
                "February",
                "March",
                "April",
                "May",
                "June",
                "July",
                "August",
                "September",
                "October",
                "November",
                "December"};
    m_years = {"2015", "2016", "2017", "2018", "2019", "2020", "2021", "2022"};

    m_temperatureAxis->setTitle("Average temperature");
    m_temperatureAxis->setSegmentCount(m_segments);
    m_temperatureAxis->setSubSegmentCount(m_subSegments);
    m_temperatureAxis->setRange(m_minval, m_maxval);
    m_temperatureAxis->setLabelFormat(u"%.1f "_s + m_celsiusString);
    m_temperatureAxis->setLabelAutoAngle(30.0f);
    m_temperatureAxis->setTitleVisible(true);

    m_yearAxis->setTitle("Year");
    m_yearAxis->setLabelAutoAngle(30.0f);
    m_yearAxis->setTitleVisible(true);

    m_monthAxis->setTitle("Month");
    m_monthAxis->setLabelAutoAngle(30.0f);
    m_monthAxis->setTitleVisible(true);

    m_graph->setValueAxis(m_temperatureAxis);
    m_graph->setRowAxis(m_yearAxis);
    m_graph->setColumnAxis(m_monthAxis);

    m_primarySeries->setItemLabelFormat(u"Oulu - @colLabel @rowLabel: @valueLabel"_s);
    m_primarySeries->setMesh(QAbstract3DSeries::Mesh::BevelBar);
    m_primarySeries->setMeshSmooth(false);

    m_secondarySeries->setItemLabelFormat(u"Helsinki - @colLabel @rowLabel: @valueLabel"_s);
    m_secondarySeries->setMesh(QAbstract3DSeries::Mesh::BevelBar);
    m_secondarySeries->setMeshSmooth(false);
    m_secondarySeries->setVisible(false);

    m_graph->addSeries(m_primarySeries);
    m_graph->addSeries(m_secondarySeries);

    changePresetCamera();

    resetTemperatureData();

    connect(m_graph,
            &Q3DBarsWidgetItem::sliceImageChanged,
            this,
            &BarGraphModifier::sliceImageChanged);
}

void BarGraphModifier::resetTemperatureData()
{
    static const float tempOulu[8][12] = {
        {-7.4f, -2.4f, 0.0f, 3.0f, 8.2f, 11.6f, 14.7f, 15.4f, 11.4f, 4.2f, 2.1f, -2.3f},     // 2015
        {-13.4f, -3.9f, -1.8f, 3.1f, 10.6f, 13.7f, 17.8f, 13.6f, 10.7f, 3.5f, -3.1f, -4.2f}, // 2016
        {-5.7f, -6.7f, -3.0f, -0.1f, 4.7f, 12.4f, 16.1f, 14.1f, 9.4f, 3.0f, -0.3f, -3.2f},   // 2017
        {-6.4f, -11.9f, -7.4f, 1.9f, 11.4f, 12.4f, 21.5f, 16.1f, 11.0f, 4.4f, 2.1f, -4.1f},  // 2018
        {-11.7f, -6.1f, -2.4f, 3.9f, 7.2f, 14.5f, 15.6f, 14.4f, 8.5f, 2.0f, -3.0f, -1.5f},   // 2019
        {-2.1f, -3.4f, -1.8f, 0.6f, 7.0f, 17.1f, 15.6f, 15.4f, 11.1f, 5.6f, 1.9f, -1.7f},    // 2020
        {-9.6f, -11.6f, -3.2f, 2.4f, 7.8f, 17.3f, 19.4f, 14.2f, 8.0f, 5.2f, -2.2f, -8.6f},   // 2021
        {-7.3f, -6.4f, -1.8f, 1.3f, 8.1f, 15.5f, 17.6f, 17.6f, 9.1f, 5.4f, -1.5f, -4.4f}     // 2022
    };

    static const float tempHelsinki[8][12] = {
        {-2.0f, -0.1f, 1.8f, 5.1f, 9.7f, 13.7f, 16.3f, 17.3f, 12.7f, 5.4f, 4.6f, 2.1f},     // 2015
        {-10.3f, -0.6f, 0.0f, 4.9f, 14.3f, 15.7f, 17.7f, 16.0f, 12.7f, 4.6f, -1.0f, -0.9f}, // 2016
        {-2.9f, -3.3f, 0.7f, 2.3f, 9.9f, 13.8f, 16.1f, 15.9f, 11.4f, 5.0f, 2.7f, 0.7f},     // 2017
        {-2.2f, -8.4f, -4.7f, 5.0f, 15.3f, 15.8f, 21.2f, 18.2f, 13.3f, 6.7f, 2.8f, -2.0f},  // 2018
        {-6.2f, -0.5f, -0.3f, 6.8f, 10.6f, 17.9f, 17.5f, 16.8f, 11.3f, 5.2f, 1.8f, 1.4f},   // 2019
        {1.9f, 0.5f, 1.7f, 4.5f, 9.5f, 18.4f, 16.5f, 16.8f, 13.0f, 8.2f, 4.4f, 0.9f},       // 2020
        {-4.7f, -8.1f, -0.9f, 4.5f, 10.4f, 19.2f, 20.9f, 15.4f, 9.5f, 8.0f, 1.5f, -6.7f},   // 2021
        {-3.3f, -2.2f, -0.2f, 3.3f, 9.6f, 16.9f, 18.1f, 18.9f, 9.2f, 7.6f, 2.3f, -3.4f}     // 2022
    };

    QBarDataArray dataSet;
    QBarDataArray dataSet2;

    dataSet.reserve(m_years.size());
    for (qsizetype year = 0; year < m_years.size(); ++year) {
        QBarDataRow dataRow(m_months.size());
        QBarDataRow dataRow2(m_months.size());
        for (qsizetype month = 0; month < m_months.size(); ++month) {
            dataRow[month].setValue(tempOulu[year][month]);
            dataRow2[month].setValue(tempHelsinki[year][month]);
        }
        dataSet.append(dataRow);
        dataSet2.append(dataRow2);
    }

    m_primarySeries->dataProxy()->resetArray(dataSet, m_years, m_months);
    m_secondarySeries->dataProxy()->resetArray(dataSet2, m_years, m_months);
}

void BarGraphModifier::changePresetCamera()
{
    m_graph->setCameraTargetPosition(QVector3D(0.0f, 0.0f, 0.0f));

    static int preset = int(QtGraphs3D::CameraPreset::Front);

    m_graph->setCameraPreset((QtGraphs3D::CameraPreset) preset);

    if (++preset > int(QtGraphs3D::CameraPreset::DirectlyBelow))
        preset = int(QtGraphs3D::CameraPreset::FrontLow);
}

void BarGraphModifier::renderSliceToImage(QtGraphs3D::SliceCaptureType sliceType, int requestedIndex)
{
    m_graph->renderSliceToImage(requestedIndex, sliceType);
}

void BarGraphModifier::changeSelectionMode(bool checked, bool nopick)
{
    auto mode = nopick
                    ? QtGraphs3D::SelectionFlag::None
                    : (checked ? (QtGraphs3D::SelectionFlag::Row | QtGraphs3D::SelectionFlag::Slice)
                               : (QtGraphs3D::SelectionFlag::Column
                                  | QtGraphs3D::SelectionFlag::Slice));
    m_graph->setSelectionMode(mode);
}
