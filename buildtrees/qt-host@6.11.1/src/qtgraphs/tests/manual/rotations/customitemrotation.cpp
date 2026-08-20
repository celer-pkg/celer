// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "customitemrotation.h"
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSlider>
#include <QtWidgets/QLabel>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QMessageBox>
#include <QtGui/QScreen>

CustomItemRotation::CustomItemRotation(QObject *parent)
: QObject(parent)
{
    initialize();
}

void CustomItemRotation::initialize()
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



    QCheckBox *absoluteCheckBox = new QCheckBox(m_container);
    absoluteCheckBox->setText(QStringLiteral("Absolute scaling:"));
    absoluteCheckBox->setChecked(false);

    QCheckBox *axisXReversed = new QCheckBox(m_container);
    axisXReversed->setText("X-axis reversed:");
    QCheckBox *axisYReversed = new QCheckBox(m_container);
    axisYReversed->setText("Y-axis reversed:");
    QCheckBox *axisZReversed = new QCheckBox(m_container);
    axisZReversed->setText("Z-axis reversed:");

    QPushButton *toggleTimerButton = new  QPushButton(m_container);
    toggleTimerButton->setText(QStringLiteral("Toggle animation"));

    QCheckBox *customRotation = new QCheckBox(m_container);
    customRotation->setText("Use custom rotation");

    QSlider *xSlider = new QSlider(Qt::Horizontal, m_container);
    xSlider->setTickPosition(QSlider::NoTicks);
    xSlider->setSingleStep(10);
    xSlider->setMinimum(-180);
    xSlider->setMaximum(180);
    xSlider->setVisible(customRotation->isChecked());

    QSlider *ySlider = new QSlider(Qt::Horizontal, m_container);
    ySlider->setTickPosition(QSlider::NoTicks);
    ySlider->setSingleStep(10);
    ySlider->setMinimum(-180);
    ySlider->setMaximum(180);
    ySlider->setVisible(customRotation->isChecked());

    QSlider *zSlider = new QSlider(Qt::Horizontal, m_container);
    zSlider->setTickPosition(QSlider::NoTicks);
    zSlider->setSingleStep(10);
    zSlider->setMinimum(-180);
    zSlider->setMaximum(180);
    zSlider->setVisible(customRotation->isChecked());

    QLabel *xLabel = new QLabel(m_container);
    xLabel->setText("X: 0");
    xLabel->setVisible(customRotation->isChecked());

    QLabel *yLabel = new QLabel(m_container);
    yLabel->setText("Y: 0");
    yLabel->setVisible(customRotation->isChecked());

    QLabel *zLabel = new QLabel(m_container);
    zLabel->setText("Z: 0");
    zLabel->setVisible(customRotation->isChecked());

    vLayout->addWidget(absoluteCheckBox);
    vLayout->addWidget(axisXReversed);
    vLayout->addWidget(axisYReversed);
    vLayout->addWidget(axisZReversed);
    vLayout->addWidget(toggleTimerButton);
    vLayout->addWidget(customRotation);
    vLayout->addWidget(xSlider);
    vLayout->addWidget(xLabel);
    vLayout->addWidget(ySlider);
    vLayout->addWidget(yLabel);
    vLayout->addWidget(zSlider);
    vLayout->addWidget(zLabel);

    modifier = new CustomItemDataModifier(graph);

    QObject::connect(toggleTimerButton, &QPushButton::clicked, modifier,
                    &CustomItemDataModifier::toggleTimer);
    QObject::connect(absoluteCheckBox, &QCheckBox::checkStateChanged, modifier,
                    &CustomItemDataModifier::toggleAbsolute);
    QObject::connect(axisXReversed, &QCheckBox::checkStateChanged, modifier,
                    &CustomItemDataModifier::setXAxisReversed);
    QObject::connect(axisYReversed, &QCheckBox::checkStateChanged, modifier,
                    &CustomItemDataModifier::setYAxisReversed);
    QObject::connect(axisZReversed, &QCheckBox::checkStateChanged, modifier,
                    &CustomItemDataModifier::setZAxisReversed);
    QObject::connect(customRotation, &QCheckBox::checkStateChanged, modifier,
                    &CustomItemDataModifier::setCustomRotation);
    QObject::connect(xSlider, &QSlider::valueChanged, modifier,
                    &CustomItemDataModifier::setCustomX);
    QObject::connect(ySlider, &QSlider::valueChanged, modifier,
                    &CustomItemDataModifier::setCustomY);
    QObject::connect(zSlider, &QSlider::valueChanged, modifier,
                    &CustomItemDataModifier::setCustomZ);

    QObject::connect(customRotation, &QCheckBox::checkStateChanged, this,
                    [toggleTimerButton, xSlider, ySlider, zSlider, xLabel, yLabel, zLabel, customRotation]() {
                        xSlider->setVisible(customRotation->isChecked());
                        ySlider->setVisible(customRotation->isChecked());
                        zSlider->setVisible(customRotation->isChecked());
                        xLabel->setVisible(customRotation->isChecked());
                        yLabel->setVisible(customRotation->isChecked());
                        zLabel->setVisible(customRotation->isChecked());
                        toggleTimerButton->setVisible(!customRotation->isChecked());
                    });

    QObject::connect(xSlider, &QSlider::valueChanged, this,
        [xSlider, xLabel]() {
            xLabel->setText(QString("X: %1").arg(xSlider->value()));
        });
    QObject::connect(ySlider, &QSlider::valueChanged, this,
        [ySlider, yLabel]() {
            yLabel->setText(QString("Y: %1").arg(ySlider->value()));
        });
    QObject::connect(zSlider, &QSlider::valueChanged, this,
        [zSlider, zLabel]() {
            zLabel->setText(QString("Z: %1").arg(zSlider->value()));
        });

}
