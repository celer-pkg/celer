// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "scatterchart.h"

#include <QApplication>
#include <QWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QCheckBox>
#include <QSlider>
#include <QComboBox>
#include <QFontComboBox>
#include <QLabel>
#include <QScreen>
#include <QFontDatabase>
#include <QLinearGradient>
#include <QPainter>

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    //QCoreApplication::setAttribute(Qt::AA_UseOpenGLES);

    QWidget *widget = new QWidget;
    QHBoxLayout *hLayout = new QHBoxLayout(widget);
    QVBoxLayout *vLayout = new QVBoxLayout();
    QVBoxLayout *vLayout2 = new QVBoxLayout();
    QVBoxLayout *vLayout3 = new QVBoxLayout();
    auto quickWidget = new QQuickWidget;
    Q3DScatterWidgetItem *graph = new Q3DScatterWidgetItem();
    graph->setWidget(quickWidget);
    QSize screenSize = graph->widget()->screen()->size();

    graph->widget()->setMinimumSize(QSize(screenSize.width() / 2, screenSize.height() / 2));
    graph->widget()->setMaximumSize(screenSize);
    graph->widget()->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    graph->widget()->setFocusPolicy(Qt::StrongFocus);
    graph->widget()->setResizeMode(QQuickWidget::SizeRootObjectToView);

    widget->setWindowTitle(QStringLiteral("values of some things in something"));

    hLayout->addWidget(graph->widget(), 1);
    hLayout->addLayout(vLayout);
    hLayout->addLayout(vLayout2);
    hLayout->addLayout(vLayout3);

    QPushButton *themeButton = new QPushButton(widget);
    themeButton->setText(QStringLiteral("Change theme"));

    QPushButton *labelButton = new QPushButton(widget);
    labelButton->setText(QStringLiteral("Change label style"));

    QPushButton *styleButton = new QPushButton(widget);
    styleButton->setText(QStringLiteral("Change item style"));

    QPushButton *cameraButton = new QPushButton(widget);
    cameraButton->setText(QStringLiteral("Change camera preset"));

    QPushButton *clearButton = new QPushButton(widget);
    clearButton->setText(QStringLiteral("Clear graph"));

    QPushButton *resetButton = new QPushButton(widget);
    resetButton->setText(QStringLiteral("Reset axes"));

    QPushButton *addOneButton = new QPushButton(widget);
    addOneButton->setText(QStringLiteral("Add item"));

    QPushButton *addBunchButton = new QPushButton(widget);
    addBunchButton->setText(QStringLiteral("Add bunch of items"));

    QPushButton *insertOneButton = new QPushButton(widget);
    insertOneButton->setText(QStringLiteral("Insert item"));

    QPushButton *insertBunchButton = new QPushButton(widget);
    insertBunchButton->setText(QStringLiteral("Insert bunch of items"));

    QPushButton *changeOneButton = new QPushButton(widget);
    changeOneButton->setText(QStringLiteral("Change selected item"));

    QPushButton *changeBunchButton = new QPushButton(widget);
    changeBunchButton->setText(QStringLiteral("Change bunch of items"));

    QPushButton *removeOneButton = new QPushButton(widget);
    removeOneButton->setText(QStringLiteral("Remove selected item"));

    QPushButton *removeBunchButton = new QPushButton(widget);
    removeBunchButton->setText(QStringLiteral("Remove bunch of items"));

    QPushButton *setSelectedItemButton = new QPushButton(widget);
    setSelectedItemButton->setText(QStringLiteral("Select/deselect item 3"));

    QPushButton *clearSeriesDataButton = new QPushButton(widget);
    clearSeriesDataButton->setText(QStringLiteral("Clear series data"));

    QPushButton *addSeriesButton = new QPushButton(widget);
    addSeriesButton->setText(QStringLiteral("Add Series"));

    QPushButton *removeSeriesButton = new QPushButton(widget);
    removeSeriesButton->setText(QStringLiteral("Remove Series"));

    QPushButton *toggleSeriesVisibilityButton = new QPushButton(widget);
    toggleSeriesVisibilityButton->setText(QStringLiteral("Toggle visibility"));

    QPushButton *changeSeriesNameButton = new QPushButton(widget);
    changeSeriesNameButton->setText(QStringLiteral("Series name"));

    QPushButton *startTimerButton = new QPushButton(widget);
    startTimerButton->setText(QStringLiteral("Start/stop timer"));

    QPushButton *massiveDataTestButton = new QPushButton(widget);
    massiveDataTestButton->setText(QStringLiteral("Massive data test"));

    QPushButton *testItemChangesButton = new QPushButton(widget);
    testItemChangesButton->setText(QStringLiteral("Test Item changing"));

    QPushButton *testReverseButton = new QPushButton(widget);
    testReverseButton->setText(QStringLiteral("Test Axis Reversing"));

    QPushButton *testNanSeriesButton = new QPushButton(widget);
    testNanSeriesButton->setText(QStringLiteral("Test NaN Series"));

    QPushButton *renderToImageButton = new QPushButton(widget);
    renderToImageButton->setText(QStringLiteral("Render the graph to an image"));

    QLinearGradient grBtoY(0, 0, 100, 0);
    grBtoY.setColorAt(1.0, Qt::black);
    grBtoY.setColorAt(0.67, Qt::blue);
    grBtoY.setColorAt(0.33, Qt::red);
    grBtoY.setColorAt(0.0, Qt::yellow);
    QPixmap pm(100, 24);
    QPainter pmp(&pm);
    pmp.setBrush(QBrush(grBtoY));
    pmp.setPen(Qt::NoPen);
    pmp.drawRect(0, 0, 100, 24);
    QPushButton *gradientBtoYPB = new QPushButton(widget);
    gradientBtoYPB->setIcon(QIcon(pm));
    gradientBtoYPB->setIconSize(QSize(100, 24));

    QLabel *fpsLabel = new QLabel(QStringLiteral(""));

    QCheckBox *fpsCheckBox = new QCheckBox(widget);
    fpsCheckBox->setText(QStringLiteral("Measure Fps"));
    fpsCheckBox->setChecked(false);

    QCheckBox *backgroundCheckBox = new QCheckBox(widget);
    backgroundCheckBox->setText(QStringLiteral("Show background"));
    backgroundCheckBox->setChecked(true);

    QCheckBox *gridCheckBox = new QCheckBox(widget);
    gridCheckBox->setText(QStringLiteral("Show grid"));
    gridCheckBox->setChecked(true);

    QComboBox *shadowQuality = new QComboBox(widget);
    shadowQuality->addItem(QStringLiteral("None"));
    shadowQuality->addItem(QStringLiteral("Low"));
    shadowQuality->addItem(QStringLiteral("Medium"));
    shadowQuality->addItem(QStringLiteral("High"));
    shadowQuality->addItem(QStringLiteral("Low Soft"));
    shadowQuality->addItem(QStringLiteral("Medium Soft"));
    shadowQuality->addItem(QStringLiteral("High Soft"));
    shadowQuality->setCurrentIndex(0);

    QComboBox *lightingMode = new QComboBox(widget);
    lightingMode->addItem(QStringLiteral("Shaded"));
    lightingMode->addItem(QStringLiteral("Unshaded"));
    lightingMode->setCurrentIndex(0);


    QFontComboBox *fontList = new QFontComboBox(widget);

    QSlider *fontSizeSlider = new QSlider(Qt::Horizontal, widget);
    fontSizeSlider->setTickInterval(15);
    fontSizeSlider->setTickPosition(QSlider::TicksBelow);
    fontSizeSlider->setMinimum(1);
    fontSizeSlider->setValue(30);
    fontSizeSlider->setMaximum(200);

    QSlider *labelSizeSlider = new QSlider(Qt::Horizontal, widget);
    labelSizeSlider->setTickInterval(1);
    labelSizeSlider->setTickPosition(QSlider::TicksBelow);
    labelSizeSlider->setMinimum(1);
    labelSizeSlider->setValue(10);
    labelSizeSlider->setMaximum(30);

    QSlider *pointSizeSlider = new QSlider(Qt::Horizontal, widget);
    pointSizeSlider->setTickInterval(15);
    pointSizeSlider->setTickPosition(QSlider::TicksBelow);
    pointSizeSlider->setMinimum(1);
    pointSizeSlider->setValue(30);
    pointSizeSlider->setMaximum(100);

    QSlider *minSliderX = new QSlider(Qt::Horizontal, widget);
    minSliderX->setTickInterval(50);
    minSliderX->setTickPosition(QSlider::TicksBelow);
    minSliderX->setMinimum(-100);
    minSliderX->setValue(-50);
    minSliderX->setMaximum(100);

    QSlider *minSliderY = new QSlider(Qt::Horizontal, widget);
    minSliderY->setTickInterval(100);
    minSliderY->setTickPosition(QSlider::TicksBelow);
    minSliderY->setMinimum(-200);
    minSliderY->setValue(-100);
    minSliderY->setMaximum(200);

    QSlider *minSliderZ = new QSlider(Qt::Horizontal, widget);
    minSliderZ->setTickInterval(50);
    minSliderZ->setTickPosition(QSlider::TicksBelow);
    minSliderZ->setMinimum(-100);
    minSliderZ->setValue(-50);
    minSliderZ->setMaximum(100);

    QSlider *maxSliderX = new QSlider(Qt::Horizontal, widget);
    maxSliderX->setTickInterval(50);
    maxSliderX->setTickPosition(QSlider::TicksAbove);
    maxSliderX->setMinimum(-100);
    maxSliderX->setValue(50);
    maxSliderX->setMaximum(100);

    QSlider *maxSliderY = new QSlider(Qt::Horizontal, widget);
    maxSliderY->setTickInterval(100);
    maxSliderY->setTickPosition(QSlider::TicksAbove);
    maxSliderY->setMinimum(-200);
    maxSliderY->setValue(120);
    maxSliderY->setMaximum(200);

    QSlider *maxSliderZ = new QSlider(Qt::Horizontal, widget);
    maxSliderZ->setTickInterval(50);
    maxSliderZ->setTickPosition(QSlider::TicksAbove);
    maxSliderZ->setMinimum(-100);
    maxSliderZ->setValue(50);
    maxSliderZ->setMaximum(100);

    QSlider *cutoffMarginSlider = new QSlider(Qt::Horizontal, widget);
    cutoffMarginSlider->setTickInterval(500);
    cutoffMarginSlider->setTickPosition(QSlider::TicksAbove);
    cutoffMarginSlider->setMinimum(-1000);
    cutoffMarginSlider->setValue(0);
    cutoffMarginSlider->setMaximum(1000);

    QSlider *aspectRatioSlider = new QSlider(Qt::Horizontal, widget);
    aspectRatioSlider->setTickInterval(10);
    aspectRatioSlider->setTickPosition(QSlider::TicksBelow);
    aspectRatioSlider->setMinimum(1);
    aspectRatioSlider->setValue(20);
    aspectRatioSlider->setMaximum(100);

    QSlider *horizontalAspectRatioSlider = new QSlider(Qt::Horizontal, widget);
    horizontalAspectRatioSlider->setTickInterval(30);
    horizontalAspectRatioSlider->setTickPosition(QSlider::TicksBelow);
    horizontalAspectRatioSlider->setMinimum(0);
    horizontalAspectRatioSlider->setValue(0);
    horizontalAspectRatioSlider->setMaximum(300);

    QCheckBox *optimizationLegacyCB = new QCheckBox(widget);
    optimizationLegacyCB->setText(QStringLiteral("Legacy optimization"));
    optimizationLegacyCB->setChecked(false);

    QCheckBox *orthoCB = new QCheckBox(widget);
    orthoCB->setText(QStringLiteral("Orthographic projection"));
    orthoCB->setChecked(false);

    QCheckBox *polarCB = new QCheckBox(widget);
    polarCB->setText(QStringLiteral("Polar graph"));
    polarCB->setChecked(false);

    QCheckBox *axisTitlesVisibleCB = new QCheckBox(widget);
    axisTitlesVisibleCB->setText(QStringLiteral("Axis titles visible"));
    axisTitlesVisibleCB->setChecked(false);

    QCheckBox *labelAdjustCB = new QCheckBox(widget);
    labelAdjustCB->setText(QStringLiteral("Use label adjustment"));
    labelAdjustCB->setChecked(false);

    QCheckBox *axisTitlesFixedCB = new QCheckBox(widget);
    axisTitlesFixedCB->setText(QStringLiteral("Axis titles fixed"));
    axisTitlesFixedCB->setChecked(true);

    QSlider *axisLabelRotationSlider = new QSlider(Qt::Horizontal, widget);
    axisLabelRotationSlider->setTickInterval(10);
    axisLabelRotationSlider->setTickPosition(QSlider::TicksBelow);
    axisLabelRotationSlider->setMinimum(0);
    axisLabelRotationSlider->setValue(0);
    axisLabelRotationSlider->setMaximum(90);

    QSlider *axisTitleOffsetSlider = new QSlider(Qt::Horizontal, widget);
    axisTitleOffsetSlider->setTickInterval(10);
    axisTitleOffsetSlider->setTickPosition(QSlider::TicksBelow);
    axisTitleOffsetSlider->setMinimum(-100);
    axisTitleOffsetSlider->setValue(0);
    axisTitleOffsetSlider->setMaximum(100);

    QSlider *radialLabelSlider = new QSlider(Qt::Horizontal, widget);
    radialLabelSlider->setTickInterval(10);
    radialLabelSlider->setTickPosition(QSlider::TicksBelow);
    radialLabelSlider->setMinimum(0);
    radialLabelSlider->setValue(100);
    radialLabelSlider->setMaximum(150);

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

    vLayout->addWidget(themeButton, 0, Qt::AlignTop);
    vLayout->addWidget(labelButton, 0, Qt::AlignTop);
    vLayout->addWidget(styleButton, 0, Qt::AlignTop);
    vLayout->addWidget(cameraButton, 0, Qt::AlignTop);
    vLayout->addWidget(clearButton, 0, Qt::AlignTop);
    vLayout->addWidget(resetButton, 0, Qt::AlignTop);
    vLayout->addWidget(addOneButton, 0, Qt::AlignTop);
    vLayout->addWidget(addBunchButton, 0, Qt::AlignTop);
    vLayout->addWidget(insertOneButton, 0, Qt::AlignTop);
    vLayout->addWidget(insertBunchButton, 0, Qt::AlignTop);
    vLayout->addWidget(changeOneButton, 0, Qt::AlignTop);
    vLayout->addWidget(changeBunchButton, 0, Qt::AlignTop);
    vLayout->addWidget(removeOneButton, 0, Qt::AlignTop);
    vLayout->addWidget(removeBunchButton, 0, Qt::AlignTop);
    vLayout->addWidget(setSelectedItemButton, 0, Qt::AlignTop);
    vLayout->addWidget(clearSeriesDataButton, 0, Qt::AlignTop);
    vLayout->addWidget(addSeriesButton, 0, Qt::AlignTop);
    vLayout->addWidget(removeSeriesButton, 0, Qt::AlignTop);
    vLayout->addWidget(toggleSeriesVisibilityButton, 0, Qt::AlignTop);
    vLayout->addWidget(changeSeriesNameButton, 0, Qt::AlignTop);
    vLayout->addWidget(startTimerButton, 0, Qt::AlignTop);
    vLayout->addWidget(massiveDataTestButton, 0, Qt::AlignTop);
    vLayout->addWidget(testItemChangesButton, 0, Qt::AlignTop);
    vLayout->addWidget(testReverseButton, 0, Qt::AlignTop);
    vLayout->addWidget(testNanSeriesButton, 0, Qt::AlignTop);
    vLayout->addWidget(renderToImageButton, 1, Qt::AlignTop);

    vLayout2->addWidget(gradientBtoYPB, 0, Qt::AlignTop);
    vLayout2->addWidget(fpsLabel, 0, Qt::AlignTop);
    vLayout2->addWidget(fpsCheckBox, 0, Qt::AlignTop);
    vLayout2->addWidget(backgroundCheckBox);
    vLayout2->addWidget(gridCheckBox);
    vLayout2->addWidget(new QLabel(QStringLiteral("Adjust shadow quality")));
    vLayout2->addWidget(shadowQuality, 0, Qt::AlignTop);
    vLayout2->addWidget(new QLabel(QStringLiteral("Adjust lighting mode")));
    vLayout2->addWidget(lightingMode, 0, Qt::AlignTop);
    vLayout2->addWidget(new QLabel(QStringLiteral("Adjust point size")));
    vLayout2->addWidget(pointSizeSlider, 0, Qt::AlignTop);
    vLayout2->addWidget(new QLabel(QStringLiteral("Adjust data window")));
    vLayout2->addWidget(minSliderX, 0, Qt::AlignTop);
    vLayout2->addWidget(maxSliderX, 0, Qt::AlignTop);
    vLayout2->addWidget(minSliderY, 0, Qt::AlignTop);
    vLayout2->addWidget(maxSliderY, 0, Qt::AlignTop);
    vLayout2->addWidget(minSliderZ, 0, Qt::AlignTop);
    vLayout2->addWidget(maxSliderZ, 0, Qt::AlignTop);
    vLayout2->addWidget(new QLabel(QStringLiteral("Adjust cutoffMargin")));
    vLayout2->addWidget(cutoffMarginSlider, 0, Qt::AlignTop);
    vLayout2->addWidget(new QLabel(QStringLiteral("Change font")));
    vLayout2->addWidget(fontList);
    vLayout2->addWidget(new QLabel(QStringLiteral("Adjust font size")));
    vLayout2->addWidget(fontSizeSlider);
    vLayout2->addWidget(new QLabel(QStringLiteral("Adjust label size")));
    vLayout2->addWidget(labelSizeSlider);
    vLayout2->addWidget(new QLabel(QStringLiteral("Adjust vertical aspect ratio")));
    vLayout2->addWidget(aspectRatioSlider);
    vLayout2->addWidget(new QLabel(QStringLiteral("Adjust horizontal aspect ratio")));
    vLayout2->addWidget(horizontalAspectRatioSlider, 1, Qt::AlignTop);

    vLayout3->addWidget(optimizationLegacyCB);
    vLayout3->addWidget(orthoCB);
    vLayout3->addWidget(polarCB);
    vLayout3->addWidget(axisTitlesVisibleCB);
    vLayout3->addWidget(axisTitlesFixedCB);
    vLayout3->addWidget(labelAdjustCB);
    vLayout3->addWidget(new QLabel(QStringLiteral("Axis label rotation")));
    vLayout3->addWidget(axisLabelRotationSlider);
    vLayout3->addWidget(new QLabel(QStringLiteral("Axis title offset")));
    vLayout3->addWidget(axisTitleOffsetSlider);
    vLayout3->addWidget(new QLabel(QStringLiteral("Radial label offset")));
    vLayout3->addWidget(radialLabelSlider, 0, Qt::AlignTop);
    vLayout3->addWidget(new QLabel(QStringLiteral("Camera target")), 0, Qt::AlignTop);
    vLayout3->addWidget(cameraTargetSliderX, 0, Qt::AlignTop);
    vLayout3->addWidget(cameraTargetSliderY, 0, Qt::AlignTop);
    vLayout3->addWidget(cameraTargetSliderZ, 0, Qt::AlignTop);
    vLayout3->addWidget(new QLabel(QStringLiteral("Adjust margin")), 0, Qt::AlignTop);
    vLayout3->addWidget(marginSlider, 1, Qt::AlignTop);

    vLayout3->addWidget(new QLabel(QStringLiteral("Adjust X-axis segments")), 0, Qt::AlignTop);
    vLayout3->addWidget(xSegmentSlider, 1, Qt::AlignTop);
    vLayout3->addWidget(xSubsegmentSlider, 1, Qt::AlignTop);
    vLayout3->addWidget(new QLabel(QStringLiteral("Adjust Y-axis segments")), 0, Qt::AlignTop);
    vLayout3->addWidget(ySegmentSlider, 1, Qt::AlignTop);
    vLayout3->addWidget(ySubsegmentSlider, 1, Qt::AlignTop);
    vLayout3->addWidget(new QLabel(QStringLiteral("Adjust Z-axis segments")), 0, Qt::AlignTop);
    vLayout3->addWidget(zSegmentSlider, 1, Qt::AlignTop);
    vLayout3->addWidget(zSubsegmentSlider, 1, Qt::AlignTop);

    ScatterDataModifier *modifier = new ScatterDataModifier(graph);

    QObject::connect(fontSizeSlider, &QSlider::valueChanged, modifier,
                     &ScatterDataModifier::changeFontSize);
    QObject::connect(pointSizeSlider,
                     &QSlider::valueChanged,
                     modifier,
                     &ScatterDataModifier::changePointSize);
    QObject::connect(labelSizeSlider,
                     &QSlider::valueChanged,
                     modifier,
                     &ScatterDataModifier::changeLabelSize);

    QObject::connect(styleButton, &QPushButton::clicked, modifier,
                     &ScatterDataModifier::changeStyle);
    QObject::connect(cameraButton, &QPushButton::clicked, modifier,
                     &ScatterDataModifier::changePresetCamera);
    QObject::connect(clearButton, &QPushButton::clicked, modifier,
                     &ScatterDataModifier::clear);
    QObject::connect(resetButton, &QPushButton::clicked, modifier,
                     &ScatterDataModifier::resetAxes);
    QObject::connect(addOneButton, &QPushButton::clicked, modifier,
                     &ScatterDataModifier::addOne);
    QObject::connect(addBunchButton, &QPushButton::clicked, modifier,
                     &ScatterDataModifier::addBunch);
    QObject::connect(insertOneButton, &QPushButton::clicked, modifier,
                     &ScatterDataModifier::insertOne);
    QObject::connect(insertBunchButton, &QPushButton::clicked, modifier,
                     &ScatterDataModifier::insertBunch);
    QObject::connect(changeOneButton, &QPushButton::clicked, modifier,
                     &ScatterDataModifier::changeOne);
    QObject::connect(changeBunchButton, &QPushButton::clicked, modifier,
                     &ScatterDataModifier::changeBunch);
    QObject::connect(removeOneButton, &QPushButton::clicked, modifier,
                     &ScatterDataModifier::removeOne);
    QObject::connect(removeBunchButton, &QPushButton::clicked, modifier,
                     &ScatterDataModifier::removeBunch);
    QObject::connect(setSelectedItemButton, &QPushButton::clicked, modifier,
                     &ScatterDataModifier::selectItem);
    QObject::connect(clearSeriesDataButton, &QPushButton::clicked, modifier,
                     &ScatterDataModifier::clearSeriesData);
    QObject::connect(addSeriesButton, &QPushButton::clicked, modifier,
                     &ScatterDataModifier::addSeries);
    QObject::connect(removeSeriesButton, &QPushButton::clicked, modifier,
                     &ScatterDataModifier::removeSeries);
    QObject::connect(toggleSeriesVisibilityButton,
                     &QPushButton::clicked,
                     modifier,
                     &ScatterDataModifier::toggleSeriesVisibility);
    QObject::connect(changeSeriesNameButton, &QPushButton::clicked, modifier,
                     &ScatterDataModifier::changeSeriesName);
    QObject::connect(startTimerButton, &QPushButton::clicked, modifier,
                     &ScatterDataModifier::startStopTimer);
    QObject::connect(massiveDataTestButton, &QPushButton::clicked, modifier,
                     &ScatterDataModifier::massiveDataTest);
    QObject::connect(testItemChangesButton, &QPushButton::clicked, modifier,
                     &ScatterDataModifier::testItemChanges);
    QObject::connect(testReverseButton, &QPushButton::clicked, modifier,
                     &ScatterDataModifier::testAxisReverse);
    QObject::connect(testNanSeriesButton, &QPushButton::clicked, modifier,
                     &ScatterDataModifier::testNanSeries);
    QObject::connect(renderToImageButton, &QPushButton::clicked, modifier,
                     &ScatterDataModifier::renderToImage);
    QObject::connect(gradientBtoYPB, &QPushButton::clicked, modifier,
                     &ScatterDataModifier::setGradient);
    QObject::connect(themeButton, &QPushButton::clicked, modifier,
                     &ScatterDataModifier::changeTheme);
    QObject::connect(labelButton, &QPushButton::clicked, modifier,
                     &ScatterDataModifier::changeLabelStyle);

    QObject::connect(shadowQuality, SIGNAL(currentIndexChanged(int)), modifier,
                     SLOT(changeShadowQuality(int)));
    QObject::connect(modifier,
                     &ScatterDataModifier::shadowQualityChanged,
                     shadowQuality,
                     &QComboBox::setCurrentIndex);
    QObject::connect(lightingMode, &QComboBox::currentIndexChanged, modifier,
                     &ScatterDataModifier::changeLightingMode);
    QObject::connect(fontList, &QFontComboBox::currentFontChanged, modifier,
                     &ScatterDataModifier::changeFont);

    QObject::connect(fpsCheckBox, &QCheckBox::checkStateChanged, modifier,
                     &ScatterDataModifier::setFpsMeasurement);
    QObject::connect(backgroundCheckBox,
                     &QCheckBox::checkStateChanged,
                     modifier,
                     &ScatterDataModifier::setBackgroundVisible);
    QObject::connect(gridCheckBox,
                     &QCheckBox::checkStateChanged,
                     modifier,
                     &ScatterDataModifier::setGridVisible);

    QObject::connect(minSliderX, &QSlider::valueChanged, modifier,
                     &ScatterDataModifier::setMinX);
    QObject::connect(minSliderY, &QSlider::valueChanged, modifier,
                     &ScatterDataModifier::setMinY);
    QObject::connect(minSliderZ, &QSlider::valueChanged, modifier,
                     &ScatterDataModifier::setMinZ);
    QObject::connect(maxSliderX, &QSlider::valueChanged, modifier,
                     &ScatterDataModifier::setMaxX);
    QObject::connect(maxSliderY, &QSlider::valueChanged, modifier,
                     &ScatterDataModifier::setMaxY);
    QObject::connect(maxSliderZ, &QSlider::valueChanged, modifier,
                     &ScatterDataModifier::setMaxZ);
    QObject::connect(cutoffMarginSlider, &QSlider::valueChanged, modifier,
                     &ScatterDataModifier::setCutoffMargin);
    QObject::connect(optimizationLegacyCB,
                     &QCheckBox::checkStateChanged,
                     modifier,
                     &ScatterDataModifier::toggleLegacy);
    QObject::connect(orthoCB, &QCheckBox::checkStateChanged, modifier,
                     &ScatterDataModifier::toggleOrtho);
    QObject::connect(polarCB, &QCheckBox::checkStateChanged, modifier,
                     &ScatterDataModifier::togglePolar);
    QObject::connect(axisTitlesVisibleCB, &QCheckBox::checkStateChanged, modifier,
                     &ScatterDataModifier::toggleAxisTitleVisibility);
    QObject::connect(axisTitlesFixedCB, &QCheckBox::checkStateChanged, modifier,
                     &ScatterDataModifier::toggleAxisTitleFixed);
    QObject::connect(labelAdjustCB,
                     &QCheckBox::checkStateChanged,
                     modifier,
                     &ScatterDataModifier::toggleLabelAdjustment);
    QObject::connect(axisLabelRotationSlider,
                     &QSlider::valueChanged,
                     modifier,
                     &ScatterDataModifier::changeLabelRotation);
    QObject::connect(axisTitleOffsetSlider,
                     &QSlider::valueChanged,
                     modifier,
                     &ScatterDataModifier::changeTitleOffset);
    QObject::connect(aspectRatioSlider,
                     &QSlider::valueChanged,
                     modifier,
                     &ScatterDataModifier::setAspectRatio);
    QObject::connect(horizontalAspectRatioSlider,
                     &QSlider::valueChanged,
                     modifier,
                     &ScatterDataModifier::setHorizontalAspectRatio);
    QObject::connect(radialLabelSlider, &QSlider::valueChanged, modifier,
                     &ScatterDataModifier::changeRadialLabelOffset);
    QObject::connect(cameraTargetSliderX, &QSlider::valueChanged, modifier,
                     &ScatterDataModifier::setCameraTargetX);
    QObject::connect(cameraTargetSliderY, &QSlider::valueChanged, modifier,
                     &ScatterDataModifier::setCameraTargetY);
    QObject::connect(cameraTargetSliderZ, &QSlider::valueChanged, modifier,
                     &ScatterDataModifier::setCameraTargetZ);
    QObject::connect(marginSlider, &QSlider::valueChanged, modifier,
                     &ScatterDataModifier::setGraphMargin);
    QObject::connect(xSegmentSlider,
                     &QSlider::valueChanged,
                     modifier,
                     &ScatterDataModifier::setXAxisSegemntCount);
    QObject::connect(ySegmentSlider,
                     &QSlider::valueChanged,
                     modifier,
                     &ScatterDataModifier::setYAxisSegemntCount);
    QObject::connect(zSegmentSlider,
                     &QSlider::valueChanged,
                     modifier,
                     &ScatterDataModifier::setZAxisSegemntCount);
    QObject::connect(xSubsegmentSlider,
                     &QSlider::valueChanged,
                     modifier,
                     &ScatterDataModifier::setXAxisSubsegemntCount);
    QObject::connect(ySubsegmentSlider,
                     &QSlider::valueChanged,
                     modifier,
                     &ScatterDataModifier::setYAxisSubsegemntCount);
    QObject::connect(zSubsegmentSlider,
                     &QSlider::valueChanged,
                     modifier,
                     &ScatterDataModifier::setZAxisSubsegemntCount);

    modifier->setFpsLabel(fpsLabel);

    graph->widget()->setGeometry(QRect(0, 0, 800, 800));

    modifier->start();
    //modifier->renderToImage(); // Initial hidden render

    widget->show();
    int retVal = app.exec();
    delete modifier;
    delete quickWidget;
    return retVal;
}
