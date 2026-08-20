// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "scatterdatamodifier.h"

#include <QApplication>
#include <QWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QCheckBox>
#include <QComboBox>
#include <QFontComboBox>
#include <QLabel>
#include <QScreen>
#include <QFontDatabase>

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    QQuickWidget *quickWidget = new QQuickWidget;
    Q3DScatterWidgetItem *graph = new Q3DScatterWidgetItem();
    graph->setWidget(quickWidget);
    graph->setMeasureFps(true);

    QSize screenSize = graph->widget()->screen()->size();
    graph->widget()->setMinimumSize(QSize(screenSize.width() / 2, screenSize.height() / 1.5));
    graph->widget()->setMaximumSize(screenSize);
    graph->widget()->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    graph->widget()->setFocusPolicy(Qt::StrongFocus);

    QWidget *widget = new QWidget;
    QHBoxLayout *hLayout = new QHBoxLayout(widget);
    QVBoxLayout *vLayout = new QVBoxLayout();
    hLayout->addWidget(graph->widget(), 1);
    hLayout->addLayout(vLayout);

    widget->setWindowTitle(QStringLiteral("Directional scatter"));

    QComboBox *themeList = new QComboBox(widget);
    themeList->addItem(QStringLiteral("QtGreen"));
    themeList->addItem(QStringLiteral("QtGreenNeon"));
    themeList->addItem(QStringLiteral("MixSeries"));
    themeList->addItem(QStringLiteral("OrangeSeries"));
    themeList->addItem(QStringLiteral("YellowSeries"));
    themeList->addItem(QStringLiteral("BlueSeries"));
    themeList->addItem(QStringLiteral("PurpleSeries"));
    themeList->addItem(QStringLiteral("GreySeries"));
    themeList->addItem(QStringLiteral("UserDefined"));
    themeList->setCurrentIndex(6);

    QPushButton *labelButton = new QPushButton(widget);
    labelButton->setText(QStringLiteral("Change label style"));

    QComboBox *itemStyleList = new QComboBox(widget);
    itemStyleList->addItem(QStringLiteral("Arrow"), int(QAbstract3DSeries::Mesh::Arrow));
    itemStyleList->addItem(QStringLiteral("Cube"), int(QAbstract3DSeries::Mesh::Cube));
    itemStyleList->addItem(QStringLiteral("Minimal"), int(QAbstract3DSeries::Mesh::Minimal));
    itemStyleList->setCurrentIndex(-1);

    QPushButton *cameraButton = new QPushButton(widget);
    cameraButton->setText(QStringLiteral("Change camera preset"));

    QPushButton *toggleRotationButton = new QPushButton(widget);
    toggleRotationButton->setText(QStringLiteral("Toggle animation"));

    QCheckBox *backgroundCheckBox = new QCheckBox(widget);
    backgroundCheckBox->setText(QStringLiteral("Show background"));
    backgroundCheckBox->setChecked(true);

    QCheckBox *optimizationCheckBox = new QCheckBox(widget);
    optimizationCheckBox->setText(QStringLiteral("Optimization static"));

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
    shadowQuality->setCurrentIndex(4);

    QFontComboBox *fontList = new QFontComboBox(widget);
    fontList->setCurrentFont(QFont("Arial"));

    vLayout->addWidget(labelButton, 0, Qt::AlignTop);
    vLayout->addWidget(cameraButton, 0, Qt::AlignTop);
    vLayout->addWidget(toggleRotationButton, 0, Qt::AlignTop);
    vLayout->addWidget(optimizationCheckBox);
    vLayout->addWidget(backgroundCheckBox);
    vLayout->addWidget(gridCheckBox);
    vLayout->addWidget(new QLabel(QStringLiteral("Change dot style")));
    vLayout->addWidget(itemStyleList);
    vLayout->addWidget(new QLabel(QStringLiteral("Change theme")));
    vLayout->addWidget(themeList);
    vLayout->addWidget(new QLabel(QStringLiteral("Adjust shadow quality")));
    vLayout->addWidget(shadowQuality);
    vLayout->addWidget(new QLabel(QStringLiteral("Change font")));
    vLayout->addWidget(fontList, 1, Qt::AlignTop);

    ScatterDataModifier *modifier = new ScatterDataModifier(graph);

    QObject::connect(graph, &Q3DGraphsWidgetItem::currentFpsChanged, modifier,
                     &ScatterDataModifier::fpsChanged);

    QObject::connect(cameraButton, &QPushButton::clicked, modifier,
                     &ScatterDataModifier::changePresetCamera);
    QObject::connect(toggleRotationButton, &QPushButton::clicked, modifier,
                     &ScatterDataModifier::toggleRotation);
    QObject::connect(labelButton, &QPushButton::clicked, modifier,
                     &ScatterDataModifier::changeLabelStyle);

    QObject::connect(backgroundCheckBox, &QCheckBox::checkStateChanged, modifier,
                     &ScatterDataModifier::setBackgroundVisible);
    QObject::connect(gridCheckBox, &QCheckBox::checkStateChanged, modifier,
                     &ScatterDataModifier::setGridVisible);

    QObject::connect(modifier, &ScatterDataModifier::backgroundVisibleChanged,
                     backgroundCheckBox, &QCheckBox::setChecked);
    QObject::connect(optimizationCheckBox, &QCheckBox::checkStateChanged,
                     modifier, &ScatterDataModifier::enableOptimization);
    QObject::connect(modifier, &ScatterDataModifier::gridVisibleChanged,
                     gridCheckBox, &QCheckBox::setChecked);
    QObject::connect(itemStyleList, SIGNAL(currentIndexChanged(int)), modifier,
                     SLOT(changeStyle(int)));

    QObject::connect(themeList, SIGNAL(currentIndexChanged(int)), modifier,
                     SLOT(changeTheme(int)));
    modifier->changeTheme(themeList->currentIndex());

    QObject::connect(shadowQuality, SIGNAL(currentIndexChanged(int)), modifier,
                     SLOT(changeShadowQuality(int)));

    QObject::connect(modifier, &ScatterDataModifier::shadowQualityChanged, shadowQuality,
                     &QComboBox::setCurrentIndex);
    QObject::connect(graph, &Q3DScatterWidgetItem::shadowQualityChanged, modifier,
                     &ScatterDataModifier::shadowQualityUpdatedByVisual);

    QObject::connect(fontList, &QFontComboBox::currentFontChanged, modifier,
                     &ScatterDataModifier::changeFont);

    QObject::connect(modifier, &ScatterDataModifier::fontChanged, fontList,
                     &QFontComboBox::setCurrentFont);

    itemStyleList->setCurrentIndex(0);
    optimizationCheckBox->setChecked(true);

    widget->show();
    return app.exec();
}
