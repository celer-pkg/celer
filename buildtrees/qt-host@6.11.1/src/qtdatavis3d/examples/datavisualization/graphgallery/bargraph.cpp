// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "bargraph.h"
#include "graphmodifier.h"

#include <QtWidgets/qboxlayout.h>
#include <QtWidgets/qpushbutton.h>
#include <QtWidgets/qcheckbox.h>
#include <QtWidgets/qslider.h>
#include <QtWidgets/qfontcombobox.h>
#include <QtWidgets/qlabel.h>
#include <QtWidgets/qradiobutton.h>
#include <QtWidgets/qbuttongroup.h>
#include <QtGui/qfontdatabase.h>

using namespace Qt::StringLiterals;

BarGraph::BarGraph()
{
    //! [0]
    m_barsGraph = new Q3DBars();
    //! [0]
}

BarGraph::~BarGraph() = default;

bool BarGraph::initialize(const QSize &minimumGraphSize, const QSize &maximumGraphSize)
{
    if (!m_barsGraph->hasContext())
        return false;

    //! [1]
    m_barsWidget = new QWidget;
    auto *hLayout = new QHBoxLayout(m_barsWidget);
    m_container = QWidget::createWindowContainer(m_barsGraph, m_barsWidget);
    m_barsGraph->resize(minimumGraphSize);
    m_container->setMinimumSize(minimumGraphSize);
    m_container->setMaximumSize(maximumGraphSize);
    m_container->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_container->setFocusPolicy(Qt::StrongFocus);
    hLayout->addWidget(m_container, 1);

    auto *vLayout = new QVBoxLayout();
    hLayout->addLayout(vLayout);
    //! [1]

    auto *themeList = new QComboBox(m_barsWidget);
    themeList->addItem(u"Qt"_s);
    themeList->addItem(u"Primary Colors"_s);
    themeList->addItem(u"Digia"_s);
    themeList->addItem(u"Stone Moss"_s);
    themeList->addItem(u"Army Blue"_s);
    themeList->addItem(u"Retro"_s);
    themeList->addItem(u"Ebony"_s);
    themeList->addItem(u"Isabelle"_s);
    themeList->setCurrentIndex(0);

    auto *labelButton = new QPushButton(m_barsWidget);
    labelButton->setText(u"Change label style"_s);

    auto *smoothCheckBox = new QCheckBox(m_barsWidget);
    smoothCheckBox->setText(u"Smooth bars"_s);
    smoothCheckBox->setChecked(false);

    auto *barStyleList = new QComboBox(m_barsWidget);
    barStyleList->addItem(u"Bar"_s, QAbstract3DSeries::MeshBar);
    barStyleList->addItem(u"Pyramid"_s, QAbstract3DSeries::MeshPyramid);
    barStyleList->addItem(u"Cone"_s, QAbstract3DSeries::MeshCone);
    barStyleList->addItem(u"Cylinder"_s, QAbstract3DSeries::MeshCylinder);
    barStyleList->addItem(u"Bevel bar"_s, QAbstract3DSeries::MeshBevelBar);
    barStyleList->addItem(u"Sphere"_s, QAbstract3DSeries::MeshSphere);
    barStyleList->setCurrentIndex(4);

    auto *cameraButton = new QPushButton(m_barsWidget);
    cameraButton->setText(u"Change camera preset"_s);

    auto *zoomToSelectedButton = new QPushButton(m_barsWidget);
    zoomToSelectedButton->setText(u"Zoom to selected bar"_s);

    auto *selectionModeList = new QComboBox(m_barsWidget);
    selectionModeList->addItem(u"None"_s,
                               int(QAbstract3DGraph::SelectionNone));
    selectionModeList->addItem(u"Bar"_s,
                               int(QAbstract3DGraph::SelectionItem));
    selectionModeList->addItem(u"Row"_s,
                               int(QAbstract3DGraph::SelectionRow));
    selectionModeList->addItem(u"Bar and Row"_s,
                               int(QAbstract3DGraph::SelectionItemAndRow));
    selectionModeList->addItem(u"Column"_s,
                               int(QAbstract3DGraph::SelectionColumn));
    selectionModeList->addItem(u"Bar and Column"_s,
                               int(QAbstract3DGraph::SelectionItemAndColumn));
    selectionModeList->addItem(u"Row and Column"_s,
                               int(QAbstract3DGraph::SelectionRowAndColumn));
    selectionModeList->addItem(u"Bar, Row and Column"_s,
                               int(QAbstract3DGraph::SelectionItemRowAndColumn));
    selectionModeList->addItem(u"Slice into Row"_s,
                               int(QAbstract3DGraph::SelectionSlice | QAbstract3DGraph::SelectionRow));
    selectionModeList->addItem(u"Slice into Row and Item"_s,
                               int(QAbstract3DGraph::SelectionSlice | QAbstract3DGraph::SelectionItemAndRow));
    selectionModeList->addItem(u"Slice into Column"_s,
                               int(QAbstract3DGraph::SelectionSlice | QAbstract3DGraph::SelectionColumn));
    selectionModeList->addItem(u"Slice into Column and Item"_s,
                               int(QAbstract3DGraph::SelectionSlice | QAbstract3DGraph::SelectionItemAndColumn));
    selectionModeList->addItem(u"Multi: Bar, Row, Col"_s,
                               int(QAbstract3DGraph::SelectionItemRowAndColumn
                                   | QAbstract3DGraph::SelectionMultiSeries));
    selectionModeList->addItem(u"Multi, Slice: Row, Item"_s,
                               int(QAbstract3DGraph::SelectionSlice | QAbstract3DGraph::SelectionItemAndRow
                                   | QAbstract3DGraph::SelectionMultiSeries));
    selectionModeList->addItem(u"Multi, Slice: Col, Item"_s,
                               int(QAbstract3DGraph::SelectionSlice | QAbstract3DGraph::SelectionItemAndColumn
                                   | QAbstract3DGraph::SelectionMultiSeries));
    selectionModeList->setCurrentIndex(1);

    auto *backgroundCheckBox = new QCheckBox(m_barsWidget);
    backgroundCheckBox->setText(u"Show background"_s);
    backgroundCheckBox->setChecked(false);

    auto *gridCheckBox = new QCheckBox(m_barsWidget);
    gridCheckBox->setText(u"Show grid"_s);
    gridCheckBox->setChecked(true);

    auto *seriesCheckBox = new QCheckBox(m_barsWidget);
    seriesCheckBox->setText(u"Show second series"_s);
    seriesCheckBox->setChecked(false);

    auto *reverseValueAxisCheckBox = new QCheckBox(m_barsWidget);
    reverseValueAxisCheckBox->setText(u"Reverse value axis"_s);
    reverseValueAxisCheckBox->setChecked(false);

    auto *reflectionCheckBox = new QCheckBox(m_barsWidget);
    reflectionCheckBox->setText(u"Show reflections"_s);
    reflectionCheckBox->setChecked(false);

    //! [3]
    auto *rotationSliderX = new QSlider(Qt::Horizontal, m_barsWidget);
    rotationSliderX->setTickInterval(30);
    rotationSliderX->setTickPosition(QSlider::TicksBelow);
    rotationSliderX->setMinimum(-180);
    rotationSliderX->setValue(0);
    rotationSliderX->setMaximum(180);
    //! [3]
    auto *rotationSliderY = new QSlider(Qt::Horizontal, m_barsWidget);
    rotationSliderY->setTickInterval(15);
    rotationSliderY->setTickPosition(QSlider::TicksAbove);
    rotationSliderY->setMinimum(-90);
    rotationSliderY->setValue(0);
    rotationSliderY->setMaximum(90);

    auto *fontSizeSlider = new QSlider(Qt::Horizontal, m_barsWidget);
    fontSizeSlider->setTickInterval(10);
    fontSizeSlider->setTickPosition(QSlider::TicksBelow);
    fontSizeSlider->setMinimum(1);
    fontSizeSlider->setValue(30);
    fontSizeSlider->setMaximum(100);

    auto *fontList = new QFontComboBox(m_barsWidget);
    fontList->setCurrentFont(QFont("Times New Roman"));

    auto *shadowQuality = new QComboBox(m_barsWidget);
    shadowQuality->addItem(u"None"_s);
    shadowQuality->addItem(u"Low"_s);
    shadowQuality->addItem(u"Medium"_s);
    shadowQuality->addItem(u"High"_s);
    shadowQuality->addItem(u"Low Soft"_s);
    shadowQuality->addItem(u"Medium Soft"_s);
    shadowQuality->addItem(u"High Soft"_s);
    shadowQuality->setCurrentIndex(5);

    auto *rangeList = new QComboBox(m_barsWidget);
    rangeList->addItem(u"2015"_s);
    rangeList->addItem(u"2016"_s);
    rangeList->addItem(u"2017"_s);
    rangeList->addItem(u"2018"_s);
    rangeList->addItem(u"2019"_s);
    rangeList->addItem(u"2020"_s);
    rangeList->addItem(u"2021"_s);
    rangeList->addItem(u"2022"_s);
    rangeList->addItem(u"All"_s);
    rangeList->setCurrentIndex(8);

    auto *axisTitlesVisibleCB = new QCheckBox(m_barsWidget);
    axisTitlesVisibleCB->setText(u"Axis titles visible"_s);
    axisTitlesVisibleCB->setChecked(true);

    auto *axisTitlesFixedCB = new QCheckBox(m_barsWidget);
    axisTitlesFixedCB->setText(u"Axis titles fixed"_s);
    axisTitlesFixedCB->setChecked(true);

    auto *axisLabelRotationSlider = new QSlider(Qt::Horizontal, m_barsWidget);
    axisLabelRotationSlider->setTickInterval(10);
    axisLabelRotationSlider->setTickPosition(QSlider::TicksBelow);
    axisLabelRotationSlider->setMinimum(0);
    axisLabelRotationSlider->setValue(30);
    axisLabelRotationSlider->setMaximum(90);

    auto *modeGroup = new QButtonGroup(m_barsWidget);
    auto *modeWeather = new QRadioButton(u"Temperature Data"_s, m_barsWidget);
    modeWeather->setChecked(true);
    auto *modeCustomProxy = new QRadioButton(u"Custom Proxy Data"_s, m_barsWidget);
    modeGroup->addButton(modeWeather);
    modeGroup->addButton(modeCustomProxy);

    //! [4]
    vLayout->addWidget(new QLabel(u"Rotate horizontally"_s));
    vLayout->addWidget(rotationSliderX, 0, Qt::AlignTop);
    //! [4]
    vLayout->addWidget(new QLabel(u"Rotate vertically"_s));
    vLayout->addWidget(rotationSliderY, 0, Qt::AlignTop);
    vLayout->addWidget(labelButton, 0, Qt::AlignTop);
    vLayout->addWidget(cameraButton, 0, Qt::AlignTop);
    vLayout->addWidget(zoomToSelectedButton, 0, Qt::AlignTop);
    vLayout->addWidget(backgroundCheckBox);
    vLayout->addWidget(gridCheckBox);
    vLayout->addWidget(smoothCheckBox);
    vLayout->addWidget(reflectionCheckBox);
    vLayout->addWidget(seriesCheckBox);
    vLayout->addWidget(reverseValueAxisCheckBox);
    vLayout->addWidget(axisTitlesVisibleCB);
    vLayout->addWidget(axisTitlesFixedCB);
    vLayout->addWidget(new QLabel(u"Show year"_s));
    vLayout->addWidget(rangeList);
    vLayout->addWidget(new QLabel(u"Change bar style"_s));
    vLayout->addWidget(barStyleList);
    vLayout->addWidget(new QLabel(u"Change selection mode"_s));
    vLayout->addWidget(selectionModeList);
    vLayout->addWidget(new QLabel(u"Change theme"_s));
    vLayout->addWidget(themeList);
    vLayout->addWidget(new QLabel(u"Adjust shadow quality"_s));
    vLayout->addWidget(shadowQuality);
    vLayout->addWidget(new QLabel(u"Change font"_s));
    vLayout->addWidget(fontList);
    vLayout->addWidget(new QLabel(u"Adjust font size"_s));
    vLayout->addWidget(fontSizeSlider);
    vLayout->addWidget(new QLabel(u"Axis label rotation"_s));
    vLayout->addWidget(axisLabelRotationSlider, 0, Qt::AlignTop);
    vLayout->addWidget(modeWeather, 0, Qt::AlignTop);
    vLayout->addWidget(modeCustomProxy, 1, Qt::AlignTop);

    //! [2]
    auto *modifier = new GraphModifier(m_barsGraph, this);
    //! [2]

    //! [5]
    QObject::connect(rotationSliderX, &QSlider::valueChanged, modifier, &GraphModifier::rotateX);
    //! [5]
    QObject::connect(rotationSliderY, &QSlider::valueChanged, modifier, &GraphModifier::rotateY);

    QObject::connect(labelButton, &QPushButton::clicked, modifier,
                     &GraphModifier::changeLabelBackground);
    QObject::connect(cameraButton, &QPushButton::clicked, modifier,
                     &GraphModifier::changePresetCamera);
    QObject::connect(zoomToSelectedButton, &QPushButton::clicked, modifier,
                     &GraphModifier::zoomToSelectedBar);

    QObject::connect(backgroundCheckBox, &QCheckBox::stateChanged, modifier,
                     &GraphModifier::setBackgroundEnabled);
    QObject::connect(gridCheckBox, &QCheckBox::stateChanged, modifier,
                     &GraphModifier::setGridEnabled);
    QObject::connect(smoothCheckBox, &QCheckBox::stateChanged, modifier,
                     &GraphModifier::setSmoothBars);
    QObject::connect(seriesCheckBox, &QCheckBox::stateChanged, modifier,
                     &GraphModifier::setSeriesVisibility);
    QObject::connect(reverseValueAxisCheckBox, &QCheckBox::stateChanged, modifier,
                     &GraphModifier::setReverseValueAxis);
    QObject::connect(reflectionCheckBox, &QCheckBox::stateChanged, modifier,
                     &GraphModifier::setReflection);

    QObject::connect(modifier, &GraphModifier::backgroundEnabledChanged,
                     backgroundCheckBox, &QCheckBox::setChecked);
    QObject::connect(modifier, &GraphModifier::gridEnabledChanged,
                     gridCheckBox, &QCheckBox::setChecked);

    QObject::connect(rangeList, &QComboBox::currentIndexChanged, modifier,
                     &GraphModifier::changeRange);

    QObject::connect(barStyleList, &QComboBox::currentIndexChanged, modifier,
                     &GraphModifier::changeStyle);

    QObject::connect(selectionModeList, &QComboBox::currentIndexChanged, modifier,
                     &GraphModifier::changeSelectionMode);

    QObject::connect(themeList, &QComboBox::currentIndexChanged, modifier,
                     &GraphModifier::changeTheme);

    QObject::connect(shadowQuality, &QComboBox::currentIndexChanged, modifier,
                     &GraphModifier::changeShadowQuality);

    QObject::connect(modifier, &GraphModifier::shadowQualityChanged, shadowQuality,
                     &QComboBox::setCurrentIndex);
    QObject::connect(m_barsGraph, &Q3DBars::shadowQualityChanged, modifier,
                     &GraphModifier::shadowQualityUpdatedByVisual);

    QObject::connect(fontSizeSlider, &QSlider::valueChanged, modifier,
                     &GraphModifier::changeFontSize);
    QObject::connect(fontList, &QFontComboBox::currentFontChanged, modifier,
                     &GraphModifier::changeFont);

    QObject::connect(modifier, &GraphModifier::fontSizeChanged, fontSizeSlider,
                     &QSlider::setValue);
    QObject::connect(modifier, &GraphModifier::fontChanged, fontList,
                     &QFontComboBox::setCurrentFont);

    QObject::connect(axisTitlesVisibleCB, &QCheckBox::stateChanged, modifier,
                     &GraphModifier::setAxisTitleVisibility);
    QObject::connect(axisTitlesFixedCB, &QCheckBox::stateChanged, modifier,
                     &GraphModifier::setAxisTitleFixed);
    QObject::connect(axisLabelRotationSlider, &QSlider::valueChanged, modifier,
                     &GraphModifier::changeLabelRotation);

    QObject::connect(modeWeather, &QRadioButton::toggled, modifier,
                     &GraphModifier::setDataModeToWeather);
    QObject::connect(modeCustomProxy, &QRadioButton::toggled, modifier,
                     &GraphModifier::setDataModeToCustom);
    QObject::connect(modeWeather, &QRadioButton::toggled, seriesCheckBox,
                     &QCheckBox::setEnabled);
    QObject::connect(modeWeather, &QRadioButton::toggled, rangeList,
                     &QComboBox::setEnabled);
    QObject::connect(modeWeather, &QRadioButton::toggled, axisTitlesVisibleCB,
                     &QCheckBox::setEnabled);
    QObject::connect(modeWeather, &QRadioButton::toggled, axisTitlesFixedCB,
                     &QCheckBox::setEnabled);
    QObject::connect(modeWeather, &QRadioButton::toggled, axisLabelRotationSlider,
                     &QSlider::setEnabled);

    return true;
}
