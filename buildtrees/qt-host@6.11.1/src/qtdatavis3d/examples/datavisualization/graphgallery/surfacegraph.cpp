// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "surfacegraph.h"
#include "surfacegraphmodifier.h"

#include <QtWidgets/qboxlayout.h>
#include <QtWidgets/qcheckbox.h>
#include <QtWidgets/qcombobox.h>
#include <QtWidgets/qradiobutton.h>
#include <QtWidgets/qgroupbox.h>
#include <QtWidgets/qlabel.h>
#include <QtWidgets/qcommandlinkbutton.h>
#include <QtGui/qpainter.h>

using namespace Qt::StringLiterals;

static QPixmap gradientBtoYPB_Pixmap()
{
    QLinearGradient grBtoY(0, 0, 1, 100);
    grBtoY.setColorAt(1.f, Qt::black);
    grBtoY.setColorAt(0.67f, Qt::blue);
    grBtoY.setColorAt(0.33f, Qt::red);
    grBtoY.setColorAt(0.f, Qt::yellow);
    QPixmap pm(24, 100);
    QPainter pmp(&pm);
    pmp.setBrush(QBrush(grBtoY));
    pmp.setPen(Qt::NoPen);
    pmp.drawRect(0, 0, 24, 100);
    return pm;
}

static QPixmap gradientGtoRPB_Pixmap()
{
    QLinearGradient grGtoR(0, 0, 1, 100);
    grGtoR.setColorAt(1.f, Qt::darkGreen);
    grGtoR.setColorAt(0.5f, Qt::yellow);
    grGtoR.setColorAt(0.2f, Qt::red);
    grGtoR.setColorAt(0.f, Qt::darkRed);
    QPixmap pm(24, 100);
    QPainter pmp(&pm);
    pmp.setBrush(QBrush(grGtoR));
    pmp.drawRect(0, 0, 24, 100);
    return pm;
}

static QPixmap highlightPixmap()
{
    constexpr int height = 400;
    constexpr int width = 110;
    constexpr int border = 10;
    QLinearGradient gr(0, 0, 1, height - 2 * border);
    gr.setColorAt(1.f, Qt::black);
    gr.setColorAt(0.8f, Qt::darkGreen);
    gr.setColorAt(0.6f, Qt::green);
    gr.setColorAt(0.4f, Qt::yellow);
    gr.setColorAt(0.2f, Qt::red);
    gr.setColorAt(0.f, Qt::darkRed);

    QPixmap pmHighlight(width, height);
    pmHighlight.fill(Qt::transparent);
    QPainter pmpHighlight(&pmHighlight);
    pmpHighlight.setBrush(QBrush(gr));
    pmpHighlight.setPen(Qt::NoPen);
    pmpHighlight.drawRect(border, border, 35, height - 2 * border);
    pmpHighlight.setPen(Qt::black);
    int step = (height - 2 * border) / 5;
    for (int i = 0; i < 6; ++i) {
        int yPos = i * step + border;
        pmpHighlight.drawLine(border, yPos, 55, yPos);
        const int height = 550 - (i * 110);
        pmpHighlight.drawText(60, yPos + 2, QString::number(height) + u" m"_s);
    }
    return pmHighlight;
}

SurfaceGraph::SurfaceGraph()
{
    m_surfaceGraph = new Q3DSurface();
}

SurfaceGraph::~SurfaceGraph() = default;

bool SurfaceGraph::initialize(const QSize &minimumGraphSize, const QSize &maximumGraphSize)
{
    if (!m_surfaceGraph->hasContext())
        return false;

    m_surfaceWidget = new QWidget;
    auto *hLayout = new QHBoxLayout(m_surfaceWidget);
    m_container = QWidget::createWindowContainer(m_surfaceGraph, m_surfaceWidget);
    m_container->setMinimumSize(minimumGraphSize);
    m_container->setMaximumSize(maximumGraphSize);
    m_container->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_container->setFocusPolicy(Qt::StrongFocus);
    hLayout->addWidget(m_container, 1);

    auto *vLayout = new QVBoxLayout();
    hLayout->addLayout(vLayout);
    vLayout->setAlignment(Qt::AlignTop);

    // Create control widgets
    auto *modelGroupBox = new QGroupBox(u"Model"_s);

    auto *sqrtSinModelRB = new QRadioButton(m_surfaceWidget);
    sqrtSinModelRB->setText(u"Sqrt && Sin"_s);
    sqrtSinModelRB->setChecked(false);

    auto *heightMapModelRB = new QRadioButton(m_surfaceWidget);
    heightMapModelRB->setText(u"Multiseries\nHeight Map"_s);
    heightMapModelRB->setChecked(false);

    auto *texturedModelRB = new QRadioButton(m_surfaceWidget);
    texturedModelRB->setText(u"Textured\nTopography"_s);
    texturedModelRB->setChecked(false);

    auto *modelVBox = new QVBoxLayout;
    modelVBox->addWidget(sqrtSinModelRB);
    modelVBox->addWidget(heightMapModelRB);
    modelVBox->addWidget(texturedModelRB);
    modelGroupBox->setLayout(modelVBox);

    auto *selectionGroupBox = new QGroupBox(u"Graph Selection Mode"_s);

    auto *modeNoneRB = new QRadioButton(m_surfaceWidget);
    modeNoneRB->setText(u"No selection"_s);
    modeNoneRB->setChecked(false);

    auto *modeItemRB = new QRadioButton(m_surfaceWidget);
    modeItemRB->setText(u"Item"_s);
    modeItemRB->setChecked(false);

    auto *modeSliceRowRB = new QRadioButton(m_surfaceWidget);
    modeSliceRowRB->setText(u"Row Slice"_s);
    modeSliceRowRB->setChecked(false);

    auto *modeSliceColumnRB = new QRadioButton(m_surfaceWidget);
    modeSliceColumnRB->setText(u"Column Slice"_s);
    modeSliceColumnRB->setChecked(false);

    auto *selectionVBox = new QVBoxLayout;
    selectionVBox->addWidget(modeNoneRB);
    selectionVBox->addWidget(modeItemRB);
    selectionVBox->addWidget(modeSliceRowRB);
    selectionVBox->addWidget(modeSliceColumnRB);
    selectionGroupBox->setLayout(selectionVBox);

    auto *axisGroupBox = new QGroupBox(u"Axis ranges"_s);

    auto *axisMinSliderX = new QSlider(Qt::Horizontal);
    axisMinSliderX->setMinimum(0);
    axisMinSliderX->setTickInterval(1);
    axisMinSliderX->setEnabled(true);
    auto *axisMaxSliderX = new QSlider(Qt::Horizontal);
    axisMaxSliderX->setMinimum(1);
    axisMaxSliderX->setTickInterval(1);
    axisMaxSliderX->setEnabled(true);
    auto *axisMinSliderZ = new QSlider(Qt::Horizontal);
    axisMinSliderZ->setMinimum(0);
    axisMinSliderZ->setTickInterval(1);
    axisMinSliderZ->setEnabled(true);
    auto *axisMaxSliderZ = new QSlider(Qt::Horizontal);
    axisMaxSliderZ->setMinimum(1);
    axisMaxSliderZ->setTickInterval(1);
    axisMaxSliderZ->setEnabled(true);

    auto *axisVBox = new QVBoxLayout;
    axisVBox->addWidget(new QLabel(u"Column range"_s));
    axisVBox->addWidget(axisMinSliderX);
    axisVBox->addWidget(axisMaxSliderX);
    axisVBox->addWidget(new QLabel(u"Row range"_s));
    axisVBox->addWidget(axisMinSliderZ);
    axisVBox->addWidget(axisMaxSliderZ);
    axisGroupBox->setLayout(axisVBox);

    // Mode-dependent controls
    // sqrt-sin
    auto *colorGroupBox = new QGroupBox(u"Custom gradient"_s);

    QPixmap pixmap = gradientBtoYPB_Pixmap();
    auto *gradientBtoYPB = new QPushButton(m_surfaceWidget);
    gradientBtoYPB->setIcon(QIcon(pixmap));
    gradientBtoYPB->setIconSize(pixmap.size());

    pixmap = gradientGtoRPB_Pixmap();
    auto *gradientGtoRPB = new QPushButton(m_surfaceWidget);
    gradientGtoRPB->setIcon(QIcon(pixmap));
    gradientGtoRPB->setIconSize(pixmap.size());
    auto *colorHBox = new QHBoxLayout;
    colorHBox->addWidget(gradientBtoYPB);
    colorHBox->addWidget(gradientGtoRPB);
    colorGroupBox->setLayout(colorHBox);

    // Multiseries heightmap
    auto *showGroupBox = new QGroupBox(u"_show Object"_s);
    showGroupBox->setVisible(false);

    auto *checkboxShowOilRigOne = new QCheckBox(u"Oil Rig 1"_s);
    checkboxShowOilRigOne->setChecked(true);

    auto *checkboxShowOilRigTwo = new QCheckBox(u"Oil Rig 2"_s);
    checkboxShowOilRigTwo->setChecked(true);

    auto *checkboxShowRefinery = new QCheckBox(u"Refinery"_s);

    auto *showVBox = new QVBoxLayout;
    showVBox->addWidget(checkboxShowOilRigOne);
    showVBox->addWidget(checkboxShowOilRigTwo);
    showVBox->addWidget(checkboxShowRefinery);
    showGroupBox->setLayout(showVBox);

    auto *visualsGroupBox = new QGroupBox(u"Visuals"_s);
    visualsGroupBox->setVisible(false);

    auto *checkboxVisualsSeeThrough = new QCheckBox(u"See-Through"_s);

    auto *checkboxHighlightOil = new QCheckBox(u"Highlight Oil"_s);

    auto *checkboxShowShadows = new QCheckBox(u"Shadows"_s);
    checkboxShowShadows->setChecked(true);

    auto *visualVBox = new QVBoxLayout;
    visualVBox->addWidget(checkboxVisualsSeeThrough);
    visualVBox->addWidget(checkboxHighlightOil);
    visualVBox->addWidget(checkboxShowShadows);
    visualsGroupBox->setLayout(visualVBox);

    auto *labelSelection = new QLabel(u"Selection:"_s);
    labelSelection->setVisible(false);

    auto *labelSelectedItem = new QLabel(u"Nothing"_s);
    labelSelectedItem->setVisible(false);

    // Textured topography heightmap
    auto *enableTexture = new QCheckBox(u"Surface texture"_s);
    enableTexture->setVisible(false);

    auto *label = new QLabel(m_surfaceWidget);
    label->setPixmap(highlightPixmap());

    auto *heightMapGroupBox = new QGroupBox(u"Highlight color map"_s);
    auto *colorMapVBox = new QVBoxLayout;
    colorMapVBox->addWidget(label);
    heightMapGroupBox->setLayout(colorMapVBox);
    heightMapGroupBox->setVisible(false);

    // Populate vertical layout
    // Common
    vLayout->addWidget(modelGroupBox);
    vLayout->addWidget(selectionGroupBox);
    vLayout->addWidget(axisGroupBox);

    // Sqrt Sin
    vLayout->addWidget(colorGroupBox);

    // Multiseries heightmap
    vLayout->addWidget(showGroupBox);
    vLayout->addWidget(visualsGroupBox);
    vLayout->addWidget(labelSelection);
    vLayout->addWidget(labelSelectedItem);

    // Textured topography
    vLayout->addWidget(heightMapGroupBox);
    vLayout->addWidget(enableTexture);

    // Create the controller
    auto *modifier = new SurfaceGraphModifier(m_surfaceGraph, labelSelectedItem, this);

    // Connect widget controls to controller
    QObject::connect(heightMapModelRB, &QRadioButton::toggled,
                     modifier, &SurfaceGraphModifier::enableHeightMapModel);
    QObject::connect(sqrtSinModelRB, &QRadioButton::toggled,
                     modifier, &SurfaceGraphModifier::enableSqrtSinModel);
    QObject::connect(texturedModelRB, &QRadioButton::toggled,
                     modifier, &SurfaceGraphModifier::enableTopographyModel);

    QObject::connect(modeNoneRB, &QRadioButton::toggled,
                     modifier, &SurfaceGraphModifier::toggleModeNone);
    QObject::connect(modeItemRB,  &QRadioButton::toggled,
                     modifier, &SurfaceGraphModifier::toggleModeItem);
    QObject::connect(modeSliceRowRB,  &QRadioButton::toggled,
                     modifier, &SurfaceGraphModifier::toggleModeSliceRow);
    QObject::connect(modeSliceColumnRB,  &QRadioButton::toggled,
                     modifier, &SurfaceGraphModifier::toggleModeSliceColumn);

    QObject::connect(axisMinSliderX, &QSlider::valueChanged,
                     modifier, &SurfaceGraphModifier::adjustXMin);
    QObject::connect(axisMaxSliderX, &QSlider::valueChanged,
                     modifier, &SurfaceGraphModifier::adjustXMax);
    QObject::connect(axisMinSliderZ, &QSlider::valueChanged,
                     modifier, &SurfaceGraphModifier::adjustZMin);
    QObject::connect(axisMaxSliderZ, &QSlider::valueChanged,
                     modifier, &SurfaceGraphModifier::adjustZMax);

    // Mode dependent connections
    QObject::connect(gradientBtoYPB, &QPushButton::pressed,
                     modifier, &SurfaceGraphModifier::setBlackToYellowGradient);
    QObject::connect(gradientGtoRPB, &QPushButton::pressed,
                     modifier, &SurfaceGraphModifier::setGreenToRedGradient);

    QObject::connect(checkboxShowOilRigOne, &QCheckBox::stateChanged,
                     modifier, &SurfaceGraphModifier::toggleItemOne);
    QObject::connect(checkboxShowOilRigTwo, &QCheckBox::stateChanged,
                     modifier, &SurfaceGraphModifier::toggleItemTwo);
    QObject::connect(checkboxShowRefinery, &QCheckBox::stateChanged,
                     modifier, &SurfaceGraphModifier::toggleItemThree);

    QObject::connect(checkboxVisualsSeeThrough, &QCheckBox::stateChanged,
                     modifier, &SurfaceGraphModifier::toggleSeeThrough);
    QObject::connect(checkboxHighlightOil, &QCheckBox::stateChanged,
                     modifier, &SurfaceGraphModifier::toggleOilHighlight);
    QObject::connect(checkboxShowShadows, &QCheckBox::stateChanged,
                     modifier, &SurfaceGraphModifier::toggleShadows);

    QObject::connect(enableTexture, &QCheckBox::stateChanged,
                     modifier, &SurfaceGraphModifier::toggleSurfaceTexture);

    // Connections to disable features depending on mode
    QObject::connect(sqrtSinModelRB, &QRadioButton::toggled,
                     colorGroupBox, &QGroupBox::setVisible);

    QObject::connect(heightMapModelRB, &QRadioButton::toggled,
                     showGroupBox, &QGroupBox::setVisible);
    QObject::connect(heightMapModelRB, &QRadioButton::toggled,
                     visualsGroupBox, &QGroupBox::setVisible);
    QObject::connect(heightMapModelRB, &QRadioButton::toggled,
                     labelSelection, &QLabel::setVisible);
    QObject::connect(heightMapModelRB, &QRadioButton::toggled,
                     labelSelectedItem, &QLabel::setVisible);

    QObject::connect(texturedModelRB, &QRadioButton::toggled,
                     enableTexture, &QLabel::setVisible);
    QObject::connect(texturedModelRB, &QRadioButton::toggled,
                     heightMapGroupBox, &QGroupBox::setVisible);

    modifier->setAxisMinSliderX(axisMinSliderX);
    modifier->setAxisMaxSliderX(axisMaxSliderX);
    modifier->setAxisMinSliderZ(axisMinSliderZ);
    modifier->setAxisMaxSliderZ(axisMaxSliderZ);

    sqrtSinModelRB->setChecked(true);
    modeItemRB->setChecked(true);
    enableTexture->setChecked(true);

    return true;
}
