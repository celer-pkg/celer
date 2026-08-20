// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "bargraph.h"

#include <QtGui/qfontdatabase.h>
#include <QtWidgets/qboxlayout.h>
#include <QtWidgets/qbuttongroup.h>
#include <QtWidgets/qcheckbox.h>
#include <QtWidgets/qfontcombobox.h>
#include <QtWidgets/qlabel.h>
#include <QtWidgets/qpushbutton.h>
#include <QtWidgets/qradiobutton.h>
#include <QtWidgets/qslider.h>

using namespace Qt::StringLiterals;

BarGraph::BarGraph(QWidget *parent)
{
    Q_UNUSED(parent)
    //! [creation]
    m_quickWidget = new QQuickWidget();
    m_barGraph = new Q3DBarsWidgetItem(this);
    m_barGraph->setWidget(m_quickWidget);
    //! [creation]
    initialize();
}

void BarGraph::initialize()
{
    //! [adding to layout]
    m_container = new QWidget();
    auto *hLayout = new QHBoxLayout(m_container);
    QSize screenSize = m_quickWidget->screen()->size();
    m_quickWidget->setMinimumSize(QSize(screenSize.width() / 2, screenSize.height() / 1.75));
    m_quickWidget->setMaximumSize(screenSize);
    m_quickWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_quickWidget->setFocusPolicy(Qt::StrongFocus);
    hLayout->addWidget(m_quickWidget, 1);

    auto *vLayout = new QVBoxLayout();
    hLayout->addLayout(vLayout);
    //! [adding to layout]

    auto *themeList = new QComboBox(m_container);
    themeList->addItem(u"QtGreen"_s);
    themeList->addItem(u"QtGreenNeon"_s);
    themeList->addItem(u"MixSeries"_s);
    themeList->addItem(u"OrangeSeries"_s);
    themeList->addItem(u"YellowSeries"_s);
    themeList->addItem(u"BlueSeries"_s);
    themeList->addItem(u"PurpleSeries"_s);
    themeList->addItem(u"GreySeries"_s);
    themeList->setCurrentIndex(0);

    auto *labelButton = new QPushButton(m_container);
    labelButton->setText(u"Change label style"_s);

    auto *smoothCheckBox = new QCheckBox(m_container);
    smoothCheckBox->setText(u"Smooth bars"_s);
    smoothCheckBox->setChecked(false);

    auto *barStyleList = new QComboBox(m_container);
    const QMetaObject &metaObj = QAbstract3DSeries::staticMetaObject;
    int index = metaObj.indexOfEnumerator("Mesh");
    QMetaEnum metaEnum = metaObj.enumerator(index);
    barStyleList->addItem(u"Bar"_s, metaEnum.value(static_cast<int>(QAbstract3DSeries::Mesh::Bar)));
    barStyleList->addItem(u"Pyramid"_s,
                          metaEnum.value(static_cast<int>(QAbstract3DSeries::Mesh::Pyramid)));
    barStyleList->addItem(u"Cone"_s,
                          metaEnum.value(static_cast<int>(QAbstract3DSeries::Mesh::Cone)));
    barStyleList->addItem(u"Cylinder"_s,
                          metaEnum.value(static_cast<int>(QAbstract3DSeries::Mesh::Cylinder)));
    barStyleList->addItem(u"Bevel bar"_s,
                          metaEnum.value(static_cast<int>(QAbstract3DSeries::Mesh::BevelBar)));
    barStyleList->addItem(u"Sphere"_s,
                          metaEnum.value(static_cast<int>(QAbstract3DSeries::Mesh::Sphere)));
    barStyleList->addItem(u"UserDefined"_s,
                          metaEnum.value(static_cast<int>(QAbstract3DSeries::Mesh::UserDefined)));
    barStyleList->setCurrentIndex(4);

    auto *cameraButton = new QPushButton(m_container);
    cameraButton->setText(u"Change camera preset"_s);

    auto *zoomToSelectedButton = new QPushButton(m_container);
    zoomToSelectedButton->setText(u"Zoom to selected bar"_s);

    auto *selectionModeList = new QComboBox(m_container);
    selectionModeList->addItem(u"None"_s, int(QtGraphs3D::SelectionFlag::None));
    selectionModeList->addItem(u"Bar"_s, int(QtGraphs3D::SelectionFlag::Item));
    selectionModeList->addItem(u"Row"_s, int(QtGraphs3D::SelectionFlag::Row));
    selectionModeList->addItem(u"Bar and Row"_s, int(QtGraphs3D::SelectionFlag::ItemAndRow));
    selectionModeList->addItem(u"Column"_s, int(QtGraphs3D::SelectionFlag::Column));
    selectionModeList->addItem(u"Bar and Column"_s, int(QtGraphs3D::SelectionFlag::ItemAndColumn));
    selectionModeList->addItem(u"Row and Column"_s, int(QtGraphs3D::SelectionFlag::RowAndColumn));
    selectionModeList->addItem(u"Bar, Row and Column"_s,
                               int(QtGraphs3D::SelectionFlag::ItemRowAndColumn));
    selectionModeList->addItem(u"Slice into Row"_s,
                               int(QtGraphs3D::SelectionFlag::Slice
                                   | QtGraphs3D::SelectionFlag::Row));
    selectionModeList->addItem(u"Slice into Row and Item"_s,
                               int(QtGraphs3D::SelectionFlag::Slice
                                   | QtGraphs3D::SelectionFlag::ItemAndRow));
    selectionModeList->addItem(u"Slice into Column"_s,
                               int(QtGraphs3D::SelectionFlag::Slice
                                   | QtGraphs3D::SelectionFlag::Column));
    selectionModeList->addItem(u"Slice into Column and Item"_s,
                               int(QtGraphs3D::SelectionFlag::Slice
                                   | QtGraphs3D::SelectionFlag::ItemAndColumn));
    selectionModeList->addItem(u"Multi: Bar, Row, Col"_s,
                               int(QtGraphs3D::SelectionFlag::ItemRowAndColumn
                                   | QtGraphs3D::SelectionFlag::MultiSeries));
    selectionModeList->addItem(u"Multi, Slice: Row, Item"_s,
                               int(QtGraphs3D::SelectionFlag::Slice
                                   | QtGraphs3D::SelectionFlag::ItemAndRow
                                   | QtGraphs3D::SelectionFlag::MultiSeries));
    selectionModeList->addItem(u"Multi, Slice: Col, Item"_s,
                               int(QtGraphs3D::SelectionFlag::Slice
                                   | QtGraphs3D::SelectionFlag::ItemAndColumn
                                   | QtGraphs3D::SelectionFlag::MultiSeries));
    selectionModeList->setCurrentIndex(1);

    auto *backgroundCheckBox = new QCheckBox(m_container);
    backgroundCheckBox->setText(u"Show graph background"_s);
    backgroundCheckBox->setChecked(false);

    auto *gridCheckBox = new QCheckBox(m_container);
    gridCheckBox->setText(u"Show grid"_s);
    gridCheckBox->setChecked(true);

    auto *seriesCheckBox = new QCheckBox(m_container);
    seriesCheckBox->setText(u"Show second series"_s);
    seriesCheckBox->setChecked(false);

    auto *reverseValueAxisCheckBox = new QCheckBox(m_container);
    reverseValueAxisCheckBox->setText(u"Reverse value axis"_s);
    reverseValueAxisCheckBox->setChecked(false);

    //! [creating a slider]
    auto *rotationSliderX = new QSlider(Qt::Horizontal, m_container);
    rotationSliderX->setTickInterval(30);
    rotationSliderX->setTickPosition(QSlider::TicksBelow);
    rotationSliderX->setMinimum(-180);
    rotationSliderX->setValue(0);
    rotationSliderX->setMaximum(180);
    //! [creating a slider]
    auto *rotationSliderY = new QSlider(Qt::Horizontal, m_container);
    rotationSliderY->setTickInterval(15);
    rotationSliderY->setTickPosition(QSlider::TicksAbove);
    rotationSliderY->setMinimum(-90);
    rotationSliderY->setValue(0);
    rotationSliderY->setMaximum(90);

    auto *fontSizeSlider = new QSlider(Qt::Horizontal, m_container);
    fontSizeSlider->setTickInterval(10);
    fontSizeSlider->setTickPosition(QSlider::TicksBelow);
    fontSizeSlider->setMinimum(1);
    fontSizeSlider->setValue(30);
    fontSizeSlider->setMaximum(100);

    auto *fontList = new QFontComboBox(m_container);
    fontList->setCurrentFont(QFont("Times New Roman"));

    auto *shadowQuality = new QComboBox(m_container);
    shadowQuality->addItem(u"None"_s);
    shadowQuality->addItem(u"Low"_s);
    shadowQuality->addItem(u"Medium"_s);
    shadowQuality->addItem(u"High"_s);
    shadowQuality->addItem(u"Low Soft"_s);
    shadowQuality->addItem(u"Medium Soft"_s);
    shadowQuality->addItem(u"High Soft"_s);
    shadowQuality->setCurrentIndex(5);

    auto *rangeList = new QComboBox(m_container);
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

    auto *axisTitlesVisibleCB = new QCheckBox(m_container);
    axisTitlesVisibleCB->setText(u"Axis titles visible"_s);
    axisTitlesVisibleCB->setChecked(true);

    auto *axisTitlesFixedCB = new QCheckBox(m_container);
    axisTitlesFixedCB->setText(u"Axis titles fixed"_s);
    axisTitlesFixedCB->setChecked(true);

    auto *axisLabelRotationSlider = new QSlider(Qt::Horizontal, m_container);
    axisLabelRotationSlider->setTickInterval(10);
    axisLabelRotationSlider->setTickPosition(QSlider::TicksBelow);
    axisLabelRotationSlider->setMinimum(0);
    axisLabelRotationSlider->setValue(30);
    axisLabelRotationSlider->setMaximum(90);

    auto *modeGroup = new QButtonGroup(m_container);
    auto *modeWeather = new QRadioButton(u"Temperature Data"_s, m_container);
    modeWeather->setChecked(true);
    auto *modelProxy = new QRadioButton(u"Model Proxy Data"_s, m_container);
    modeGroup->addButton(modeWeather);
    modeGroup->addButton(modelProxy);

    //! [adding the slider]
    vLayout->addWidget(new QLabel(u"Rotate horizontally"_s));
    vLayout->addWidget(rotationSliderX, 0, Qt::AlignTop);
    //! [adding the slider]
    vLayout->addWidget(new QLabel(u"Rotate vertically"_s));
    vLayout->addWidget(rotationSliderY, 0, Qt::AlignTop);
    vLayout->addWidget(labelButton, 0, Qt::AlignTop);
    vLayout->addWidget(cameraButton, 0, Qt::AlignTop);
    vLayout->addWidget(zoomToSelectedButton, 0, Qt::AlignTop);
    vLayout->addWidget(backgroundCheckBox);
    vLayout->addWidget(gridCheckBox);
    vLayout->addWidget(smoothCheckBox);
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
    vLayout->addWidget(modelProxy, 1, Qt::AlignTop);

    // Raise the graph to the top of the widget stack, to hide UI if resized smaller
    m_quickWidget->raise();

    //! [create the data handler]
    m_modifier = new GraphModifier(m_barGraph, this);
    //! [create the data handler]
    m_modifier->changeTheme(themeList->currentIndex());

    //! [connecting the slider]
    QObject::connect(rotationSliderX, &QSlider::valueChanged, m_modifier, &GraphModifier::rotateX);
    //! [connecting the slider]
    QObject::connect(rotationSliderY, &QSlider::valueChanged, m_modifier, &GraphModifier::rotateY);

    QObject::connect(labelButton,
                     &QPushButton::clicked,
                     m_modifier,
                     &GraphModifier::changeLabelBackground);
    QObject::connect(cameraButton,
                     &QPushButton::clicked,
                     m_modifier,
                     &GraphModifier::changePresetCamera);
    QObject::connect(zoomToSelectedButton,
                     &QPushButton::clicked,
                     m_modifier,
                     &GraphModifier::zoomToSelectedBar);

    QObject::connect(backgroundCheckBox,
                     &QCheckBox::checkStateChanged,
                     m_modifier,
                     &GraphModifier::setBackgroundVisible);
    QObject::connect(gridCheckBox,
                     &QCheckBox::checkStateChanged,
                     m_modifier,
                     &GraphModifier::setGridVisible);
    QObject::connect(smoothCheckBox,
                     &QCheckBox::checkStateChanged,
                     m_modifier,
                     &GraphModifier::setSmoothBars);
    QObject::connect(seriesCheckBox,
                     &QCheckBox::checkStateChanged,
                     m_modifier,
                     &GraphModifier::setSeriesVisibility);
    QObject::connect(reverseValueAxisCheckBox,
                     &QCheckBox::checkStateChanged,
                     m_modifier,
                     &GraphModifier::setReverseValueAxis);

    QObject::connect(m_modifier,
                     &GraphModifier::backgroundVisibleChanged,
                     backgroundCheckBox,
                     &QCheckBox::setChecked);
    QObject::connect(m_modifier,
                     &GraphModifier::gridVisibleChanged,
                     gridCheckBox,
                     &QCheckBox::setChecked);

    QObject::connect(rangeList,
                     &QComboBox::currentIndexChanged,
                     m_modifier,
                     &GraphModifier::changeRange);

    QObject::connect(barStyleList,
                     &QComboBox::currentIndexChanged,
                     m_modifier,
                     &GraphModifier::changeStyle);

    QObject::connect(selectionModeList,
                     &QComboBox::currentIndexChanged,
                     m_modifier,
                     &GraphModifier::changeSelectionMode);

    QObject::connect(themeList,
                     &QComboBox::currentIndexChanged,
                     m_modifier,
                     &GraphModifier::changeTheme);

    QObject::connect(shadowQuality,
                     &QComboBox::currentIndexChanged,
                     m_modifier,
                     &GraphModifier::changeShadowQuality);

    QObject::connect(m_modifier,
                     &GraphModifier::shadowQualityChanged,
                     shadowQuality,
                     &QComboBox::setCurrentIndex);
    QObject::connect(m_barGraph,
                     &Q3DBarsWidgetItem::shadowQualityChanged,
                     m_modifier,
                     &GraphModifier::shadowQualityUpdatedByVisual);

    QObject::connect(fontSizeSlider,
                     &QSlider::valueChanged,
                     m_modifier,
                     &GraphModifier::changeFontSize);
    QObject::connect(fontList,
                     &QFontComboBox::currentFontChanged,
                     m_modifier,
                     &GraphModifier::changeFont);

    QObject::connect(m_modifier,
                     &GraphModifier::fontSizeChanged,
                     fontSizeSlider,
                     &QSlider::setValue);
    QObject::connect(m_modifier,
                     &GraphModifier::fontChanged,
                     fontList,
                     &QFontComboBox::setCurrentFont);

    QObject::connect(axisTitlesVisibleCB,
                     &QCheckBox::checkStateChanged,
                     m_modifier,
                     &GraphModifier::setAxisTitleVisibility);
    QObject::connect(axisTitlesFixedCB,
                     &QCheckBox::checkStateChanged,
                     m_modifier,
                     &GraphModifier::setAxisTitleFixed);
    QObject::connect(axisLabelRotationSlider,
                     &QSlider::valueChanged,
                     m_modifier,
                     &GraphModifier::changeLabelRotation);

    QObject::connect(modeWeather,
                     &QRadioButton::toggled,
                     m_modifier,
                     &GraphModifier::setDataModeToWeather);
    QObject::connect(modelProxy,
                     &QRadioButton::toggled,
                     m_modifier,
                     &GraphModifier::setDataModeToModel);
    QObject::connect(modeWeather, &QRadioButton::toggled, seriesCheckBox, &QCheckBox::setEnabled);
    QObject::connect(modeWeather, &QRadioButton::toggled, rangeList, &QComboBox::setEnabled);
    QObject::connect(modeWeather,
                     &QRadioButton::toggled,
                     axisTitlesVisibleCB,
                     &QCheckBox::setEnabled);
    QObject::connect(modeWeather, &QRadioButton::toggled, axisTitlesFixedCB, &QCheckBox::setEnabled);
    QObject::connect(modeWeather,
                     &QRadioButton::toggled,
                     axisLabelRotationSlider,
                     &QSlider::setEnabled);
}
