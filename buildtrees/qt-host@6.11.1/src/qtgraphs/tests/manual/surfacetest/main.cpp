// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "graphmodifier.h"
#include "buttonwrapper.h"
#include "checkboxwrapper.h"
#include <QtGraphs/qgraphstheme.h>

#include <QApplication>
#include <QWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QCheckBox>
#include <QRadioButton>
#include <QSlider>
#include <QLabel>
#include <QScreen>
#include <QPainter>
#include <QFontComboBox>
#include <QFrame>
#include <QDebug>
#include <qcheckbox.h>
#include <qobject.h>

const int initialTheme = 4;

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QWidget *widget = new QWidget;
    QHBoxLayout *hLayout = new QHBoxLayout(widget);
    QVBoxLayout *vLayout = new QVBoxLayout();
    QVBoxLayout *vLayout2 = new QVBoxLayout();
    QVBoxLayout *vLayout3 = new QVBoxLayout();
    vLayout->setAlignment(Qt::AlignTop);
    vLayout2->setAlignment(Qt::AlignTop);
    vLayout3->setAlignment(Qt::AlignTop);

    auto quickWidget = new QQuickWidget;
    Q3DSurfaceWidgetItem *surfaceGraph = new Q3DSurfaceWidgetItem();
    surfaceGraph->setWidget(quickWidget);
    QSize screenSize = surfaceGraph->widget()->screen()->size();

    // Set to default, should be same as the initial on themeList
    surfaceGraph->activeTheme()->setTheme(QGraphsTheme::Theme(initialTheme));

    surfaceGraph->widget()->setMinimumSize(QSize(screenSize.width() / 2, screenSize.height() / 4));
    surfaceGraph->widget()->setMaximumSize(screenSize);
    surfaceGraph->widget()->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    surfaceGraph->widget()->setFocusPolicy(Qt::StrongFocus);
    surfaceGraph->widget()->setResizeMode(QQuickWidget::SizeRootObjectToView);

    widget->setWindowTitle(QStringLiteral("Surface tester"));

    hLayout->addWidget(surfaceGraph->widget(), 1);
    hLayout->addLayout(vLayout);
    hLayout->addLayout(vLayout2);
    hLayout->addLayout(vLayout3);

    QCheckBox *smoothCB = new QCheckBox(widget);
    smoothCB->setText(QStringLiteral("Flat Surface"));
    smoothCB->setChecked(true);

    QCheckBox *surfaceGridCB = new QCheckBox(widget);
    surfaceGridCB->setText(QStringLiteral("Surface Grid"));
    surfaceGridCB->setChecked(true);

    QCheckBox *surfaceCB = new QCheckBox(widget);
    surfaceCB->setText(QStringLiteral("Surface Visible"));
    surfaceCB->setChecked(true);

    QCheckBox *seriesVisibleCB = new QCheckBox(widget);
    seriesVisibleCB->setText(QStringLiteral("Series Visible"));
    seriesVisibleCB->setChecked(true);

    QCheckBox *fillCB = new QCheckBox(widget);
    fillCB->setText(QStringLiteral("Fill Surface"));
    fillCB->setChecked(false);

#ifdef MULTI_SERIES
    smoothCB->setText(QStringLiteral("S1 Flat Surface"));
    surfaceGridCB->setText(QStringLiteral("S1 Surface Grid"));
    surfaceCB->setText(QStringLiteral("S1 Surface Visible"));
    seriesVisibleCB->setText(QStringLiteral("Series 1 Visible"));
    fillCB->setText(QStringLiteral("Series 1 Fill"));

    QCheckBox *smoothS2CB = new QCheckBox(widget);
    smoothS2CB->setText(QStringLiteral("S2 Flat Surface"));
    smoothS2CB->setChecked(true);

    QCheckBox *surfaceGridS2CB = new QCheckBox(widget);
    surfaceGridS2CB->setText(QStringLiteral("S2 Surface Grid"));
    surfaceGridS2CB->setChecked(true);

    QCheckBox *surfaceS2CB = new QCheckBox(widget);
    surfaceS2CB->setText(QStringLiteral("S2 Surface Visible"));
    surfaceS2CB->setChecked(true);

    QCheckBox *series2VisibleCB = new QCheckBox(widget);
    series2VisibleCB->setText(QStringLiteral("Series 2 Visible"));
    series2VisibleCB->setChecked(true);

    QCheckBox *fillS2CB = new QCheckBox(widget);
    fillS2CB->setText(QStringLiteral("Series 2 Fill"));
    fillS2CB->setChecked(false);

    QCheckBox *smoothS3CB = new QCheckBox(widget);
    smoothS3CB->setText(QStringLiteral("S3 Flat Surface"));
    smoothS3CB->setChecked(true);

    QCheckBox *surfaceGridS3CB = new QCheckBox(widget);
    surfaceGridS3CB->setText(QStringLiteral("S3 Surface Grid"));
    surfaceGridS3CB->setChecked(true);

    QCheckBox *surfaceS3CB = new QCheckBox(widget);
    surfaceS3CB->setText(QStringLiteral("S3 Surface Visible"));
    surfaceS3CB->setChecked(true);

    QCheckBox *series3VisibleCB = new QCheckBox(widget);
    series3VisibleCB->setText(QStringLiteral("Series 3 Visible"));
    series3VisibleCB->setChecked(true);

    QCheckBox *fillS3CB = new QCheckBox(widget);
    fillS3CB->setText(QStringLiteral("Series 3 Fill"));
    fillS3CB->setChecked(false);

    QCheckBox *smoothS4CB = new QCheckBox(widget);
    smoothS4CB->setText(QStringLiteral("S4 Flat Surface"));
    smoothS4CB->setChecked(true);

    QCheckBox *surfaceGridS4CB = new QCheckBox(widget);
    surfaceGridS4CB->setText(QStringLiteral("S4 Surface Grid"));
    surfaceGridS4CB->setChecked(true);

    QCheckBox *surfaceS4CB = new QCheckBox(widget);
    surfaceS4CB->setText(QStringLiteral("S4 Surface Visible"));
    surfaceS4CB->setChecked(true);

    QCheckBox *series4VisibleCB = new QCheckBox(widget);
    series4VisibleCB->setText(QStringLiteral("Series 4 Visible"));
    series4VisibleCB->setChecked(true);

    QCheckBox *fillS4CB = new QCheckBox(widget);
    fillS4CB->setText(QStringLiteral("Series 4 Fill"));
    fillS4CB->setChecked(false);

    QCheckBox *series1CB = new QCheckBox(widget);
    series1CB->setText(QStringLiteral("Series 1"));

    QCheckBox *series2CB = new QCheckBox(widget);
    series2CB->setText(QStringLiteral("Series 2"));

    QCheckBox *series3CB = new QCheckBox(widget);
    series3CB->setText(QStringLiteral("Series 3"));

    QCheckBox *series4CB = new QCheckBox(widget);
    series4CB->setText(QStringLiteral("Series 4"));

    QCheckBox *lineSeriesCB = new QCheckBox(widget);
    lineSeriesCB->setText(QStringLiteral("Line Series"));

    QCheckBox *surfaceGridLineCB = new QCheckBox(widget);
    surfaceGridLineCB->setText(QStringLiteral("Line Surface Grid"));
    surfaceGridLineCB->setChecked(false);

    QCheckBox *lineSeriesVisibleCB = new QCheckBox(widget);
    lineSeriesVisibleCB->setText(QStringLiteral("Line Series Visible"));
    lineSeriesVisibleCB->setChecked(true);

    QCheckBox *fillLineCB = new QCheckBox(widget);
    fillLineCB->setText(QStringLiteral("Line Series Fill"));
    fillLineCB->setChecked(false);

#else
    //QCheckBox *sqrtSinCB = new QCheckBox(widget);
    QRadioButton *sqrtSinCB = new QRadioButton(widget);
    sqrtSinCB->setText(QStringLiteral("Sqrt & Sin"));
    sqrtSinCB->setChecked(false);

    QRadioButton *planeCB = new QRadioButton(widget);
    planeCB->setText(QStringLiteral("Plane"));
    planeCB->setChecked(false);

    QRadioButton *heightMapCB = new QRadioButton(widget);
    heightMapCB->setText(QStringLiteral("Height map"));
    heightMapCB->setChecked(false);

    QCheckBox *gridSlidersLockCB = new QCheckBox(widget);
    gridSlidersLockCB->setText(QStringLiteral("Lock"));
    gridSlidersLockCB->setChecked(false);

    QSlider *gridSliderX = new QSlider(Qt::Horizontal, widget);
    gridSliderX->setTickInterval(1);
    gridSliderX->setMinimum(2);
    gridSliderX->setValue(30);
    gridSliderX->setMaximum(200);
    gridSliderX->setEnabled(true);
    QSlider *gridSliderZ = new QSlider(Qt::Horizontal, widget);
    gridSliderZ->setTickInterval(1);
    gridSliderZ->setMinimum(2);
    gridSliderZ->setValue(30);
    gridSliderZ->setMaximum(200);
    gridSliderZ->setEnabled(true);
#endif

    QSlider *axisRangeSliderX = new QSlider(Qt::Horizontal, widget);
    axisRangeSliderX->setTickInterval(1);
    axisRangeSliderX->setMinimum(1);
    axisRangeSliderX->setValue(34);
    axisRangeSliderX->setMaximum(100);
    axisRangeSliderX->setEnabled(true);
    QSlider *axisRangeSliderY = new QSlider(Qt::Horizontal, widget);
    axisRangeSliderY->setTickInterval(1);
    axisRangeSliderY->setMinimum(1);
    axisRangeSliderY->setValue(20);
    axisRangeSliderY->setMaximum(100);
    axisRangeSliderY->setEnabled(true);
    QSlider *axisRangeSliderZ = new QSlider(Qt::Horizontal, widget);
    axisRangeSliderZ->setTickInterval(1);
    axisRangeSliderZ->setMinimum(1);
    axisRangeSliderZ->setValue(34);
    axisRangeSliderZ->setMaximum(100);
    axisRangeSliderZ->setEnabled(true);

    QSlider *axisMinSliderX = new QSlider(Qt::Horizontal, widget);
    axisMinSliderX->setTickInterval(1);
    axisMinSliderX->setMinimum(-100);
    axisMinSliderX->setValue(-17);
    axisMinSliderX->setMaximum(100);
    axisMinSliderX->setEnabled(true);
    QSlider *axisMinSliderY = new QSlider(Qt::Horizontal, widget);
    axisMinSliderY->setTickInterval(1);
    axisMinSliderY->setMinimum(-100);
    axisMinSliderY->setValue(-8);
    axisMinSliderY->setMaximum(100);
    axisMinSliderY->setEnabled(true);
    QSlider *axisMinSliderZ = new QSlider(Qt::Horizontal, widget);
    axisMinSliderZ->setTickInterval(1);
    axisMinSliderZ->setMinimum(-100);
    axisMinSliderZ->setValue(-17);
    axisMinSliderZ->setMaximum(100);
    axisMinSliderZ->setEnabled(true);

    QSlider *cutoffMarginSlider = new QSlider(Qt::Horizontal, widget);
    cutoffMarginSlider->setTickInterval(500);
    cutoffMarginSlider->setMinimum(-1000);
    cutoffMarginSlider->setValue(0);
    cutoffMarginSlider->setMaximum(1000);
    cutoffMarginSlider->setEnabled(true);

    QSlider *aspectRatioSlider = new QSlider(Qt::Horizontal, widget);
    aspectRatioSlider->setMinimum(1);
    aspectRatioSlider->setValue(20);
    aspectRatioSlider->setMaximum(100);

    QSlider *horizontalAspectRatioSlider = new QSlider(Qt::Horizontal, widget);
    horizontalAspectRatioSlider->setMinimum(0);
    horizontalAspectRatioSlider->setValue(0);
    horizontalAspectRatioSlider->setMaximum(300);

    QLinearGradient gr(0, 0, 100, 1);
    gr.setColorAt(0.0, Qt::black);
    gr.setColorAt(0.33, Qt::blue);
    gr.setColorAt(0.67, Qt::red);
    gr.setColorAt(1.0, Qt::yellow);
    QPixmap pm(100, 24);
    QPainter pmp(&pm);
    pmp.setBrush(QBrush(gr));
    pmp.setPen(Qt::NoPen);
    pmp.drawRect(0, 0, 100, 24);
    QPushButton *colorPB = new QPushButton();
    colorPB->setIcon(QIcon(pm));
    colorPB->setIconSize(QSize(100, 24));

    QFontComboBox *fontList = new QFontComboBox(widget);
    fontList->setCurrentFont(QFont("Arial"));

    QPushButton *labelButton = new QPushButton(widget);
    labelButton->setText(QStringLiteral("Change label style"));

    QPushButton *meshButton = new QPushButton(widget);
    meshButton->setText(QStringLiteral("Change pointer mesh"));

    QComboBox *colorSchemeList = new QComboBox(widget);
    colorSchemeList->addItem(QStringLiteral("Light"));
    colorSchemeList->addItem(QStringLiteral("Dark"));
    colorSchemeList->setCurrentIndex(0);

    QComboBox *themeList = new QComboBox(widget);
    themeList->addItem(QStringLiteral("QtGreen"));
    themeList->addItem(QStringLiteral("QtGreenNeon"));
    themeList->addItem(QStringLiteral("MixSeries"));
    themeList->addItem(QStringLiteral("OrangeSeries"));
    themeList->addItem(QStringLiteral("YellowSeries"));
    themeList->addItem(QStringLiteral("BlueSeries"));
    themeList->addItem(QStringLiteral("PurpleSeries"));
    themeList->addItem(QStringLiteral("GreySeries"));
    themeList->setCurrentIndex(initialTheme);

    QComboBox *shadowQuality = new QComboBox(widget);
    shadowQuality->addItem(QStringLiteral("None"));
    shadowQuality->addItem(QStringLiteral("Low"));
    shadowQuality->addItem(QStringLiteral("Medium"));
    shadowQuality->addItem(QStringLiteral("High"));
    shadowQuality->addItem(QStringLiteral("Low Soft"));
    shadowQuality->addItem(QStringLiteral("Medium Soft"));
    shadowQuality->addItem(QStringLiteral("High Soft"));
    shadowQuality->setCurrentIndex(3);

    QComboBox *transparencyTechnique = new QComboBox(widget);
    transparencyTechnique->addItem(QStringLiteral("Default"));
    transparencyTechnique->addItem(QStringLiteral("Approximate"));
    transparencyTechnique->addItem(QStringLiteral("Accurate"));
    transparencyTechnique->setCurrentIndex(0);

    QComboBox *selectionMode = new QComboBox(widget);
    selectionMode->addItem(QStringLiteral("None"), int(QtGraphs3D::SelectionFlag::None));
    selectionMode->addItem(QStringLiteral("Item"), int(QtGraphs3D::SelectionFlag::Item));
    selectionMode->addItem(QStringLiteral("Multi: Item"),
                           int(QtGraphs3D::SelectionFlag::Item
                               | QtGraphs3D::SelectionFlag::MultiSeries));
    selectionMode->addItem(QStringLiteral("Row"), int(QtGraphs3D::SelectionFlag::Row));
    selectionMode->addItem(QStringLiteral("Item and Row"),
                           int(QtGraphs3D::SelectionFlag::ItemAndRow));
    selectionMode->addItem(QStringLiteral("Column"), int(QtGraphs3D::SelectionFlag::Column));
    selectionMode->addItem(QStringLiteral("Item and Column"),
                           int(QtGraphs3D::SelectionFlag::ItemAndColumn));
    selectionMode->addItem(QStringLiteral("Row and Column"),
                           int(QtGraphs3D::SelectionFlag::RowAndColumn));
    selectionMode->addItem(QStringLiteral("Item, Row and Column"),
                           int(QtGraphs3D::SelectionFlag::ItemRowAndColumn));
    selectionMode->addItem(QStringLiteral("Multi: Item, Row and Column"),
                           int(QtGraphs3D::SelectionFlag::ItemRowAndColumn
                               | QtGraphs3D::SelectionFlag::MultiSeries));
    selectionMode->addItem(QStringLiteral("Slice into Row"),
                           int(QtGraphs3D::SelectionFlag::Slice | QtGraphs3D::SelectionFlag::Row));
    selectionMode->addItem(QStringLiteral("Slice into Row and Item"),
                           int(QtGraphs3D::SelectionFlag::Slice
                               | QtGraphs3D::SelectionFlag::ItemAndRow));
    selectionMode->addItem(QStringLiteral("Multi: Slice, Row & Item"),
                           int(QtGraphs3D::SelectionFlag::Slice
                               | QtGraphs3D::SelectionFlag::ItemAndRow
                               | QtGraphs3D::SelectionFlag::MultiSeries));
    selectionMode->addItem(QStringLiteral("Slice into Column"),
                           int(QtGraphs3D::SelectionFlag::Slice
                               | QtGraphs3D::SelectionFlag::Column));
    selectionMode->addItem(QStringLiteral("Slice into Column and Item"),
                           int(QtGraphs3D::SelectionFlag::Slice
                               | QtGraphs3D::SelectionFlag::ItemAndColumn));
    selectionMode->addItem(QStringLiteral("Multi: Slice, Column & Item"),
                           int(QtGraphs3D::SelectionFlag::Slice
                               | QtGraphs3D::SelectionFlag::ItemAndColumn
                               | QtGraphs3D::SelectionFlag::MultiSeries));

#ifndef MULTI_SERIES
    QPushButton *selectButton = new QPushButton(widget);
    selectButton->setText(QStringLiteral("Select random point"));

    QPushButton *flipViewsButton = new QPushButton(widget);
    flipViewsButton->setText(QStringLiteral("Flip Views"));

    QLabel *selectionInfoLabel = new QLabel(widget);
#endif

    QPushButton *changeSubViews = new QPushButton(widget);
    changeSubViews->setText(QStringLiteral("Change subview"));
    QPushButton *changeRowButton = new QPushButton(widget);
    changeRowButton->setText(QStringLiteral("Change a row"));

    QPushButton *changeRowsButton = new QPushButton(widget);
    changeRowsButton->setText(QStringLiteral("Change 3 rows"));

    QPushButton *changeItemButton = new QPushButton(widget);
    changeItemButton->setText(QStringLiteral("Change item"));

    QPushButton *changeMultipleItemButton = new QPushButton(widget);
    changeMultipleItemButton->setText(QStringLiteral("Change many items"));

    QPushButton *changeMultipleRowsButton = new QPushButton(widget);
    changeMultipleRowsButton->setText(QStringLiteral("Change many rows"));

    QPushButton *addRowButton = new QPushButton(widget);
    addRowButton->setText(QStringLiteral("Add a row"));

    QPushButton *addRowsButton = new QPushButton(widget);
    addRowsButton->setText(QStringLiteral("Add 3 rows"));

    QPushButton *insertRowButton = new QPushButton(widget);
    insertRowButton->setText(QStringLiteral("Insert a row"));

    QPushButton *insertRowsButton = new QPushButton(widget);
    insertRowsButton->setText(QStringLiteral("Insert 3 rows"));

    QPushButton *removeRowButton = new QPushButton(widget);
    removeRowButton->setText(QStringLiteral("Remove a row"));

    QPushButton *resetArrayButton = new QPushButton(widget);
    resetArrayButton->setText(QStringLiteral("Reset Series Array to plane"));

    QPushButton *resetArrayEmptyButton = new QPushButton(widget);
    resetArrayEmptyButton->setText(QStringLiteral("Reset Series Array to empty"));

    QPushButton *massiveDataTestButton = new QPushButton(widget);
    massiveDataTestButton->setText(QStringLiteral("Massive data test"));

    QPushButton *testReverseButton = new QPushButton(widget);
    testReverseButton->setText(QStringLiteral("Test Axis Reversing"));

    QPushButton *testDataOrderingButton = new QPushButton(widget);
    testDataOrderingButton->setText(QStringLiteral("Test data ordering"));

    QPushButton *testNanSeriesButton = new QPushButton(widget);
    testNanSeriesButton->setText(QStringLiteral("Test NaN series"));

    QCheckBox *sanitizeRowsCB = new QCheckBox(widget);
    sanitizeRowsCB->setText(QStringLiteral("Sanitize Rows"));

    QFrame* line = new QFrame();
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);

    QFrame* line2 = new QFrame();
    line2->setFrameShape(QFrame::HLine);
    line2->setFrameShadow(QFrame::Sunken);

    QFrame* line3 = new QFrame();
    line3->setFrameShape(QFrame::HLine);
    line3->setFrameShadow(QFrame::Sunken);

    QFrame* line4 = new QFrame();
    line4->setFrameShape(QFrame::HLine);
    line4->setFrameShadow(QFrame::Sunken);

    QCheckBox *axisTitlesVisibleCB = new QCheckBox(widget);
    axisTitlesVisibleCB->setText(QStringLiteral("Axis titles visible"));
    axisTitlesVisibleCB->setChecked(false);

    QCheckBox *xAxisLabelsVisibleCB = new QCheckBox(widget);
    xAxisLabelsVisibleCB->setText(QStringLiteral("X-Axis labels visible"));
    xAxisLabelsVisibleCB->setChecked(true);

    QCheckBox *yAxisLabelsVisibleCB = new QCheckBox(widget);
    yAxisLabelsVisibleCB->setText(QStringLiteral("Y-Axis labels visible"));
    yAxisLabelsVisibleCB->setChecked(true);

    QCheckBox *zAxisLabelsVisibleCB = new QCheckBox(widget);
    zAxisLabelsVisibleCB->setText(QStringLiteral("Z-Axis labels visible"));
    zAxisLabelsVisibleCB->setChecked(true);

    QCheckBox *axisTitlesFixedCB = new QCheckBox(widget);
    axisTitlesFixedCB->setText(QStringLiteral("Axis titles fixed"));
    axisTitlesFixedCB->setChecked(true);

    QSlider *axisLabelRotationSlider = new QSlider(Qt::Horizontal, widget);
    axisLabelRotationSlider->setTickInterval(10);
    axisLabelRotationSlider->setTickPosition(QSlider::TicksBelow);
    axisLabelRotationSlider->setMinimum(0);
    axisLabelRotationSlider->setValue(0);
    axisLabelRotationSlider->setMaximum(90);

    QCheckBox *xAscendingCB = new QCheckBox(widget);
    xAscendingCB->setText(QStringLiteral("X Ascending"));
    xAscendingCB->setChecked(true);

    QCheckBox *zAscendingCB = new QCheckBox(widget);
    zAscendingCB->setText(QStringLiteral("Z Ascending"));
    zAscendingCB->setChecked(true);

    QCheckBox *polarCB = new QCheckBox(widget);
    polarCB->setText(QStringLiteral("Polar"));
    polarCB->setChecked(false);

    QCheckBox *surfaceTextureCB = new QCheckBox(widget);
    surfaceTextureCB->setText(QStringLiteral("Map texture"));
    surfaceTextureCB->setChecked(false);

    QCheckBox *surfaceAlphaTextureCB = new QCheckBox(widget);
    surfaceAlphaTextureCB->setText(QStringLiteral("Alpha texture"));
    surfaceAlphaTextureCB->setChecked(false);

    QCheckBox *surfaceAlphaTextureFileCB = new QCheckBox(widget);
    surfaceAlphaTextureFileCB->setText(QStringLiteral("Alpha texture file"));
    surfaceAlphaTextureFileCB->setChecked(false);

    QSlider *cameraTargetSliderX = new QSlider(Qt::Horizontal, widget);
    cameraTargetSliderX->setTickInterval(1);
    cameraTargetSliderX->setMinimum(-100);
    cameraTargetSliderX->setValue(0);
    cameraTargetSliderX->setMaximum(100);
    QSlider *cameraTargetSliderY = new QSlider(Qt::Horizontal, widget);
    cameraTargetSliderY->setTickInterval(1);
    cameraTargetSliderY->setMinimum(-100);
    cameraTargetSliderY->setValue(0);
    cameraTargetSliderY->setMaximum(100);
    QSlider *cameraTargetSliderZ = new QSlider(Qt::Horizontal, widget);
    cameraTargetSliderZ->setTickInterval(1);
    cameraTargetSliderZ->setMinimum(-100);
    cameraTargetSliderZ->setValue(0);
    cameraTargetSliderZ->setMaximum(100);

    QSlider *marginSlider = new QSlider(Qt::Horizontal, widget);
    marginSlider->setMinimum(-1);
    marginSlider->setValue(-1);
    marginSlider->setMaximum(100);

    QSlider *labelMarginSlider = new QSlider(Qt::Horizontal, widget);
    labelMarginSlider->setMinimum(-200);
    labelMarginSlider->setValue(100);
    labelMarginSlider->setMaximum(200);

    QSlider *xSegmentSlider = new QSlider(Qt::Horizontal, widget);
    xSegmentSlider->setMinimum(1);
    xSegmentSlider->setValue(2);
    xSegmentSlider->setMaximum(10);
    QSlider *ySegmentSlider = new QSlider(Qt::Horizontal, widget);
    ySegmentSlider->setMinimum(1);
    ySegmentSlider->setValue(2);
    ySegmentSlider->setMaximum(10);
    QSlider *zSegmentSlider = new QSlider(Qt::Horizontal, widget);
    zSegmentSlider->setMinimum(1);
    zSegmentSlider->setValue(2);
    zSegmentSlider->setMaximum(10);

    QSlider *xSubsegmentSlider = new QSlider(Qt::Horizontal, widget);
    xSubsegmentSlider->setMinimum(1);
    xSubsegmentSlider->setValue(2);
    xSubsegmentSlider->setMaximum(10);
    QSlider *ySubsegmentSlider = new QSlider(Qt::Horizontal, widget);
    ySubsegmentSlider->setMinimum(1);
    ySubsegmentSlider->setValue(2);
    ySubsegmentSlider->setMaximum(10);
    QSlider *zSubsegmentSlider = new QSlider(Qt::Horizontal, widget);
    zSubsegmentSlider->setMinimum(1);
    zSubsegmentSlider->setValue(2);
    zSubsegmentSlider->setMaximum(10);

    // Add controls to the layout
#ifdef MULTI_SERIES
    vLayout->addWidget(series1CB);
#endif
    vLayout->addWidget(smoothCB);
    vLayout->addWidget(surfaceGridCB);
    vLayout->addWidget(surfaceCB);
    vLayout->addWidget(seriesVisibleCB);
    vLayout->addWidget(fillCB);
#ifdef MULTI_SERIES
    vLayout->addWidget(line);
    vLayout->addWidget(series2CB);
    vLayout->addWidget(smoothS2CB);
    vLayout->addWidget(surfaceGridS2CB);
    vLayout->addWidget(surfaceS2CB);
    vLayout->addWidget(series2VisibleCB);
    vLayout->addWidget(fillS2CB);
    vLayout->addWidget(surfaceAlphaTextureFileCB);
    vLayout->addWidget(line2);
    vLayout->addWidget(series3CB);
    vLayout->addWidget(smoothS3CB);
    vLayout->addWidget(surfaceGridS3CB);
    vLayout->addWidget(surfaceS3CB);
    vLayout->addWidget(series3VisibleCB);
    vLayout->addWidget(fillS3CB);
    vLayout->addWidget(surfaceAlphaTextureCB);
    vLayout->addWidget(line3);
    vLayout->addWidget(series4CB);
    vLayout->addWidget(smoothS4CB);
    vLayout->addWidget(surfaceGridS4CB);
    vLayout->addWidget(surfaceS4CB);
    vLayout->addWidget(series4VisibleCB);
    vLayout->addWidget(fillS4CB);
    vLayout->addWidget(surfaceTextureCB);
    vLayout->addWidget(line4);
    vLayout->addWidget(lineSeriesCB);
    vLayout->addWidget(lineSeriesVisibleCB);
    vLayout->addWidget(surfaceGridLineCB);
    vLayout->addWidget(fillLineCB, 1, Qt::AlignTop);
#endif
#ifndef MULTI_SERIES
    vLayout->addWidget(new QLabel(QStringLiteral("Select surface sample")));
    vLayout->addWidget(sqrtSinCB);
    vLayout->addWidget(planeCB);
    vLayout->addWidget(heightMapCB);
    vLayout->addWidget(new QLabel(QStringLiteral("Adjust sample count")));
    vLayout->addWidget(gridSlidersLockCB);
    vLayout->addWidget(gridSliderX);
    vLayout->addWidget(gridSliderZ, 1, Qt::AlignTop);
#endif

    vLayout2->addWidget(new QLabel(QStringLiteral("Adjust vertical aspect ratio")));
    vLayout2->addWidget(aspectRatioSlider);
    vLayout2->addWidget(new QLabel(QStringLiteral("Adjust horizontal aspect ratio")));
    vLayout2->addWidget(horizontalAspectRatioSlider);
    vLayout2->addWidget(new QLabel(QStringLiteral("Adjust axis range")));
    vLayout2->addWidget(axisRangeSliderX);
    vLayout2->addWidget(axisRangeSliderY);
    vLayout2->addWidget(axisRangeSliderZ);
    vLayout2->addWidget(new QLabel(QStringLiteral("Adjust axis minimum")));
    vLayout2->addWidget(axisMinSliderX);
    vLayout2->addWidget(axisMinSliderY);
    vLayout2->addWidget(axisMinSliderZ);
    vLayout2->addWidget(new QLabel(QStringLiteral("Adjust cutoff margin")));
    vLayout2->addWidget(cutoffMarginSlider);
    vLayout2->addWidget(xAscendingCB);
    vLayout2->addWidget(zAscendingCB);
    vLayout2->addWidget(polarCB);
    vLayout2->addWidget(new QLabel(QStringLiteral("Change font")));
    vLayout2->addWidget(fontList);
    vLayout2->addWidget(new QLabel(QStringLiteral("Change Color Mode")));
    vLayout2->addWidget(colorSchemeList);
    vLayout2->addWidget(new QLabel(QStringLiteral("Change theme")));
    vLayout2->addWidget(themeList);
    vLayout2->addWidget(new QLabel(QStringLiteral("Adjust shadow quality")));
    vLayout2->addWidget(shadowQuality);
    vLayout2->addWidget(new QLabel(QStringLiteral("Change Transparency Technique")));
    vLayout2->addWidget(transparencyTechnique);
    vLayout2->addWidget(new QLabel(QStringLiteral("Selection Mode")));
    vLayout2->addWidget(selectionMode);
    vLayout2->addWidget(new QLabel(QStringLiteral("Camera target")));
    vLayout2->addWidget(cameraTargetSliderX);
    vLayout2->addWidget(cameraTargetSliderY);
    vLayout2->addWidget(cameraTargetSliderZ);
    vLayout2->addWidget(new QLabel(QStringLiteral("Adjust margin")), 0, Qt::AlignTop);
    vLayout2->addWidget(marginSlider);
    vLayout2->addWidget(new QLabel(QStringLiteral("Adjust label offset")), 0, Qt::AlignTop);
    vLayout2->addWidget(labelMarginSlider, 1, Qt::AlignTop);

    vLayout3->addWidget(labelButton);
    vLayout3->addWidget(meshButton);
#ifndef MULTI_SERIES
    vLayout3->addWidget(selectButton);
    vLayout3->addWidget(selectionInfoLabel);
    vLayout3->addWidget(flipViewsButton);
#endif

    vLayout3->addWidget(colorPB);
    vLayout3->addWidget(changeSubViews);
    vLayout3->addWidget(changeRowButton);
    vLayout3->addWidget(changeRowsButton);
    vLayout3->addWidget(changeMultipleRowsButton);
    vLayout3->addWidget(changeItemButton);
    vLayout3->addWidget(changeMultipleItemButton);
    vLayout3->addWidget(addRowButton);
    vLayout3->addWidget(addRowsButton);
    vLayout3->addWidget(insertRowButton);
    vLayout3->addWidget(insertRowsButton);
    vLayout3->addWidget(removeRowButton);
    vLayout3->addWidget(resetArrayButton);
    vLayout3->addWidget(resetArrayEmptyButton);
    vLayout3->addWidget(massiveDataTestButton);
    vLayout3->addWidget(testReverseButton);
    vLayout3->addWidget(testDataOrderingButton);
    vLayout3->addWidget(testNanSeriesButton);
    vLayout3->addWidget(sanitizeRowsCB);
    vLayout3->addWidget(axisTitlesVisibleCB);
    vLayout3->addWidget(xAxisLabelsVisibleCB);
    vLayout3->addWidget(yAxisLabelsVisibleCB);
    vLayout3->addWidget(zAxisLabelsVisibleCB);
    vLayout3->addWidget(axisTitlesFixedCB);
    vLayout3->addWidget(new QLabel(QStringLiteral("Axis label rotation")));
    vLayout3->addWidget(axisLabelRotationSlider, 1);
    vLayout3->addWidget(new QLabel(QStringLiteral("Adjust X-axis segments")), 0);
    vLayout3->addWidget(xSegmentSlider, 1);
    vLayout3->addWidget(xSubsegmentSlider, 1);
    vLayout3->addWidget(new QLabel(QStringLiteral("Adjust Y-axis segments")), 0);
    vLayout3->addWidget(ySegmentSlider, 1);
    vLayout3->addWidget(ySubsegmentSlider, 1);
    vLayout3->addWidget(new QLabel(QStringLiteral("Adjust Z-axis segments")), 0);
    vLayout3->addWidget(zSegmentSlider, 1);
    vLayout3->addWidget(zSubsegmentSlider, 1, Qt::AlignTop);

    widget->show();

    GraphModifier *modifier = new GraphModifier(surfaceGraph);

    // Connect controls to slots on modifier
    QObject::connect(smoothCB, &QCheckBox::checkStateChanged,
                     modifier, &GraphModifier::toggleSmooth);
    QObject::connect(surfaceGridCB, &QCheckBox::checkStateChanged,
                     modifier, &GraphModifier::toggleSurfaceGrid);
    QObject::connect(surfaceCB, &QCheckBox::checkStateChanged,
                     modifier, &GraphModifier::toggleSurface);
    QObject::connect(seriesVisibleCB, &QCheckBox::checkStateChanged,
                     modifier, &GraphModifier::toggleSeriesVisible);
    QObject::connect(fillCB, &QCheckBox::checkStateChanged,
                     modifier, &GraphModifier::toggleFill);
#ifdef MULTI_SERIES
    QObject::connect(smoothS2CB, &QCheckBox::checkStateChanged,
                     modifier, &GraphModifier::toggleSmoothS2);
    QObject::connect(surfaceGridS2CB, &QCheckBox::checkStateChanged,
                     modifier, &GraphModifier::toggleSurfaceGridS2);
    QObject::connect(surfaceS2CB, &QCheckBox::checkStateChanged,
                     modifier, &GraphModifier::toggleSurfaceS2);
    QObject::connect(series2VisibleCB, &QCheckBox::checkStateChanged,
                     modifier, &GraphModifier::toggleSeries2Visible);
    QObject::connect(fillS2CB, &QCheckBox::checkStateChanged,
                     modifier, &GraphModifier::toggleFillS2);

    QObject::connect(smoothS3CB, &QCheckBox::checkStateChanged,
                     modifier, &GraphModifier::toggleSmoothS3);
    QObject::connect(surfaceGridS3CB, &QCheckBox::checkStateChanged,
                     modifier, &GraphModifier::toggleSurfaceGridS3);
    QObject::connect(surfaceS3CB, &QCheckBox::checkStateChanged,
                     modifier, &GraphModifier::toggleSurfaceS3);
    QObject::connect(series3VisibleCB, &QCheckBox::checkStateChanged,
                     modifier, &GraphModifier::toggleSeries3Visible);
    QObject::connect(fillS3CB, &QCheckBox::checkStateChanged,
                     modifier, &GraphModifier::toggleFillS3);

    QObject::connect(smoothS4CB, &QCheckBox::checkStateChanged,
                     modifier, &GraphModifier::toggleSmoothS4);
    QObject::connect(surfaceGridS4CB, &QCheckBox::checkStateChanged,
                     modifier, &GraphModifier::toggleSurfaceGridS4);
    QObject::connect(surfaceS4CB, &QCheckBox::checkStateChanged,
                     modifier, &GraphModifier::toggleSurfaceS4);
    QObject::connect(series4VisibleCB, &QCheckBox::checkStateChanged,
                     modifier, &GraphModifier::toggleSeries4Visible);
    QObject::connect(fillS4CB, &QCheckBox::checkStateChanged,
                     modifier, &GraphModifier::toggleFillS4);

    QObject::connect(lineSeriesVisibleCB, &QCheckBox::checkStateChanged,
                     modifier, &GraphModifier::toggleLineSeries);
    QObject::connect(surfaceGridLineCB, &QCheckBox::checkStateChanged,
                     modifier, &GraphModifier::toggleLineSurfaceGrid);
    QObject::connect(fillLineCB, &QCheckBox::checkStateChanged,
                     modifier, &GraphModifier::toggleFillLine);


    CheckBoxWrapper *series1SmoothCBWrapper = new CheckBoxWrapper(smoothCB);
    CheckBoxWrapper *series1SurfaceGridCBWrapper = new CheckBoxWrapper(surfaceGridCB);
    CheckBoxWrapper *series1surfaceCBWrapper = new CheckBoxWrapper(surfaceCB);
    CheckBoxWrapper *series1VisibleCBWrapper = new CheckBoxWrapper(seriesVisibleCB);
    QObject::connect(series1CB, &QCheckBox::checkStateChanged,
                     modifier, &GraphModifier::toggleSeries1);
    QObject::connect(series1CB, &QCheckBox::checkStateChanged,
                     series1SmoothCBWrapper, &CheckBoxWrapper::setEnabled);
    QObject::connect(series1CB, &QCheckBox::checkStateChanged,
                     series1SurfaceGridCBWrapper, &CheckBoxWrapper::setEnabled);
    QObject::connect(series1CB, &QCheckBox::checkStateChanged,
                     series1surfaceCBWrapper, &CheckBoxWrapper::setEnabled);
    QObject::connect(series1CB, &QCheckBox::checkStateChanged,
                     series1VisibleCBWrapper, &CheckBoxWrapper::setEnabled);


    CheckBoxWrapper *series2SmoothCBWrapper = new CheckBoxWrapper(smoothS2CB);
    CheckBoxWrapper *series2SurfaceGridCBWrapper = new CheckBoxWrapper(surfaceGridS2CB);
    CheckBoxWrapper *series2surfaceCBWrapper = new CheckBoxWrapper(surfaceS2CB);
    CheckBoxWrapper *series2VisibleCBWrapper = new CheckBoxWrapper(series2VisibleCB);
    QObject::connect(series2CB, &QCheckBox::checkStateChanged,
                     modifier, &GraphModifier::toggleSeries2);
    QObject::connect(series2CB, &QCheckBox::checkStateChanged,
                     series2SmoothCBWrapper, &CheckBoxWrapper::setEnabled);
    QObject::connect(series2CB, &QCheckBox::checkStateChanged,
                     series2SurfaceGridCBWrapper, &CheckBoxWrapper::setEnabled);
    QObject::connect(series2CB, &QCheckBox::checkStateChanged,
                     series2surfaceCBWrapper, &CheckBoxWrapper::setEnabled);
    QObject::connect(series2CB, &QCheckBox::checkStateChanged,
                     series2VisibleCBWrapper, &CheckBoxWrapper::setEnabled);

    CheckBoxWrapper *series3SmoothCBWrapper = new CheckBoxWrapper(smoothS3CB);
    CheckBoxWrapper *series3SurfaceGridCBWrapper = new CheckBoxWrapper(surfaceGridS3CB);
    CheckBoxWrapper *series3surfaceCBWrapper = new CheckBoxWrapper(surfaceS3CB);
    CheckBoxWrapper *series3VisibleCBWrapper = new CheckBoxWrapper(series3VisibleCB);
    QObject::connect(series3CB, &QCheckBox::checkStateChanged,
                     modifier, &GraphModifier::toggleSeries3);
    QObject::connect(series3CB, &QCheckBox::checkStateChanged,
                     series3SmoothCBWrapper, &CheckBoxWrapper::setEnabled);
    QObject::connect(series3CB, &QCheckBox::checkStateChanged,
                     series3SurfaceGridCBWrapper, &CheckBoxWrapper::setEnabled);
    QObject::connect(series3CB, &QCheckBox::checkStateChanged,
                     series3surfaceCBWrapper, &CheckBoxWrapper::setEnabled);
    QObject::connect(series3CB, &QCheckBox::checkStateChanged,
                     series3VisibleCBWrapper, &CheckBoxWrapper::setEnabled);

    CheckBoxWrapper *series4SmoothCBWrapper = new CheckBoxWrapper(smoothS4CB);
    CheckBoxWrapper *series4SurfaceGridCBWrapper = new CheckBoxWrapper(surfaceGridS4CB);
    CheckBoxWrapper *series4surfaceCBWrapper = new CheckBoxWrapper(surfaceS4CB);
    CheckBoxWrapper *series4VisibleCBWrapper = new CheckBoxWrapper(series4VisibleCB);
    QObject::connect(series4CB, &QCheckBox::checkStateChanged,
                     modifier, &GraphModifier::toggleSeries4);
    QObject::connect(series4CB, &QCheckBox::checkStateChanged,
                     series4SmoothCBWrapper, &CheckBoxWrapper::setEnabled);
    QObject::connect(series4CB, &QCheckBox::checkStateChanged,
                     series4SurfaceGridCBWrapper, &CheckBoxWrapper::setEnabled);
    QObject::connect(series4CB, &QCheckBox::checkStateChanged,
                     series4surfaceCBWrapper, &CheckBoxWrapper::setEnabled);
    QObject::connect(series4CB, &QCheckBox::checkStateChanged,
                     series4VisibleCBWrapper, &CheckBoxWrapper::setEnabled);

    CheckBoxWrapper *lineSeriesSurfaceGridCBWrapper = new CheckBoxWrapper(surfaceGridLineCB);
    CheckBoxWrapper *lineSeriessurfaceCBWrapper = new CheckBoxWrapper(lineSeriesCB);
    CheckBoxWrapper *lineSeriesVisibleCBWrapper = new CheckBoxWrapper(lineSeriesVisibleCB);
    CheckBoxWrapper *lineSeriesFillCBWrapper = new CheckBoxWrapper(fillLineCB);
    QObject::connect(lineSeriesCB, &QCheckBox::checkStateChanged,
                     modifier, &GraphModifier::toggleLineSeries);
    QObject::connect(lineSeriesCB, &QCheckBox::checkStateChanged,
                     lineSeriesSurfaceGridCBWrapper, &CheckBoxWrapper::setEnabled);
    QObject::connect(lineSeriesCB, &QCheckBox::checkStateChanged,
                     lineSeriesVisibleCBWrapper, &CheckBoxWrapper::setEnabled);
    QObject::connect(lineSeriesCB, &QCheckBox::checkStateChanged,
                     lineSeriesFillCBWrapper, &CheckBoxWrapper::setEnabled);
#else
    QObject::connect(sqrtSinCB, &QRadioButton::toggled,
                     modifier, &GraphModifier::toggleSqrtSin);
    QObject::connect(planeCB, &QCheckBox::toggled,
                     modifier, &GraphModifier::togglePlane);
    QObject::connect(heightMapCB, &QCheckBox::toggled,
                     modifier, &GraphModifier::setHeightMapData);
    QObject::connect(gridSlidersLockCB, &QCheckBox::checkStateChanged,
                     modifier, &GraphModifier::toggleGridSliderLock);
    QObject::connect(gridSliderX, &QSlider::valueChanged,
                     modifier, &GraphModifier::adjustXCount);
    QObject::connect(gridSliderZ, &QSlider::valueChanged,
                     modifier, &GraphModifier::adjustZCount);
#endif
    QObject::connect(axisRangeSliderX, &QSlider::valueChanged,
                     modifier, &GraphModifier::adjustXRange);
    QObject::connect(axisRangeSliderY, &QSlider::valueChanged,
                     modifier, &GraphModifier::adjustYRange);
    QObject::connect(axisRangeSliderZ, &QSlider::valueChanged,
                     modifier, &GraphModifier::adjustZRange);
    QObject::connect(axisMinSliderX, &QSlider::valueChanged,
                     modifier, &GraphModifier::adjustXMin);
    QObject::connect(axisMinSliderY, &QSlider::valueChanged,
                     modifier, &GraphModifier::adjustYMin);
    QObject::connect(axisMinSliderZ, &QSlider::valueChanged,
                     modifier, &GraphModifier::adjustZMin);
    QObject::connect(cutoffMarginSlider, &QSlider::valueChanged,
                     modifier, &GraphModifier::adjustCutoffMargin);
    QObject::connect(colorPB, &QPushButton::pressed,
                     modifier, &GraphModifier::gradientPressed);
    QObject::connect(fontList, &QFontComboBox::currentFontChanged,
                     modifier, &GraphModifier::changeFont);
    QObject::connect(labelButton, &QPushButton::clicked,
                     modifier, &GraphModifier::changeStyle);
    QObject::connect(meshButton, &QPushButton::clicked,
                     modifier, &GraphModifier::changeMesh);
    QObject::connect(colorSchemeList, SIGNAL(currentIndexChanged(int)),
                     modifier, SLOT(changeColorScheme(int)));
    QObject::connect(themeList, SIGNAL(currentIndexChanged(int)),
                     modifier, SLOT(changeTheme(int)));
    QObject::connect(shadowQuality, SIGNAL(currentIndexChanged(int)),
                     modifier, SLOT(changeShadowQuality(int)));
    QObject::connect(transparencyTechnique,
                     SIGNAL(currentIndexChanged(int)),
                     modifier,
                     SLOT(changeTransparencyTechnique(int)));
    QObject::connect(selectionMode,
                     SIGNAL(currentIndexChanged(int)),
                     modifier,
                     SLOT(changeSelectionMode(int)));
#ifndef MULTI_SERIES
    QObject::connect(selectButton, &QPushButton::clicked,
                     modifier, &GraphModifier::selectButtonClicked);
    QObject::connect(flipViewsButton, &QPushButton::clicked,
                     modifier, &GraphModifier::flipViews);
#endif
    QObject::connect(changeSubViews, &QPushButton::clicked, modifier, &GraphModifier::changeSubView);
    QObject::connect(changeRowButton, &QPushButton::clicked, modifier, &GraphModifier::changeRow);
    QObject::connect(changeRowsButton,&QPushButton::clicked,
                     modifier, &GraphModifier::changeRows);
    QObject::connect(changeItemButton,&QPushButton::clicked,
                     modifier, &GraphModifier::changeItem);
    QObject::connect(changeMultipleItemButton,&QPushButton::clicked,
                     modifier, &GraphModifier::changeMultipleItem);
    QObject::connect(changeMultipleRowsButton,&QPushButton::clicked,
                     modifier, &GraphModifier::changeMultipleRows);
    QObject::connect(addRowButton,&QPushButton::clicked,
                     modifier, &GraphModifier::addRow);
    QObject::connect(addRowsButton,&QPushButton::clicked,
                     modifier, &GraphModifier::addRows);
    QObject::connect(insertRowButton,&QPushButton::clicked,
                     modifier, &GraphModifier::insertRow);
    QObject::connect(insertRowsButton,&QPushButton::clicked,
                     modifier, &GraphModifier::insertRows);
    QObject::connect(removeRowButton,&QPushButton::clicked,
                     modifier, &GraphModifier::removeRow);
    QObject::connect(resetArrayButton,&QPushButton::clicked,
                     modifier, &GraphModifier::resetArray);
    QObject::connect(resetArrayEmptyButton,&QPushButton::clicked,
                     modifier, &GraphModifier::resetArrayEmpty);
    QObject::connect(massiveDataTestButton,&QPushButton::clicked,
                     modifier, &GraphModifier::massiveDataTest);
    QObject::connect(testReverseButton, &QPushButton::clicked,
                     modifier, &GraphModifier::testAxisReverse);
    QObject::connect(testDataOrderingButton, &QPushButton::clicked,
                     modifier, &GraphModifier::testDataOrdering);
    QObject::connect(testNanSeriesButton, &QPushButton::clicked,
                     modifier, &GraphModifier::testNanSeries);
    QObject::connect(sanitizeRowsCB, &QCheckBox::checkStateChanged,
                     modifier, &GraphModifier::setRowSanitization);
    QObject::connect(axisTitlesVisibleCB, &QCheckBox::checkStateChanged,
                     modifier, &GraphModifier::toggleAxisTitleVisibility);
    QObject::connect(xAxisLabelsVisibleCB,
                     &QCheckBox::checkStateChanged,
                     modifier,
                     &GraphModifier::toggleXAxisLabelsVisibility);
    QObject::connect(yAxisLabelsVisibleCB,
                     &QCheckBox::checkStateChanged,
                     modifier,
                     &GraphModifier::toggleYAxisLabelsVisibility);
    QObject::connect(zAxisLabelsVisibleCB,
                     &QCheckBox::checkStateChanged,
                     modifier,
                     &GraphModifier::toggleZAxisLabelsVisibility);
    QObject::connect(axisTitlesFixedCB,
                     &QCheckBox::checkStateChanged,
                     modifier,
                     &GraphModifier::toggleAxisTitleFixed);
    QObject::connect(axisLabelRotationSlider, &QSlider::valueChanged, modifier,
                     &GraphModifier::changeLabelRotation);
    QObject::connect(xAscendingCB, &QCheckBox::checkStateChanged,
                     modifier, &GraphModifier::toggleXAscending);
    QObject::connect(zAscendingCB, &QCheckBox::checkStateChanged,
                     modifier, &GraphModifier::toggleZAscending);
    QObject::connect(polarCB, &QCheckBox::checkStateChanged,
                     modifier, &GraphModifier::togglePolar);

    QObject::connect(aspectRatioSlider, &QSlider::valueChanged,
                     modifier, &GraphModifier::setAspectRatio);
    QObject::connect(horizontalAspectRatioSlider, &QSlider::valueChanged,
                     modifier, &GraphModifier::setHorizontalAspectRatio);
    QObject::connect(surfaceTextureCB, &QCheckBox::checkStateChanged,
                     modifier, &GraphModifier::setSurfaceTexture);
    QObject::connect(surfaceAlphaTextureCB,
                     &QCheckBox::checkStateChanged,
                     modifier,
                     &GraphModifier::setSurfaceAlphaTexture);
    QObject::connect(surfaceAlphaTextureFileCB,
                     &QCheckBox::checkStateChanged,
                     modifier,
                     &GraphModifier::setSurfaceAlphaTextureFile);
    QObject::connect(cameraTargetSliderX,
                     &QSlider::valueChanged,
                     modifier,
                     &GraphModifier::setCameraTargetX);
    QObject::connect(cameraTargetSliderY, &QSlider::valueChanged, modifier,
                     &GraphModifier::setCameraTargetY);
    QObject::connect(cameraTargetSliderZ, &QSlider::valueChanged, modifier,
                     &GraphModifier::setCameraTargetZ);
    QObject::connect(marginSlider, &QSlider::valueChanged, modifier,
                     &GraphModifier::setGraphMargin);
    QObject::connect(labelMarginSlider,
                     &QSlider::valueChanged,
                     modifier,
                     &GraphModifier::setLabelMargin);

    QObject::connect(xSegmentSlider,
                     &QSlider::valueChanged,
                     modifier,
                     &GraphModifier::setXAxisSegemntCount);
    QObject::connect(ySegmentSlider,
                     &QSlider::valueChanged,
                     modifier,
                     &GraphModifier::setYAxisSegemntCount);
    QObject::connect(zSegmentSlider,
                     &QSlider::valueChanged,
                     modifier,
                     &GraphModifier::setZAxisSegemntCount);
    QObject::connect(xSubsegmentSlider,
                     &QSlider::valueChanged,
                     modifier,
                     &GraphModifier::setXAxisSubsegemntCount);
    QObject::connect(ySubsegmentSlider,
                     &QSlider::valueChanged,
                     modifier,
                     &GraphModifier::setYAxisSubsegemntCount);
    QObject::connect(zSubsegmentSlider,
                     &QSlider::valueChanged,
                     modifier,
                     &GraphModifier::setZAxisSubsegemntCount);

#ifdef MULTI_SERIES
    modifier->setSeries1CB(series1CB);
    modifier->setSeries2CB(series2CB);
    modifier->setSeries3CB(series3CB);
    modifier->setSeries4CB(series4CB);
    series1CB->setChecked(true);
    series2CB->setChecked(true);
    series3CB->setChecked(true);
    series4CB->setChecked(true);
    lineSeriesCB->setChecked(true);
#endif
    modifier->setAxisRangeSliderX(axisRangeSliderX);
    modifier->setAxisRangeSliderZ(axisRangeSliderZ);
    modifier->setAxisMinSliderX(axisMinSliderX);
    modifier->setAxisMinSliderZ(axisMinSliderZ);
    selectionMode->setCurrentIndex(1);
#ifndef MULTI_SERIES
    modifier->setGridSliderZ(gridSliderZ);
    modifier->setGridSliderX(gridSliderX);
    modifier->toggleGridSliderLock(gridSlidersLockCB->checkState());
    modifier->setSelectionInfoLabel(selectionInfoLabel);
    sqrtSinCB->setChecked(true);
#endif
    shadowQuality->setCurrentIndex(3);
    int retVal = app.exec();
    delete modifier;
    delete quickWidget;
    return retVal;
}
