// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "volumetrictest.h"
#include <QtWidgets/QApplication>
#include <QtWidgets/QWidget>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QSlider>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtGui/QScreen>

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    //Q3DScatterWidgetItem *graph = new Q3DScatterWidgetItem();
    //Q3DSurfaceWidgetItem *graph = new Q3DSurfaceWidgetItem();
    Q3DBarsWidgetItem *graph = new Q3DBarsWidgetItem();
    QQuickWidget quickWidget;
    graph->setWidget(&quickWidget);
    QSize screenSize = graph->widget()->screen()->size();
    graph->widget()->setMinimumSize(QSize(screenSize.width() / 4, screenSize.height() / 4));
    graph->widget()->setMaximumSize(screenSize);
    graph->widget()->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    graph->widget()->setFocusPolicy(Qt::StrongFocus);
    graph->widget()->setResizeMode(QQuickWidget::SizeRootObjectToView);

    QWidget *widget = new QWidget;
    QHBoxLayout *hLayout = new QHBoxLayout(widget);
    QVBoxLayout *vLayout = new QVBoxLayout();
    hLayout->addWidget(graph->widget(), 1);
    hLayout->addLayout(vLayout);

    widget->setWindowTitle(QStringLiteral("Volumetric TEST"));
    widget->resize(QSize(screenSize.width() / 1.5, screenSize.height() / 1.5));

    QCheckBox *sliceXCheckBox = new QCheckBox(widget);
    sliceXCheckBox->setText(QStringLiteral("Slice volume on X axis"));
    sliceXCheckBox->setChecked(false);
    QCheckBox *sliceYCheckBox = new QCheckBox(widget);
    sliceYCheckBox->setText(QStringLiteral("Slice volume on Y axis"));
    sliceYCheckBox->setChecked(false);
    QCheckBox *sliceZCheckBox = new QCheckBox(widget);
    sliceZCheckBox->setText(QStringLiteral("Slice volume on Z axis"));
    sliceZCheckBox->setChecked(false);

    QSlider *sliceXSlider = new QSlider(Qt::Horizontal, widget);
    sliceXSlider->setMinimum(0);
    sliceXSlider->setMaximum(1024);
    sliceXSlider->setValue(512);
    sliceXSlider->setEnabled(true);
    QSlider *sliceYSlider = new QSlider(Qt::Horizontal, widget);
    sliceYSlider->setMinimum(0);
    sliceYSlider->setMaximum(1024);
    sliceYSlider->setValue(512);
    sliceYSlider->setEnabled(true);
    QSlider *sliceZSlider = new QSlider(Qt::Horizontal, widget);
    sliceZSlider->setMinimum(0);
    sliceZSlider->setMaximum(1024);
    sliceZSlider->setValue(512);
    sliceZSlider->setEnabled(true);

    QLabel *fpsLabel = new QLabel(QStringLiteral("Fps: "), widget);

    QLabel *sliceImageXLabel = new QLabel(widget);
    QLabel *sliceImageYLabel = new QLabel(widget);
    QLabel *sliceImageZLabel = new QLabel(widget);
    sliceImageXLabel->setMinimumSize(QSize(200, 100));
    sliceImageYLabel->setMinimumSize(QSize(200, 200));
    sliceImageZLabel->setMinimumSize(QSize(200, 100));
    sliceImageXLabel->setMaximumSize(QSize(200, 100));
    sliceImageYLabel->setMaximumSize(QSize(200, 200));
    sliceImageZLabel->setMaximumSize(QSize(200, 100));
    sliceImageXLabel->setFrameShape(QFrame::Box);
    sliceImageYLabel->setFrameShape(QFrame::Box);
    sliceImageZLabel->setFrameShape(QFrame::Box);
    sliceImageXLabel->setScaledContents(true);
    sliceImageYLabel->setScaledContents(true);
    sliceImageZLabel->setScaledContents(true);

    QPushButton *testSubTextureSetting = new QPushButton(widget);
    testSubTextureSetting->setText(QStringLiteral("Test subtexture settings"));

    QLabel *rangeSliderLabel = new QLabel(QStringLiteral("Adjust ranges:"), widget);

    QSlider *rangeXSlider = new QSlider(Qt::Horizontal, widget);
    rangeXSlider->setMinimum(0);
    rangeXSlider->setMaximum(1024);
    rangeXSlider->setValue(512);
    rangeXSlider->setEnabled(true);
    QSlider *rangeYSlider = new QSlider(Qt::Horizontal, widget);
    rangeYSlider->setMinimum(0);
    rangeYSlider->setMaximum(1024);
    rangeYSlider->setValue(512);
    rangeYSlider->setEnabled(true);
    QSlider *rangeZSlider = new QSlider(Qt::Horizontal, widget);
    rangeZSlider->setMinimum(0);
    rangeZSlider->setMaximum(1024);
    rangeZSlider->setValue(512);
    rangeZSlider->setEnabled(true);

    QPushButton *reverseAxes = new QPushButton(widget);
    reverseAxes->setText(QStringLiteral("Reverse axes"));

    QPushButton *testBoundsSetting = new QPushButton(widget);
    testBoundsSetting->setText(QStringLiteral("Test data bounds"));

    vLayout->addWidget(fpsLabel);
    vLayout->addWidget(sliceXCheckBox);
    vLayout->addWidget(sliceXSlider);
    vLayout->addWidget(sliceImageXLabel);
    vLayout->addWidget(sliceYCheckBox);
    vLayout->addWidget(sliceYSlider);
    vLayout->addWidget(sliceImageYLabel);
    vLayout->addWidget(sliceZCheckBox);
    vLayout->addWidget(sliceZSlider);
    vLayout->addWidget(sliceImageZLabel);
    vLayout->addWidget(rangeSliderLabel);
    vLayout->addWidget(rangeXSlider);
    vLayout->addWidget(rangeYSlider);
    vLayout->addWidget(rangeZSlider);
    vLayout->addWidget(reverseAxes);
    vLayout->addWidget(testBoundsSetting);
    vLayout->addWidget(testSubTextureSetting, 1, Qt::AlignTop);

    VolumetricModifier *modifier = new VolumetricModifier(graph);
    modifier->setFpsLabel(fpsLabel);
    modifier->setSliceLabels(sliceImageXLabel, sliceImageYLabel, sliceImageZLabel);

    QObject::connect(sliceXCheckBox, &QCheckBox::checkStateChanged, modifier,
                     &VolumetricModifier::sliceX);
    QObject::connect(sliceYCheckBox, &QCheckBox::checkStateChanged, modifier,
                     &VolumetricModifier::sliceY);
    QObject::connect(sliceZCheckBox, &QCheckBox::checkStateChanged, modifier,
                     &VolumetricModifier::sliceZ);
    QObject::connect(sliceXSlider, &QSlider::valueChanged, modifier,
                     &VolumetricModifier::adjustSliceX);
    QObject::connect(sliceYSlider, &QSlider::valueChanged, modifier,
                     &VolumetricModifier::adjustSliceY);
    QObject::connect(sliceZSlider, &QSlider::valueChanged, modifier,
                     &VolumetricModifier::adjustSliceZ);
    QObject::connect(testSubTextureSetting, &QPushButton::clicked, modifier,
                     &VolumetricModifier::testSubtextureSetting);
    QObject::connect(rangeXSlider, &QSlider::valueChanged, modifier,
                     &VolumetricModifier::adjustRangeX);
    QObject::connect(rangeYSlider, &QSlider::valueChanged, modifier,
                     &VolumetricModifier::adjustRangeY);
    QObject::connect(rangeZSlider, &QSlider::valueChanged, modifier,
                     &VolumetricModifier::adjustRangeZ);
    QObject::connect(reverseAxes, &QPushButton::clicked, modifier,
                     &VolumetricModifier::reverseAxes);
    QObject::connect(testBoundsSetting, &QPushButton::clicked, modifier,
                     &VolumetricModifier::testBoundsSetting);

    widget->show();
    return app.exec();
}
