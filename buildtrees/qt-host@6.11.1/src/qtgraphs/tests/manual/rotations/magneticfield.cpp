// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "magneticfield.h"
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSlider>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMessageBox>
#include <QtGui/QScreen>

MagneticField::MagneticField(QObject *parent)
: QObject(parent)
{
    initialize();
}

void MagneticField::initialize()
{

    quickWidget = new QQuickWidget();
    graph = new Q3DScatterWidgetItem();
    graph->setWidget(quickWidget);

    QSize screenSize = graph->widget()->screen()->size();
    graph->widget()->setMinimumSize(QSize(screenSize.width() / 2, screenSize.height() / 1.5));
    graph->widget()->setMaximumSize(screenSize);
    graph->widget()->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    graph->widget()->setFocusPolicy(Qt::StrongFocus);
    graph->widget()->setResizeMode(QQuickWidget::SizeRootObjectToView);

    m_container = new QWidget();
    QHBoxLayout *hLayout = new QHBoxLayout(m_container);
    QVBoxLayout *vLayout = new QVBoxLayout();
    hLayout->addWidget(graph->widget(), 1);
    hLayout->addLayout(vLayout);

    m_container->setWindowTitle(QStringLiteral("Item rotations example - Magnetic field of the sun"));

    QPushButton *toggleRotationButton = new QPushButton(m_container);
    toggleRotationButton->setText(QStringLiteral("Toggle animation"));
    QPushButton *toggleSunButton = new QPushButton(m_container);
    toggleSunButton->setText(QStringLiteral("Toggle Sun"));

    QSlider *fieldLinesSlider = new QSlider(Qt::Horizontal, m_container);
    fieldLinesSlider->setTickInterval(1);
    fieldLinesSlider->setMinimum(1);
    fieldLinesSlider->setValue(12);
    fieldLinesSlider->setMaximum(128);

    QSlider *arrowsSlider = new QSlider(Qt::Horizontal, m_container);
    arrowsSlider->setTickInterval(1);
    arrowsSlider->setMinimum(8);
    arrowsSlider->setValue(16);
    arrowsSlider->setMaximum(32);

    vLayout->addWidget(toggleRotationButton);
    vLayout->addWidget(toggleSunButton);
    vLayout->addWidget(new QLabel(QStringLiteral("Field Lines (1 - 128):")));
    vLayout->addWidget(fieldLinesSlider);
    vLayout->addWidget(new QLabel(QStringLiteral("Arrows per line (8 - 32):")));
    vLayout->addWidget(arrowsSlider, 1, Qt::AlignTop);

    modifier = new ScatterDataModifier(graph);

    QObject::connect(toggleRotationButton, &QPushButton::clicked, modifier,
                     &ScatterDataModifier::toggleRotation);
    QObject::connect(toggleSunButton, &QPushButton::clicked, modifier,
                     &ScatterDataModifier::toggleSun);
    QObject::connect(fieldLinesSlider, &QSlider::valueChanged, modifier,
                     &ScatterDataModifier::setFieldLines);
    QObject::connect(arrowsSlider, &QSlider::valueChanged, modifier,
                     &ScatterDataModifier::setArrowsPerLine);

    // m_container->show();
}
