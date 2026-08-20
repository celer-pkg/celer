// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "volumetric.h"

#include <QtGui/qscreen.h>
#include <QtWidgets/qapplication.h>
#include <QtWidgets/qboxlayout.h>
#include <QtWidgets/qcheckbox.h>
#include <QtWidgets/qgroupbox.h>
#include <QtWidgets/qlabel.h>
#include <QtWidgets/qmessagebox.h>
#include <QtWidgets/qradiobutton.h>
#include <QtWidgets/qslider.h>
#include <QtWidgets/qwidget.h>

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    Q3DScatterWidgetItem *graph = new Q3DScatterWidgetItem();
    auto quickWidget = new QQuickWidget;
    graph->setWidget(quickWidget);

    QSize screenSize = graph->widget()->screen()->size();
    graph->widget()->setMinimumSize(QSize(screenSize.width() / 3, screenSize.height() / 2));
    graph->widget()->setMaximumSize(screenSize);
    graph->widget()->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    graph->widget()->setFocusPolicy(Qt::StrongFocus);

    QWidget *widget = new QWidget();
    QHBoxLayout *hLayout = new QHBoxLayout(widget);
    QVBoxLayout *vLayout = new QVBoxLayout();
    QVBoxLayout *vLayout2 = new QVBoxLayout();
    hLayout->addWidget(graph->widget(), 1);
    hLayout->addLayout(vLayout);
    hLayout->addLayout(vLayout2);

    widget->setWindowTitle(QStringLiteral("Volumetric Rendering - 3D Terrain"));

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

    QCheckBox *fpsCheckBox = new QCheckBox(widget);
    fpsCheckBox->setText(QStringLiteral("Show FPS"));
    fpsCheckBox->setChecked(false);
    QLabel *fpsLabel = new QLabel(QStringLiteral(""), widget);

    QGroupBox *textureDetailGroupBox = new QGroupBox(QStringLiteral("Texture detail"));

    QRadioButton *lowDetailRB = new QRadioButton(widget);
    lowDetailRB->setText(QStringLiteral("Low (256x128x256)"));
    lowDetailRB->setChecked(true);

    QRadioButton *mediumDetailRB = new QRadioButton(widget);
    mediumDetailRB->setText(QStringLiteral("Generating..."));
    mediumDetailRB->setChecked(false);
    mediumDetailRB->setEnabled(false);

    QRadioButton *highDetailRB = new QRadioButton(widget);
    highDetailRB->setText(QStringLiteral("Generating..."));
    highDetailRB->setChecked(false);
    highDetailRB->setEnabled(false);

    QVBoxLayout *textureDetailVBox = new QVBoxLayout;
    textureDetailVBox->addWidget(lowDetailRB);
    textureDetailVBox->addWidget(mediumDetailRB);
    textureDetailVBox->addWidget(highDetailRB);
    textureDetailGroupBox->setLayout(textureDetailVBox);

    QGroupBox *areaGroupBox = new QGroupBox(QStringLiteral("Show area"));

    QRadioButton *areaAllRB = new QRadioButton(widget);
    areaAllRB->setText(QStringLiteral("Whole region"));
    areaAllRB->setChecked(true);

    QRadioButton *areaMineRB = new QRadioButton(widget);
    areaMineRB->setText(QStringLiteral("The mine"));
    areaMineRB->setChecked(false);

    QRadioButton *areaMountainRB = new QRadioButton(widget);
    areaMountainRB->setText(QStringLiteral("The mountain"));
    areaMountainRB->setChecked(false);

    QVBoxLayout *areaVBox = new QVBoxLayout;
    areaVBox->addWidget(areaAllRB);
    areaVBox->addWidget(areaMineRB);
    areaVBox->addWidget(areaMountainRB);
    areaGroupBox->setLayout(areaVBox);

    QCheckBox *colorTableCheckBox = new QCheckBox(widget);
    colorTableCheckBox->setText(QStringLiteral("Alternate color table"));
    colorTableCheckBox->setChecked(false);

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

    QSlider *alphaMultiplierSlider = new QSlider(Qt::Horizontal, widget);
    alphaMultiplierSlider->setMinimum(0);
    alphaMultiplierSlider->setMaximum(139);
    alphaMultiplierSlider->setValue(100);
    alphaMultiplierSlider->setEnabled(true);
    QLabel *alphaMultiplierLabel = new QLabel(QStringLiteral("Alpha multiplier: 1.0"));

    QCheckBox *preserveOpacityCheckBox = new QCheckBox(widget);
    preserveOpacityCheckBox->setText(QStringLiteral("Preserve opacity"));
    preserveOpacityCheckBox->setChecked(true);

    QCheckBox *transparentGroundCheckBox = new QCheckBox(widget);
    transparentGroundCheckBox->setText(QStringLiteral("Transparent ground"));
    transparentGroundCheckBox->setChecked(false);

    QCheckBox *useHighDefShaderCheckBox = new QCheckBox(widget);
    useHighDefShaderCheckBox->setText(QStringLiteral("Use HD shader"));
    useHighDefShaderCheckBox->setChecked(true);

    QCheckBox *usePerspectiveCamera = new QCheckBox(widget);
    usePerspectiveCamera->setText(QStringLiteral("Use perspective camera"));
    usePerspectiveCamera->setChecked(false);

    QLabel *performanceNoteLabel = new QLabel(
        QStringLiteral("Note: A high end graphics card is\nrecommended with the HD shader\nwhen "
                       "the volume contains a lot of\ntransparent areas."));
    performanceNoteLabel->setFrameShape(QFrame::Box);

    QCheckBox *drawSliceFramesCheckBox = new QCheckBox(widget);
    drawSliceFramesCheckBox->setText(QStringLiteral("Draw slice frames"));
    drawSliceFramesCheckBox->setChecked(false);

    vLayout->addWidget(sliceXCheckBox);
    vLayout->addWidget(sliceXSlider);
    vLayout->addWidget(sliceImageXLabel);
    vLayout->addWidget(sliceYCheckBox);
    vLayout->addWidget(sliceYSlider);
    vLayout->addWidget(sliceImageYLabel);
    vLayout->addWidget(sliceZCheckBox);
    vLayout->addWidget(sliceZSlider);
    vLayout->addWidget(sliceImageZLabel);
    vLayout->addWidget(drawSliceFramesCheckBox, 1, Qt::AlignTop);
    vLayout2->addWidget(fpsCheckBox);
    vLayout2->addWidget(fpsLabel);
    vLayout2->addWidget(textureDetailGroupBox);
    vLayout2->addWidget(areaGroupBox);
    vLayout2->addWidget(colorTableCheckBox);
    vLayout2->addWidget(alphaMultiplierLabel);
    vLayout2->addWidget(alphaMultiplierSlider);
    vLayout2->addWidget(preserveOpacityCheckBox);
    vLayout2->addWidget(transparentGroundCheckBox);
    vLayout2->addWidget(useHighDefShaderCheckBox);
    vLayout2->addWidget(usePerspectiveCamera);
    vLayout2->addWidget(performanceNoteLabel, 1, Qt::AlignTop);

    VolumetricModifier *modifier = new VolumetricModifier(graph);
    modifier->setFpsLabel(fpsLabel);
    modifier->setMediumDetailRB(mediumDetailRB);
    modifier->setHighDetailRB(highDetailRB);
    modifier->setSliceSliders(sliceXSlider, sliceYSlider, sliceZSlider);
    modifier->setSliceLabels(sliceImageXLabel, sliceImageYLabel, sliceImageZLabel);
    modifier->setAlphaMultiplierLabel(alphaMultiplierLabel);
    modifier->setTransparentGround(transparentGroundCheckBox->isChecked());

    QObject::connect(fpsCheckBox,
                     &QCheckBox::checkStateChanged,
                     modifier,
                     &VolumetricModifier::setFpsMeasurement);
    QObject::connect(sliceXCheckBox,
                     &QCheckBox::checkStateChanged,
                     modifier,
                     &VolumetricModifier::sliceX);
    QObject::connect(sliceYCheckBox,
                     &QCheckBox::checkStateChanged,
                     modifier,
                     &VolumetricModifier::sliceY);
    QObject::connect(sliceZCheckBox,
                     &QCheckBox::checkStateChanged,
                     modifier,
                     &VolumetricModifier::sliceZ);
    QObject::connect(sliceXSlider,
                     &QSlider::valueChanged,
                     modifier,
                     &VolumetricModifier::adjustSliceX);
    QObject::connect(sliceYSlider,
                     &QSlider::valueChanged,
                     modifier,
                     &VolumetricModifier::adjustSliceY);
    QObject::connect(sliceZSlider,
                     &QSlider::valueChanged,
                     modifier,
                     &VolumetricModifier::adjustSliceZ);
    QObject::connect(lowDetailRB,
                     &QRadioButton::toggled,
                     modifier,
                     &VolumetricModifier::toggleLowDetail);
    QObject::connect(mediumDetailRB,
                     &QRadioButton::toggled,
                     modifier,
                     &VolumetricModifier::toggleMediumDetail);
    QObject::connect(highDetailRB,
                     &QRadioButton::toggled,
                     modifier,
                     &VolumetricModifier::toggleHighDetail);
    QObject::connect(colorTableCheckBox,
                     &QCheckBox::checkStateChanged,
                     modifier,
                     &VolumetricModifier::changeColorTable);
    QObject::connect(preserveOpacityCheckBox,
                     &QCheckBox::checkStateChanged,
                     modifier,
                     &VolumetricModifier::setPreserveOpacity);
    QObject::connect(transparentGroundCheckBox,
                     &QCheckBox::checkStateChanged,
                     modifier,
                     &VolumetricModifier::setTransparentGround);
    QObject::connect(useHighDefShaderCheckBox,
                     &QCheckBox::checkStateChanged,
                     modifier,
                     &VolumetricModifier::setUseHighDefShader);
    QObject::connect(usePerspectiveCamera,
                     &QCheckBox::checkStateChanged,
                     modifier,
                     &VolumetricModifier::setUsePerspectiveCamera);
    QObject::connect(alphaMultiplierSlider,
                     &QSlider::valueChanged,
                     modifier,
                     &VolumetricModifier::adjustAlphaMultiplier);
    QObject::connect(areaAllRB,
                     &QRadioButton::toggled,
                     modifier,
                     &VolumetricModifier::toggleAreaAll);
    QObject::connect(areaMineRB,
                     &QRadioButton::toggled,
                     modifier,
                     &VolumetricModifier::toggleAreaMine);
    QObject::connect(areaMountainRB,
                     &QRadioButton::toggled,
                     modifier,
                     &VolumetricModifier::toggleAreaMountain);
    QObject::connect(drawSliceFramesCheckBox,
                     &QCheckBox::checkStateChanged,
                     modifier,
                     &VolumetricModifier::setDrawSliceFrames);

    widget->show();
    int retval = app.exec();
    delete modifier;
    delete quickWidget;
    return retval;
}
