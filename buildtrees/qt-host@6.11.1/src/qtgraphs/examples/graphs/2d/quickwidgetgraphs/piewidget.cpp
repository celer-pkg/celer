// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "piewidget.h"
#include "piegraph.h"
#include <QGraphsTheme>

PieWidget::PieWidget(QWidget *parent)
{
    Q_UNUSED(parent)
    //! [1]
    m_pieGraph = new PieGraph;
    //! [1]
    //! [0]
    m_quickWidget = new QQuickWidget;
    m_widget = new QWidget;
    m_vLayout = new QVBoxLayout(m_widget);
    m_hLayout = new QHBoxLayout;
    //! [0]
    initializeQuickWidget();
    initializeButtons();

    //! [4]
    m_vLayout->addLayout(m_hLayout);
    m_vLayout->addWidget(m_quickWidget, 1);
    //! [4]
}

PieWidget::~PieWidget()
{
    delete m_quickWidget;
    delete m_hLayout;
    delete m_vLayout;
    delete m_widget;
    delete m_pieGraph;
}

//! [3]
void PieWidget::initializeButtons()
{
    QPushButton *addButton = new QPushButton("Add Slice");
    QPushButton *removeButton = new QPushButton("Remove Slice");
    QPushButton *explodeButton = new QPushButton("Explode All");
    QPushButton *clearButton = new QPushButton("Clear Series");

    m_hLayout->addWidget(addButton);
    m_hLayout->addWidget(removeButton);
    m_hLayout->addWidget(explodeButton);
    m_hLayout->addWidget(clearButton);

    QObject::connect(addButton, &QPushButton::clicked, m_pieGraph, &PieGraph::onAddSlice);
    QObject::connect(removeButton, &QPushButton::clicked, m_pieGraph, &PieGraph::onRemoveSlice);
    QObject::connect(explodeButton, &QPushButton::clicked, m_pieGraph, &PieGraph::onExplode);
    QObject::connect(clearButton, &QPushButton::clicked, m_pieGraph, &PieGraph::onClearSeries);
}
//! [3]

void PieWidget::initializeQuickWidget()
{
    //! [5]
    auto theme = new QGraphsTheme(m_quickWidget);
    theme->setTheme(QGraphsTheme::Theme::BlueSeries);
    theme->setLabelBorderVisible(true);
    theme->setLabelBackgroundVisible(true);
    theme->setBackgroundColor(Qt::black);
    //! [5]

    //! [2]
    m_quickWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);
    m_quickWidget->resize(1280, 720);
    m_quickWidget->setInitialProperties({
        {"theme", QVariant::fromValue(theme) },
        {"seriesList", QVariant::fromValue(m_pieGraph->pieSeries()) }
    });
    m_quickWidget->loadFromModule("QtGraphs", "GraphsView");
    //! [2]
}

QWidget *PieWidget::containerWidget() const
{
    return m_widget;
}
