// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "scattergraph.h"
#include "scatterdatamodifier.h"

#include <QtWidgets/qboxlayout.h>
#include <QtWidgets/qcheckbox.h>
#include <QtWidgets/qcombobox.h>
#include <QtWidgets/qlabel.h>
#include <QtWidgets/qcommandlinkbutton.h>

using namespace Qt::StringLiterals;

ScatterGraph::ScatterGraph()
{
    m_scatterGraph = new Q3DScatter();
}

ScatterGraph::~ScatterGraph() = default;

bool ScatterGraph::initialize(const QSize &minimumGraphSize, const QSize &maximumGraphSize)
{
    if (!m_scatterGraph->hasContext())
        return false;

    m_scatterWidget = new QWidget;
    auto *hLayout = new QHBoxLayout(m_scatterWidget);
    m_container = QWidget::createWindowContainer(m_scatterGraph, m_scatterWidget);
    m_container->setMinimumSize(minimumGraphSize);
    m_container->setMaximumSize(maximumGraphSize);
    m_container->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_container->setFocusPolicy(Qt::StrongFocus);
    hLayout->addWidget(m_container, 1);

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

    auto *backgroundCheckBox = new QCheckBox(m_scatterWidget);
    backgroundCheckBox->setText(u"Show background"_s);
    backgroundCheckBox->setChecked(true);

    auto *gridCheckBox = new QCheckBox(m_scatterWidget);
    gridCheckBox->setText(u"Show grid"_s);
    gridCheckBox->setChecked(true);

    auto *smoothCheckBox = new QCheckBox(m_scatterWidget);
    smoothCheckBox->setText(u"Smooth dots"_s);
    smoothCheckBox->setChecked(true);

    auto *itemStyleList = new QComboBox(m_scatterWidget);
    itemStyleList->addItem(u"Sphere"_s, QAbstract3DSeries::MeshSphere);
    itemStyleList->addItem(u"Cube"_s, QAbstract3DSeries::MeshCube);
    itemStyleList->addItem(u"Minimal"_s, QAbstract3DSeries::MeshMinimal);
    itemStyleList->addItem(u"Point"_s, QAbstract3DSeries::MeshPoint);
    itemStyleList->setCurrentIndex(0);

    auto *themeList = new QComboBox(m_scatterWidget);
    themeList->addItem(u"Qt"_s);
    themeList->addItem(u"Primary Colors"_s);
    themeList->addItem(u"Digia"_s);
    themeList->addItem(u"Stone Moss"_s);
    themeList->addItem(u"Army Blue"_s);
    themeList->addItem(u"Retro"_s);
    themeList->addItem(u"Ebony"_s);
    themeList->addItem(u"Isabelle"_s);
    themeList->setCurrentIndex(3);

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
    vLayout->addWidget(backgroundCheckBox);
    vLayout->addWidget(gridCheckBox);
    vLayout->addWidget(smoothCheckBox);
    vLayout->addWidget(new QLabel(u"Change dot style"_s));
    vLayout->addWidget(itemStyleList);
    vLayout->addWidget(new QLabel(u"Change theme"_s));
    vLayout->addWidget(themeList);
    vLayout->addWidget(new QLabel(u"Adjust shadow quality"_s));
    vLayout->addWidget(shadowQuality, 1, Qt::AlignTop);

    auto *modifier = new ScatterDataModifier(m_scatterGraph, this);

    QObject::connect(cameraButton, &QCommandLinkButton::clicked, modifier,
                     &ScatterDataModifier::changePresetCamera);
    QObject::connect(itemCountButton, &QCommandLinkButton::clicked, modifier,
                     &ScatterDataModifier::toggleItemCount);
    QObject::connect(rangeButton, &QCommandLinkButton::clicked, modifier,
                     &ScatterDataModifier::toggleRanges);

    QObject::connect(backgroundCheckBox, &QCheckBox::stateChanged, modifier,
                     &ScatterDataModifier::setBackgroundEnabled);
    QObject::connect(gridCheckBox, &QCheckBox::stateChanged, modifier,
                     &ScatterDataModifier::setGridEnabled);
    QObject::connect(smoothCheckBox, &QCheckBox::stateChanged, modifier,
                     &ScatterDataModifier::setSmoothDots);

    QObject::connect(modifier, &ScatterDataModifier::backgroundEnabledChanged,
                     backgroundCheckBox, &QCheckBox::setChecked);
    QObject::connect(modifier, &ScatterDataModifier::gridEnabledChanged,
                     gridCheckBox, &QCheckBox::setChecked);
    QObject::connect(itemStyleList, &QComboBox::currentIndexChanged, modifier,
                     &ScatterDataModifier::changeStyle);

    QObject::connect(themeList, &QComboBox::currentIndexChanged, modifier,
                     &ScatterDataModifier::changeTheme);

    QObject::connect(shadowQuality, &QComboBox::currentIndexChanged, modifier,
                     &ScatterDataModifier::changeShadowQuality);

    QObject::connect(modifier, &ScatterDataModifier::shadowQualityChanged, shadowQuality,
                     &QComboBox::setCurrentIndex);
    QObject::connect(m_scatterGraph, &Q3DScatter::shadowQualityChanged, modifier,
                     &ScatterDataModifier::shadowQualityUpdatedByVisual);

    return true;
}
