// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "graphmodifier.h"
#include "rainfalldata.h"

#include <QtCore/qmath.h>
#include <QtGraphs/q3dscene.h>
#include <QtGraphs/qgraphstheme.h>
#include <QtGraphs/qbar3dseries.h>
#include <QtGraphs/qbardataproxy.h>
#include <QtGraphs/qcategory3daxis.h>
#include <QtGraphs/qvalue3daxis.h>
#include <QtWidgets/qcombobox.h>

using namespace Qt::StringLiterals;

// TODO: Many of the values do not affect custom proxy series now - should be fixed

//! [set up the graph]
GraphModifier::GraphModifier(Q3DBarsWidgetItem *bargraph, QObject *parent)
    : QObject(parent)
    , m_graph(bargraph)
    //! [set up the graph]
    //! [set up axes]
    , m_temperatureAxis(new QValue3DAxis)
    , m_yearAxis(new QCategory3DAxis)
    , m_monthAxis(new QCategory3DAxis)
    , m_primarySeries(new QBar3DSeries)
    , m_secondarySeries(new QBar3DSeries)
    //! [set up axes]
    , m_celsiusString(u"°C"_s)
{
    //! [adjust visuals]
    m_graph->setShadowQuality(QtGraphs3D::ShadowQuality::SoftMedium);
    m_graph->setMultiSeriesUniform(true);
    // These are set through the active theme
    m_graph->activeTheme()->setPlotAreaBackgroundVisible(false);
    m_graph->activeTheme()->setLabelFont(QFont("Times New Roman", m_fontSize));
    m_graph->activeTheme()->setLabelBackgroundVisible(true);
    //! [adjust visuals]

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

    //! [activate axes]
    m_temperatureAxis->setTitle("Average temperature");
    m_temperatureAxis->setSegmentCount(m_segments);
    m_temperatureAxis->setSubSegmentCount(m_subSegments);
    m_temperatureAxis->setRange(m_minval, m_maxval);
    m_temperatureAxis->setLabelFormat(u"%.1f "_s + m_celsiusString);
    m_temperatureAxis->setLabelAutoAngle(30.0f);
    m_temperatureAxis->setTitleVisible(true);

    m_yearAxis->setTitle("Year");
    //! [auto angle]
    m_yearAxis->setLabelAutoAngle(30.0f);
    //! [auto angle]
    m_yearAxis->setTitleVisible(true);

    m_monthAxis->setTitle("Month");
    m_monthAxis->setLabelAutoAngle(30.0f);
    m_monthAxis->setTitleVisible(true);

    m_graph->setValueAxis(m_temperatureAxis);
    m_graph->setRowAxis(m_yearAxis);
    m_graph->setColumnAxis(m_monthAxis);
    //! [activate axes]

    //! [initialize series visuals]
    m_primarySeries->setItemLabelFormat(u"Oulu - @colLabel @rowLabel: @valueLabel"_s);
    m_primarySeries->setMesh(QAbstract3DSeries::Mesh::BevelBar);
    m_primarySeries->setMeshSmooth(false);

    m_secondarySeries->setItemLabelFormat(u"Helsinki - @colLabel @rowLabel: @valueLabel"_s);
    m_secondarySeries->setMesh(QAbstract3DSeries::Mesh::BevelBar);
    m_secondarySeries->setMeshSmooth(false);
    m_secondarySeries->setVisible(false);
    //! [initialize series visuals]

    //! [add series]
    m_graph->addSeries(m_primarySeries);
    m_graph->addSeries(m_secondarySeries);
    //! [add series]

    //! [invoke camera preset]
    changePresetCamera();
    //! [invoke camera preset]

    //! [invoke data reset]
    resetTemperatureData();
    //! [invoke data reset]

    // Set up property animations for zooming to the selected bar
    //! [animation setup]
    m_defaultAngleX = m_graph->cameraXRotation();
    m_defaultAngleY = m_graph->cameraYRotation();
    m_defaultZoom = m_graph->cameraZoomLevel();
    m_defaultTarget = m_graph->cameraTargetPosition();

    m_animationCameraX.setTargetObject(m_graph);
    m_animationCameraY.setTargetObject(m_graph);
    m_animationCameraZoom.setTargetObject(m_graph);
    m_animationCameraTarget.setTargetObject(m_graph);

    m_animationCameraX.setPropertyName("cameraXRotation");
    m_animationCameraY.setPropertyName("cameraYRotation");
    m_animationCameraZoom.setPropertyName("cameraZoomLevel");
    m_animationCameraTarget.setPropertyName("cameraTargetPosition");

    int duration = 1700;
    m_animationCameraX.setDuration(duration);
    m_animationCameraY.setDuration(duration);
    m_animationCameraZoom.setDuration(duration);
    m_animationCameraTarget.setDuration(duration);

    // The zoom always first zooms out above the graph and then zooms in
    qreal zoomOutFraction = 0.3;
    m_animationCameraX.setKeyValueAt(zoomOutFraction, QVariant::fromValue(0.0f));
    m_animationCameraY.setKeyValueAt(zoomOutFraction, QVariant::fromValue(90.0f));
    m_animationCameraZoom.setKeyValueAt(zoomOutFraction, QVariant::fromValue(50.0f));
    m_animationCameraTarget.setKeyValueAt(zoomOutFraction,
                                          QVariant::fromValue(QVector3D(0.0f, 0.0f, 0.0f)));
    //! [animation setup]

    m_customData = new RainfallData();
}

GraphModifier::~GraphModifier()
{
    delete m_customData;
}

void GraphModifier::resetTemperatureData()
{
    //! [set up data]
    // Set up data
    static const float tempOulu[8][12] = {
        {-7.4f, -2.4f, 0.0f, 3.0f, 8.2f, 11.6f, 14.7f, 15.4f, 11.4f, 4.2f, 2.1f, -2.3f},     // 2015
        {-13.4f, -3.9f, -1.8f, 3.1f, 10.6f, 13.7f, 17.8f, 13.6f, 10.7f, 3.5f, -3.1f, -4.2f}, // 2016
        //! [set up data]
        {-5.7f, -6.7f, -3.0f, -0.1f, 4.7f, 12.4f, 16.1f, 14.1f, 9.4f, 3.0f, -0.3f, -3.2f},  // 2017
        {-6.4f, -11.9f, -7.4f, 1.9f, 11.4f, 12.4f, 21.5f, 16.1f, 11.0f, 4.4f, 2.1f, -4.1f}, // 2018
        {-11.7f, -6.1f, -2.4f, 3.9f, 7.2f, 14.5f, 15.6f, 14.4f, 8.5f, 2.0f, -3.0f, -1.5f},  // 2019
        {-2.1f, -3.4f, -1.8f, 0.6f, 7.0f, 17.1f, 15.6f, 15.4f, 11.1f, 5.6f, 1.9f, -1.7f},   // 2020
        {-9.6f, -11.6f, -3.2f, 2.4f, 7.8f, 17.3f, 19.4f, 14.2f, 8.0f, 5.2f, -2.2f, -8.6f},  // 2021
        {-7.3f, -6.4f, -1.8f, 1.3f, 8.1f, 15.5f, 17.6f, 17.6f, 9.1f, 5.4f, -1.5f, -4.4f}    // 2022
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

    // Create data arrays
    //! [populate series vie proxies]
    QBarDataArray dataSet;
    QBarDataArray dataSet2;

    dataSet.reserve(m_years.size());
    for (qsizetype year = 0; year < m_years.size(); ++year) {
        // Create a data row
        QBarDataRow dataRow(m_months.size());
        QBarDataRow dataRow2(m_months.size());
        for (qsizetype month = 0; month < m_months.size(); ++month) {
            // Add data to the row
            dataRow[month].setValue(tempOulu[year][month]);
            dataRow2[month].setValue(tempHelsinki[year][month]);
        }
        // Add the row to the set
        dataSet.append(dataRow);
        dataSet2.append(dataRow2);
    }

    // Add data to the data proxy (the data proxy assumes ownership of it)
    m_primarySeries->dataProxy()->resetArray(dataSet, m_years, m_months);
    m_secondarySeries->dataProxy()->resetArray(dataSet2, m_years, m_months);
    //! [populate series vie proxies]
}

void GraphModifier::changeRange(int range)
{
    if (range >= m_years.count())
        m_yearAxis->setRange(0, m_years.count() - 1);
    else
        m_yearAxis->setRange(range, range);
}

void GraphModifier::changeStyle(int style)
{
    QComboBox *comboBox = qobject_cast<QComboBox *>(sender());
    if (comboBox) {
        m_barMesh = comboBox->itemData(style).value<QAbstract3DSeries::Mesh>();
        m_primarySeries->setMesh(m_barMesh);
        m_secondarySeries->setMesh(m_barMesh);
        m_customData->customSeries()->setMesh(m_barMesh);
        if (m_barMesh == QAbstract3DSeries::Mesh::UserDefined) {
            m_primarySeries->setUserDefinedMesh(":/data/narrowarrow.mesh");
            m_secondarySeries->setUserDefinedMesh(":/data/narrowarrow.mesh");
            m_customData->customSeries()->setUserDefinedMesh(":/data/narrowarrow.mesh");
        }
    }
}

void GraphModifier::changePresetCamera()
{
    m_animationCameraX.stop();
    m_animationCameraY.stop();
    m_animationCameraZoom.stop();
    m_animationCameraTarget.stop();

    // Restore camera target in case animation has changed it
    m_graph->setCameraTargetPosition(QVector3D(0.0f, 0.0f, 0.0f));

    //! [change the preset]
    static int preset = int(QtGraphs3D::CameraPreset::Front);

    m_graph->setCameraPreset((QtGraphs3D::CameraPreset) preset);

    if (++preset > int(QtGraphs3D::CameraPreset::DirectlyBelow))
        preset = int(QtGraphs3D::CameraPreset::FrontLow);
    //! [change the preset]
}

void GraphModifier::changeTheme(int theme)
{
    QGraphsTheme *currentTheme = m_graph->activeTheme();
    currentTheme->setTheme(QGraphsTheme::Theme(theme));
    emit backgroundVisibleChanged(currentTheme->isPlotAreaBackgroundVisible());
    emit gridVisibleChanged(currentTheme->isGridVisible());
    emit fontChanged(currentTheme->labelFont());
    emit fontSizeChanged(currentTheme->labelFont().pointSize());
}

void GraphModifier::changeLabelBackground()
{
    m_graph->activeTheme()->setLabelBackgroundVisible(
        !m_graph->activeTheme()->isLabelBackgroundVisible());
}

void GraphModifier::changeSelectionMode(int selectionMode)
{
    const auto *comboBox = qobject_cast<QComboBox *>(sender());
    if (comboBox) {
        int flags = comboBox->itemData(selectionMode).toInt();
        m_graph->setSelectionMode(QtGraphs3D::SelectionFlags(flags));
    }
}

void GraphModifier::changeFont(const QFont &font)
{
    m_graph->activeTheme()->setLabelFont(font);
}

void GraphModifier::changeFontSize(int fontsize)
{
    m_fontSize = fontsize;
    QFont font = m_graph->activeTheme()->labelFont();
    font.setPointSize(m_fontSize);
    m_graph->activeTheme()->setLabelFont(font);
}

void GraphModifier::shadowQualityUpdatedByVisual(QtGraphs3D::ShadowQuality sq)
{
    int quality = int(sq);
    // Updates the UI component to show correct shadow quality
    emit shadowQualityChanged(quality);
}

void GraphModifier::changeLabelRotation(int rotation)
{
    m_temperatureAxis->setLabelAutoAngle(float(rotation));
    m_monthAxis->setLabelAutoAngle(float(rotation));
    m_yearAxis->setLabelAutoAngle(float(rotation));
}

void GraphModifier::setAxisTitleVisibility(bool visible)
{
    m_temperatureAxis->setTitleVisible(visible);
    m_monthAxis->setTitleVisible(visible);
    m_yearAxis->setTitleVisible(visible);
}

void GraphModifier::setAxisTitleFixed(bool enabled)
{
    m_temperatureAxis->setTitleFixed(enabled);
    m_monthAxis->setTitleFixed(enabled);
    m_yearAxis->setTitleFixed(enabled);
}

void GraphModifier::zoomToSelectedBar()
{
    m_animationCameraX.stop();
    m_animationCameraY.stop();
    m_animationCameraZoom.stop();
    m_animationCameraTarget.stop();

    float currentX = m_graph->cameraXRotation();
    float currentY = m_graph->cameraYRotation();
    float currentZoom = m_graph->cameraZoomLevel();
    QVector3D currentTarget = m_graph->cameraTargetPosition();

    m_animationCameraX.setStartValue(QVariant::fromValue(currentX));
    m_animationCameraY.setStartValue(QVariant::fromValue(currentY));
    m_animationCameraZoom.setStartValue(QVariant::fromValue(currentZoom));
    m_animationCameraTarget.setStartValue(QVariant::fromValue(currentTarget));

    QPoint selectedBar = m_graph->selectedSeries() ? m_graph->selectedSeries()->selectedBar()
                                                   : QBar3DSeries::invalidSelectionPosition();

    if (selectedBar != QBar3DSeries::invalidSelectionPosition()) {
        // Normalize selected bar position within axis range to determine target coordinates
        //! [calculate target]
        QVector3D endTarget;
        float xMin = m_graph->columnAxis()->min();
        float xRange = m_graph->columnAxis()->max() - xMin;
        float zMin = m_graph->rowAxis()->min();
        float zRange = m_graph->rowAxis()->max() - zMin;
        endTarget.setX((selectedBar.y() - xMin) / xRange * 2.0f - 1.0f);
        endTarget.setZ((selectedBar.x() - zMin) / zRange * 2.0f - 1.0f);
        //! [calculate target]

        // Rotate the camera so that it always points approximately to the graph center
        //! [center camera]
        qreal endAngleX = 90.0 - qRadiansToDegrees(qAtan(qreal(endTarget.z() / endTarget.x())));
        if (endTarget.x() > 0.0f)
            endAngleX -= 180.0f;
        float barValue = m_graph->selectedSeries()
                             ->dataProxy()
                             ->itemAt(selectedBar.x(), selectedBar.y())
                             .value();
        float endAngleY = barValue >= 0.0f ? 30.0f : -30.0f;
        if (m_graph->valueAxis()->reversed())
            endAngleY *= -1.0f;
        //! [center camera]

        m_animationCameraX.setEndValue(QVariant::fromValue(float(endAngleX)));
        m_animationCameraY.setEndValue(QVariant::fromValue(endAngleY));
        m_animationCameraZoom.setEndValue(QVariant::fromValue(100));
        //! [set target]
        m_animationCameraTarget.setEndValue(QVariant::fromValue(endTarget));
        //! [set target]
    } else {
        // No selected bar, so return to the default view
        m_animationCameraX.setEndValue(QVariant::fromValue(m_defaultAngleX));
        m_animationCameraY.setEndValue(QVariant::fromValue(m_defaultAngleY));
        m_animationCameraZoom.setEndValue(QVariant::fromValue(m_defaultZoom));
        m_animationCameraTarget.setEndValue(QVariant::fromValue(m_defaultTarget));
    }

    m_animationCameraX.start();
    m_animationCameraY.start();
    m_animationCameraZoom.start();
    m_animationCameraTarget.start();
}

void GraphModifier::setDataModeToWeather(bool enabled)
{
    if (enabled)
        changeDataMode(false);
}

void GraphModifier::setDataModeToModel(bool enabled)
{
    if (enabled)
        changeDataMode(true);
}

void GraphModifier::changeShadowQuality(int quality)
{
    const auto sq = QtGraphs3D::ShadowQuality(quality);
    m_graph->setShadowQuality(sq);
    emit shadowQualityChanged(quality);
}

//! [rotate camera]
void GraphModifier::rotateX(int angle)
{
    m_xRotation = angle;
    m_graph->setCameraPosition(m_xRotation, m_yRotation);
}
//! [rotate camera]

void GraphModifier::rotateY(int angle)
{
    m_yRotation = angle;
    m_graph->setCameraPosition(m_xRotation, m_yRotation);
}

void GraphModifier::setBackgroundVisible(int visible)
{
    m_graph->activeTheme()->setPlotAreaBackgroundVisible(bool(visible));
}

void GraphModifier::setGridVisible(int visible)
{
    m_graph->activeTheme()->setGridVisible(bool(visible));
}

void GraphModifier::setSmoothBars(int smooth)
{
    m_smooth = bool(smooth);
    m_primarySeries->setMeshSmooth(m_smooth);
    m_secondarySeries->setMeshSmooth(m_smooth);
    m_customData->customSeries()->setMeshSmooth(m_smooth);
}

void GraphModifier::setSeriesVisibility(int visible)
{
    m_secondarySeries->setVisible(bool(visible));
}

void GraphModifier::setReverseValueAxis(int enabled)
{
    m_graph->valueAxis()->setReversed(enabled);
}

void GraphModifier::changeDataMode(bool modelData)
{
    int enabled = false;
    // Change between weather data and data from model proxy
    if (modelData) {
        m_graph->removeSeries(m_primarySeries);
        m_graph->removeSeries(m_secondarySeries);
        m_graph->addSeries(m_customData->customSeries());
        if (m_graph->valueAxis()->reversed())
            enabled = true;
        m_graph->setValueAxis(m_customData->valueAxis());
        m_graph->setRowAxis(m_customData->rowAxis());
        m_graph->setColumnAxis(m_customData->colAxis());
    } else {
        m_graph->removeSeries(m_customData->customSeries());
        m_graph->addSeries(m_primarySeries);
        m_graph->addSeries(m_secondarySeries);
        if (m_graph->valueAxis()->reversed())
            enabled = true;
        m_graph->setValueAxis(m_temperatureAxis);
        m_graph->setRowAxis(m_yearAxis);
        m_graph->setColumnAxis(m_monthAxis);
    }
    setReverseValueAxis(enabled);
}
