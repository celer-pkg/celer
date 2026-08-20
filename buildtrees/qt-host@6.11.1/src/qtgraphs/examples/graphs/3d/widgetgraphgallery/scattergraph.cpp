// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "scattergraph.h"

#include <QtWidgets/qboxlayout.h>
#include <QtWidgets/qcheckbox.h>
#include <QtWidgets/qcombobox.h>
#include <QtWidgets/qcommandlinkbutton.h>
#include <QtWidgets/qlabel.h>

using namespace Qt::StringLiterals;

ScatterGraph::ScatterGraph(QWidget *parent)
{
    m_scatterWidget = new QWidget(parent);
    initialize();
}

void ScatterGraph::initialize()
{
    m_scatterGraphWidget = new ScatterGraphWidget();
    m_scatterGraphWidget->initialize();
    auto *hLayout = new QHBoxLayout(m_scatterWidget);
    QSize screenSize = m_scatterGraphWidget->screen()->size();
    m_scatterGraphWidget->setMinimumSize(QSize(screenSize.width() / 2, screenSize.height() / 1.75));
    m_scatterGraphWidget->setMaximumSize(screenSize);
    m_scatterGraphWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_scatterGraphWidget->setFocusPolicy(Qt::StrongFocus);
    hLayout->addWidget(m_scatterGraphWidget, 1);

    auto *vLayout = new QVBoxLayout();
    hLayout->addLayout(vLayout);

    auto *cameraButton = new QCommandLinkButton(m_scatterWidget);
    cameraButton->setText(u"Change camera preset"_s);
    cameraButton->setDescription(u"Switch between a number of preset camera positions"_s);
    cameraButton->setIconSize(QSize(0, 0));

    auto *itemCountButton = new QCommandLinkButton(m_scatterWidget);
    itemCountButton->setText(u"Toggle item count"_s);
    itemCountButton->setDescription(u"Switch between 900 and 10000 data points"_s);
    itemCountButton->setIconSize(QSize(0, 0));

    auto *rangeButton = new QCommandLinkButton(m_scatterWidget);
    rangeButton->setText(u"Toggle axis ranges"_s);
    rangeButton->setDescription(u"Switch between automatic axis ranges and preset ranges"_s);
    rangeButton->setIconSize(QSize(0, 0));

    auto *rangeMinSlider = new QSlider(m_scatterWidget);
    rangeMinSlider->setOrientation(Qt::Horizontal);
    rangeMinSlider->setMinimum(-10);
    rangeMinSlider->setMaximum(1);
    rangeMinSlider->setValue(-10);

    auto *rangeMaxSlider = new QSlider(m_scatterWidget);
    rangeMaxSlider->setOrientation(Qt::Horizontal);
    rangeMaxSlider->setMinimum(1);
    rangeMaxSlider->setMaximum(10);
    rangeMaxSlider->setValue(10);

    auto *backgroundCheckBox = new QCheckBox(m_scatterWidget);
    backgroundCheckBox->setText(u"Show graph background"_s);
    backgroundCheckBox->setChecked(true);

    auto *gridCheckBox = new QCheckBox(m_scatterWidget);
    gridCheckBox->setText(u"Show grid"_s);
    gridCheckBox->setChecked(true);

    auto *smoothCheckBox = new QCheckBox(m_scatterWidget);
    smoothCheckBox->setText(u"Smooth dots"_s);
    smoothCheckBox->setChecked(true);

    auto *itemStyleList = new QComboBox(m_scatterWidget);
    const QMetaObject &metaObj = QAbstract3DSeries::staticMetaObject;
    int index = metaObj.indexOfEnumerator("Mesh");
    QMetaEnum metaEnum = metaObj.enumerator(index);
    itemStyleList->addItem(u"Sphere"_s,
                           metaEnum.value(static_cast<int>(QAbstract3DSeries::Mesh::Sphere)));
    itemStyleList->addItem(u"Cube"_s,
                           metaEnum.value(static_cast<int>(QAbstract3DSeries::Mesh::Cube)));
    itemStyleList->addItem(u"Minimal"_s,
                           metaEnum.value(static_cast<int>(QAbstract3DSeries::Mesh::Minimal)));
    itemStyleList->addItem(u"Point"_s,
                           metaEnum.value(static_cast<int>(QAbstract3DSeries::Mesh::Point)));
    itemStyleList->setCurrentIndex(0);

    auto *themeList = new QComboBox(m_scatterWidget);
    themeList->addItem(u"QtGreen"_s);
    themeList->addItem(u"QtGreenNeon"_s);
    themeList->addItem(u"MixSeries"_s);
    themeList->addItem(u"OrangeSeries"_s);
    themeList->addItem(u"YellowSeries"_s);
    themeList->addItem(u"BlueSeries"_s);
    themeList->addItem(u"PurpleSeries"_s);
    themeList->addItem(u"GreySeries"_s);
    themeList->addItem(u"UserDefined"_s);
    themeList->setCurrentIndex(2);

    auto *shadowQuality = new QComboBox(m_scatterWidget);
    shadowQuality->addItem(u"None"_s);
    shadowQuality->addItem(u"Low"_s);
    shadowQuality->addItem(u"Medium"_s);
    shadowQuality->addItem(u"High"_s);
    shadowQuality->addItem(u"Low Soft"_s);
    shadowQuality->addItem(u"Medium Soft"_s);
    shadowQuality->addItem(u"High Soft"_s);
    shadowQuality->setCurrentIndex(6);

    vLayout->addWidget(cameraButton);
    vLayout->addWidget(itemCountButton);
    vLayout->addWidget(rangeButton);
    vLayout->addWidget(new QLabel(u"Adjust axis ranges"_s));
    vLayout->addWidget(rangeMinSlider);
    vLayout->addWidget(rangeMaxSlider);
    vLayout->addWidget(backgroundCheckBox);
    vLayout->addWidget(gridCheckBox);
    vLayout->addWidget(smoothCheckBox);
    vLayout->addWidget(new QLabel(u"Change dot style"_s));
    vLayout->addWidget(itemStyleList);
    vLayout->addWidget(new QLabel(u"Change theme"_s));
    vLayout->addWidget(themeList);
    vLayout->addWidget(new QLabel(u"Adjust shadow quality"_s));
    vLayout->addWidget(shadowQuality, 1, Qt::AlignTop);

    // Raise the graph to the top of the widget stack, to hide UI if resized smaller
    m_scatterGraphWidget->raise();

    m_modifier = new ScatterDataModifier(m_scatterGraphWidget->scatterGraph(), this);
    m_modifier->changeTheme(themeList->currentIndex());

    QObject::connect(cameraButton,
                     &QCommandLinkButton::clicked,
                     m_modifier,
                     &ScatterDataModifier::changePresetCamera);
    QObject::connect(itemCountButton,
                     &QCommandLinkButton::clicked,
                     m_modifier,
                     &ScatterDataModifier::toggleItemCount);
    QObject::connect(rangeButton,
                     &QCommandLinkButton::clicked,
                     m_modifier,
                     &ScatterDataModifier::toggleRanges);

    QObject::connect(rangeMinSlider,
                     &QSlider::valueChanged,
                     m_modifier,
                     &ScatterDataModifier::adjustMinimumRange);
    QObject::connect(rangeMaxSlider,
                     &QSlider::valueChanged,
                     m_modifier,
                     &ScatterDataModifier::adjustMaximumRange);

    QObject::connect(backgroundCheckBox,
                     &QCheckBox::checkStateChanged,
                     m_modifier,
                     &ScatterDataModifier::setBackgroundVisible);
    QObject::connect(gridCheckBox,
                     &QCheckBox::checkStateChanged,
                     m_modifier,
                     &ScatterDataModifier::setGridVisible);
    QObject::connect(smoothCheckBox,
                     &QCheckBox::checkStateChanged,
                     m_modifier,
                     &ScatterDataModifier::setSmoothDots);

    QObject::connect(m_modifier,
                     &ScatterDataModifier::backgroundVisibleChanged,
                     backgroundCheckBox,
                     &QCheckBox::setChecked);
    QObject::connect(m_modifier,
                     &ScatterDataModifier::gridVisibleChanged,
                     gridCheckBox,
                     &QCheckBox::setChecked);
    QObject::connect(itemStyleList,
                     &QComboBox::currentIndexChanged,
                     m_modifier,
                     &ScatterDataModifier::changeStyle);

    QObject::connect(themeList,
                     &QComboBox::currentIndexChanged,
                     m_modifier,
                     &ScatterDataModifier::changeTheme);

    QObject::connect(shadowQuality,
                     &QComboBox::currentIndexChanged,
                     m_modifier,
                     &ScatterDataModifier::changeShadowQuality);

    QObject::connect(m_modifier,
                     &ScatterDataModifier::shadowQualityChanged,
                     shadowQuality,
                     &QComboBox::setCurrentIndex);
    QObject::connect(m_scatterGraphWidget->scatterGraph(),
                     &Q3DScatterWidgetItem::shadowQualityChanged,
                     m_modifier,
                     &ScatterDataModifier::shadowQualityUpdatedByVisual);
}
