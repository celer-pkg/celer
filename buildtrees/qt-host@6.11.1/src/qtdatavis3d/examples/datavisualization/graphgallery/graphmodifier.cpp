// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "graphmodifier.h"
#include "rainfalldata.h"

#include <QtDataVisualization/qcategory3daxis.h>
#include <QtDataVisualization/qvalue3daxis.h>
#include <QtDataVisualization/qbardataproxy.h>
#include <QtDataVisualization/q3dscene.h>
#include <QtDataVisualization/q3dcamera.h>
#include <QtDataVisualization/qbar3dseries.h>
#include <QtDataVisualization/q3dtheme.h>
#include <QtCore/qmath.h>
#include <QtWidgets/qcombobox.h>

using namespace Qt::StringLiterals;

// TODO: Many of the values do not affect custom proxy series now - should be fixed

//! [0]
GraphModifier::GraphModifier(Q3DBars *bargraph, QObject *parent) :
      QObject(parent),
      m_graph(bargraph),
      //! [0]
      //! [1]
      m_temperatureAxis(new QValue3DAxis),
      m_yearAxis(new QCategory3DAxis),
      m_monthAxis(new QCategory3DAxis),
      m_primarySeries(new QBar3DSeries),
      m_secondarySeries(new QBar3DSeries),
      m_celsiusString(u"°C"_s)
    //! [1]
{
    //! [2]
    m_graph->setShadowQuality(QAbstract3DGraph::ShadowQualitySoftMedium);
    m_graph->activeTheme()->setBackgroundEnabled(false);
    m_graph->activeTheme()->setFont(QFont("Times New Roman", m_fontSize));
    m_graph->activeTheme()->setLabelBackgroundEnabled(true);
    m_graph->setMultiSeriesUniform(true);
    //! [2]

    m_months = {"January", "February", "March", "April", "May", "June" , "July",
                "August", "September", "October" , "November", "December"};
    m_years = {"2015", "2016", "2017", "2018", "2019", "2020", "2021", "2022"};

    //! [3]
    m_temperatureAxis->setTitle("Average temperature");
    m_temperatureAxis->setSegmentCount(m_segments);
    m_temperatureAxis->setSubSegmentCount(m_subSegments);
    m_temperatureAxis->setRange(m_minval, m_maxval);
    m_temperatureAxis->setLabelFormat(u"%.1f "_s + m_celsiusString);
    m_temperatureAxis->setLabelAutoRotation(30.0f);
    m_temperatureAxis->setTitleVisible(true);

    m_yearAxis->setTitle("Year");
    m_yearAxis->setLabelAutoRotation(30.0f);
    m_yearAxis->setTitleVisible(true);
    m_monthAxis->setTitle("Month");
    m_monthAxis->setLabelAutoRotation(30.0f);
    m_monthAxis->setTitleVisible(true);

    m_graph->setValueAxis(m_temperatureAxis);
    m_graph->setRowAxis(m_yearAxis);
    m_graph->setColumnAxis(m_monthAxis);
    //! [3]

    //! [4]
    m_primarySeries->setItemLabelFormat(u"Oulu - @colLabel @rowLabel: @valueLabel"_s);
    m_primarySeries->setMesh(QAbstract3DSeries::MeshBevelBar);
    m_primarySeries->setMeshSmooth(false);

    m_secondarySeries->setItemLabelFormat(u"Helsinki - @colLabel @rowLabel: @valueLabel"_s);
    m_secondarySeries->setMesh(QAbstract3DSeries::MeshBevelBar);
    m_secondarySeries->setMeshSmooth(false);
    m_secondarySeries->setVisible(false);
    //! [4]

    //! [5]
    m_graph->addSeries(m_primarySeries);
    m_graph->addSeries(m_secondarySeries);
    //! [5]

    //! [6]
    changePresetCamera();
    //! [6]

    //! [8]
    resetTemperatureData();
    //! [8]

    // Set up property animations for zooming to the selected bar
    //! [11]
    Q3DCamera *camera = m_graph->scene()->activeCamera();
    m_defaultAngleX = camera->xRotation();
    m_defaultAngleY = camera->yRotation();
    m_defaultZoom = camera->zoomLevel();
    m_defaultTarget = camera->target();

    m_animationCameraX.setTargetObject(camera);
    m_animationCameraY.setTargetObject(camera);
    m_animationCameraZoom.setTargetObject(camera);
    m_animationCameraTarget.setTargetObject(camera);

    m_animationCameraX.setPropertyName("xRotation");
    m_animationCameraY.setPropertyName("yRotation");
    m_animationCameraZoom.setPropertyName("zoomLevel");
    m_animationCameraTarget.setPropertyName("target");

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
    //! [11]

    m_customData = new RainfallData();
}

GraphModifier::~GraphModifier()
{
    delete m_customData;
}

void GraphModifier::resetTemperatureData()
{
    //! [9a]
    // Set up data
    static const float tempOulu[8][12] = {
        {-7.4f, -2.4f, 0.0f, 3.0f, 8.2f, 11.6f, 14.7f, 15.4f, 11.4f, 4.2f, 2.1f, -2.3f},       // 2015
        {-13.4f, -3.9f, -1.8f, 3.1f, 10.6f, 13.7f, 17.8f, 13.6f, 10.7f, 3.5f, -3.1f, -4.2f},   // 2016
        //! [9a]
        {-5.7f, -6.7f, -3.0f, -0.1f, 4.7f, 12.4f, 16.1f, 14.1f, 9.4f, 3.0f, -0.3f, -3.2f},     // 2017
        {-6.4f, -11.9f, -7.4f, 1.9f, 11.4f, 12.4f, 21.5f, 16.1f, 11.0f, 4.4f, 2.1f, -4.1f},    // 2018
        {-11.7f, -6.1f, -2.4f, 3.9f, 7.2f, 14.5f, 15.6f, 14.4f, 8.5f, 2.0f, -3.0f, -1.5f},     // 2019
        {-2.1f, -3.4f, -1.8f, 0.6f, 7.0f, 17.1f, 15.6f, 15.4f, 11.1f, 5.6f, 1.9f, -1.7f},      // 2020
        {-9.6f, -11.6f, -3.2f, 2.4f, 7.8f, 17.3f, 19.4f, 14.2f, 8.0f, 5.2f, -2.2f, -8.6f},     // 2021
        {-7.3f, -6.4f, -1.8f, 1.3f, 8.1f, 15.5f, 17.6f, 17.6f, 9.1f, 5.4f, -1.5f, -4.4f}       // 2022
    };

    static const float tempHelsinki[8][12] = {
        {-2.0f, -0.1f, 1.8f, 5.1f, 9.7f, 13.7f, 16.3f, 17.3f, 12.7f, 5.4f, 4.6f, 2.1f},        // 2015
        {-10.3f, -0.6f, 0.0f, 4.9f, 14.3f, 15.7f, 17.7f, 16.0f, 12.7f, 4.6f, -1.0f, -0.9f},    // 2016
        {-2.9f, -3.3f, 0.7f, 2.3f, 9.9f, 13.8f, 16.1f, 15.9f, 11.4f, 5.0f, 2.7f, 0.7f},        // 2017
        {-2.2f, -8.4f, -4.7f, 5.0f, 15.3f, 15.8f, 21.2f, 18.2f, 13.3f, 6.7f, 2.8f, -2.0f},     // 2018
        {-6.2f, -0.5f, -0.3f, 6.8f, 10.6f, 17.9f, 17.5f, 16.8f, 11.3f, 5.2f, 1.8f, 1.4f},      // 2019
        {1.9f, 0.5f, 1.7f, 4.5f, 9.5f, 18.4f, 16.5f, 16.8f, 13.0f, 8.2f, 4.4f, 0.9f},          // 2020
        {-4.7f, -8.1f, -0.9f, 4.5f, 10.4f, 19.2f, 20.9f, 15.4f, 9.5f, 8.0f, 1.5f, -6.7f},      // 2021
        {-3.3f, -2.2f, -0.2f, 3.3f, 9.6f, 16.9f, 18.1f, 18.9f, 9.2f, 7.6f, 2.3f, -3.4f}        // 2022
    };

    // Create data arrays
    //! [9b]
    auto *dataSet = new QBarDataArray;
    auto *dataSet2 = new QBarDataArray;

    dataSet->reserve(m_years.size());
    for (qsizetype year = 0; year < m_years.size(); ++year) {
        // Create a data row
        auto *dataRow = new QBarDataRow(m_months.size());
        auto *dataRow2 = new QBarDataRow(m_months.size());
        for (qsizetype month = 0; month < m_months.size(); ++month) {
            // Add data to the row
            (*dataRow)[month].setValue(tempOulu[year][month]);
            (*dataRow2)[month].setValue(tempHelsinki[year][month]);
        }
        // Add the row to the set
        dataSet->append(dataRow);
        dataSet2->append(dataRow2);
    }

    // Add data to the data proxy (the data proxy assumes ownership of it)
    m_primarySeries->dataProxy()->resetArray(dataSet, m_years, m_months);
    m_secondarySeries->dataProxy()->resetArray(dataSet2, m_years, m_months);
    //! [9b]
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
    auto *comboBox = qobject_cast<QComboBox *>(sender());
    if (comboBox) {
        m_barMesh = comboBox->itemData(style).value<QAbstract3DSeries::Mesh>();
        m_primarySeries->setMesh(m_barMesh);
        m_secondarySeries->setMesh(m_barMesh);
        m_customData->customSeries()->setMesh(m_barMesh);
    }
}

void GraphModifier::changePresetCamera()
{
    m_animationCameraX.stop();
    m_animationCameraY.stop();
    m_animationCameraZoom.stop();
    m_animationCameraTarget.stop();

    // Restore camera target in case animation has changed it
    m_graph->scene()->activeCamera()->setTarget(QVector3D(0.0f, 0.0f, 0.0f));

    //! [7]
    static int preset = Q3DCamera::CameraPresetFront;

    m_graph->scene()->activeCamera()->setCameraPreset((Q3DCamera::CameraPreset)preset);

    if (++preset > Q3DCamera::CameraPresetDirectlyBelow)
        preset = Q3DCamera::CameraPresetFrontLow;
    //! [7]
}

void GraphModifier::changeTheme(int theme)
{
    Q3DTheme *currentTheme = m_graph->activeTheme();
    currentTheme->setType(Q3DTheme::Theme(theme));
    emit backgroundEnabledChanged(currentTheme->isBackgroundEnabled());
    emit gridEnabledChanged(currentTheme->isGridEnabled());
    emit fontChanged(currentTheme->font());
    emit fontSizeChanged(currentTheme->font().pointSize());
}

void GraphModifier::changeLabelBackground()
{
    m_graph->activeTheme()->setLabelBackgroundEnabled(!m_graph->activeTheme()->isLabelBackgroundEnabled());
}

void GraphModifier::changeSelectionMode(int selectionMode)
{
    auto *comboBox = qobject_cast<QComboBox *>(sender());
    if (comboBox) {
        int flags = comboBox->itemData(selectionMode).toInt();
        m_graph->setSelectionMode(QAbstract3DGraph::SelectionFlags(flags));
    }
}

void GraphModifier::changeFont(const QFont &font)
{
    QFont newFont = font;
    m_graph->activeTheme()->setFont(newFont);
}

void GraphModifier::changeFontSize(int fontsize)
{
    m_fontSize = fontsize;
    QFont font = m_graph->activeTheme()->font();
    font.setPointSize(m_fontSize);
    m_graph->activeTheme()->setFont(font);
}

void GraphModifier::shadowQualityUpdatedByVisual(QAbstract3DGraph::ShadowQuality sq)
{
    int quality = int(sq);
    // Updates the UI component to show correct shadow quality
    emit shadowQualityChanged(quality);
}

void GraphModifier::changeLabelRotation(int rotation)
{
    m_temperatureAxis->setLabelAutoRotation(float(rotation));
    m_monthAxis->setLabelAutoRotation(float(rotation));
    m_yearAxis->setLabelAutoRotation(float(rotation));
}

void GraphModifier::setAxisTitleVisibility(bool enabled)
{
    m_temperatureAxis->setTitleVisible(enabled);
    m_monthAxis->setTitleVisible(enabled);
    m_yearAxis->setTitleVisible(enabled);
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

    Q3DCamera *camera = m_graph->scene()->activeCamera();
    float currentX = camera->xRotation();
    float currentY = camera->yRotation();
    float currentZoom = camera->zoomLevel();
    QVector3D currentTarget = camera->target();

    m_animationCameraX.setStartValue(QVariant::fromValue(currentX));
    m_animationCameraY.setStartValue(QVariant::fromValue(currentY));
    m_animationCameraZoom.setStartValue(QVariant::fromValue(currentZoom));
    m_animationCameraTarget.setStartValue(QVariant::fromValue(currentTarget));

    QPoint selectedBar = m_graph->selectedSeries()
            ? m_graph->selectedSeries()->selectedBar()
            : QBar3DSeries::invalidSelectionPosition();

    if (selectedBar != QBar3DSeries::invalidSelectionPosition()) {
        // Normalize selected bar position within axis range to determine target coordinates
        //! [12]
        QVector3D endTarget;
        float xMin = m_graph->columnAxis()->min();
        float xRange = m_graph->columnAxis()->max() - xMin;
        float zMin = m_graph->rowAxis()->min();
        float zRange = m_graph->rowAxis()->max() - zMin;
        endTarget.setX((selectedBar.y() - xMin) / xRange * 2.0f - 1.0f);
        endTarget.setZ((selectedBar.x() - zMin) / zRange * 2.0f - 1.0f);
        //! [12]

        // Rotate the camera so that it always points approximately to the graph center
        //! [14]
        qreal endAngleX = 90.0 - qRadiansToDegrees(qAtan(qreal(endTarget.z() / endTarget.x())));
        if (endTarget.x() > 0.0f)
            endAngleX -= 180.0f;
        float barValue = m_graph->selectedSeries()->dataProxy()->itemAt(selectedBar.x(),
                                                                        selectedBar.y())->value();
        float endAngleY = barValue >= 0.0f ? 30.0f : -30.0f;
        if (m_graph->valueAxis()->reversed())
            endAngleY *= -1.0f;
        //! [14]

        m_animationCameraX.setEndValue(QVariant::fromValue(float(endAngleX)));
        m_animationCameraY.setEndValue(QVariant::fromValue(endAngleY));
        m_animationCameraZoom.setEndValue(QVariant::fromValue(250));
        //! [13]
        m_animationCameraTarget.setEndValue(QVariant::fromValue(endTarget));
        //! [13]
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

void GraphModifier::setDataModeToCustom(bool enabled)
{
    if (enabled)
        changeDataMode(true);
}

void GraphModifier::changeShadowQuality(int quality)
{
    QAbstract3DGraph::ShadowQuality sq = QAbstract3DGraph::ShadowQuality(quality);
    m_graph->setShadowQuality(sq);
    emit shadowQualityChanged(quality);
}

//! [10]
void GraphModifier::rotateX(int rotation)
{
    m_xRotation = rotation;
    m_graph->scene()->activeCamera()->setCameraPosition(m_xRotation, m_yRotation);
}
//! [10]

void GraphModifier::rotateY(int rotation)
{
    m_yRotation = rotation;
    m_graph->scene()->activeCamera()->setCameraPosition(m_xRotation, m_yRotation);
}

void GraphModifier::setBackgroundEnabled(int enabled)
{
    m_graph->activeTheme()->setBackgroundEnabled(bool(enabled));
}

void GraphModifier::setGridEnabled(int enabled)
{
    m_graph->activeTheme()->setGridEnabled(bool(enabled));
}

void GraphModifier::setSmoothBars(int smooth)
{
    m_smooth = bool(smooth);
    m_primarySeries->setMeshSmooth(m_smooth);
    m_secondarySeries->setMeshSmooth(m_smooth);
    m_customData->customSeries()->setMeshSmooth(m_smooth);
}

void GraphModifier::setSeriesVisibility(int enabled)
{
    m_secondarySeries->setVisible(bool(enabled));
}

void GraphModifier::setReverseValueAxis(int enabled)
{
    m_graph->valueAxis()->setReversed(enabled);
}

void GraphModifier::setReflection(bool enabled)
{
    m_graph->setReflection(enabled);
}

void GraphModifier::changeDataMode(bool customData)
{
    // Change between weather data and data from custom proxy
    if (customData) {
        m_graph->removeSeries(m_primarySeries);
        m_graph->removeSeries(m_secondarySeries);
        m_graph->addSeries(m_customData->customSeries());
        m_graph->setValueAxis(m_customData->valueAxis());
        m_graph->setRowAxis(m_customData->rowAxis());
        m_graph->setColumnAxis(m_customData->colAxis());
    } else {
        m_graph->removeSeries(m_customData->customSeries());
        m_graph->addSeries(m_primarySeries);
        m_graph->addSeries(m_secondarySeries);
        m_graph->setValueAxis(m_temperatureAxis);
        m_graph->setRowAxis(m_yearAxis);
        m_graph->setColumnAxis(m_monthAxis);
    }
}
