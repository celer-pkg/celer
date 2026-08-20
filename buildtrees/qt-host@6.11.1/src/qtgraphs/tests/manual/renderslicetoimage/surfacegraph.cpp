// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "surfacegraph.h"
#include "surfacegraphmodifier.h"
#include "surfacegraphwidget.h"

#include <QtCore/qregularexpression.h>
#include <QtGui/qvalidator.h>
#include <QtWidgets/qboxlayout.h>
#include <QtWidgets/qcheckbox.h>
#include <QtWidgets/qlabel.h>
#include <QtWidgets/qlineedit.h>
#include <QtWidgets/qpushbutton.h>
#include <QtWidgets/qradiobutton.h>

using namespace Qt::StringLiterals;

SurfaceGraph::SurfaceGraph(QWidget *parent)
{
    m_surfaceWidget = new QWidget(parent);
    initialize();
}

SurfaceGraph::~SurfaceGraph()
{
    delete m_surfaceWidget;
}

void SurfaceGraph::initialize()
{
    m_surfaceGraphWidget = new SurfaceGraphWidget();
    m_surfaceGraphWidget->initialize();
    QSize screenSize = m_surfaceGraphWidget->screen()->size();
    m_surfaceGraphWidget->setMinimumSize(QSize(screenSize.width() / 2, screenSize.height() / 1.75));
    m_surfaceGraphWidget->setMaximumSize(screenSize);
    m_surfaceGraphWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_surfaceGraphWidget->setFocusPolicy(Qt::StrongFocus);

    QHBoxLayout *hLayout = new QHBoxLayout;
    hLayout->addWidget(m_surfaceGraphWidget, 1);
    m_surfaceWidget->setLayout(hLayout);

    QVBoxLayout *vLayout = new QVBoxLayout;
    hLayout->addLayout(vLayout);
    vLayout->setAlignment(Qt::AlignCenter);

    m_pickCheckBox = new QCheckBox(m_surfaceWidget);
    m_pickCheckBox->setChecked(false);
    m_pickCheckBox->setText(u"Use for Picking"_s);

    m_invertCheckBox = new QCheckBox(m_surfaceWidget);
    m_invertCheckBox->setChecked(false);
    m_invertCheckBox->setText(u"Invert Picking (Row <-> Column)"_s);

    m_rowRadioButton = new QRadioButton(m_surfaceWidget);
    m_rowRadioButton->setText(u"Row"_s);
    m_rowRadioButton->setChecked(true);

    QRadioButton *columnRadioButton = new QRadioButton(m_surfaceWidget);
    columnRadioButton->setText(u"Column"_s);
    columnRadioButton->setChecked(false);

    m_lineSelectText = new QLineEdit(m_surfaceWidget);
    QRegularExpression re("\\d+");
    QRegularExpressionValidator *reValidator = new QRegularExpressionValidator(re, m_surfaceWidget);
    m_lineSelectText->setValidator(reValidator);

    QPushButton *sliceToImageButton = new QPushButton(m_surfaceWidget);
    sliceToImageButton->setText(u"Slice To Image"_s);

    m_sliceResultLabel = new QLabel(m_surfaceWidget);

    vLayout->addWidget(m_pickCheckBox);
    vLayout->addWidget(m_invertCheckBox);
    vLayout->addWidget(m_rowRadioButton);
    vLayout->addWidget(columnRadioButton);
    vLayout->addWidget(m_lineSelectText);
    vLayout->addWidget(sliceToImageButton);
    vLayout->addWidget(m_sliceResultLabel);

    m_surfaceGraphWidget->raise();

    m_modifier = new SurfaceGraphModifier(m_surfaceGraphWidget->surfaceGraph(), this);

    QObject::connect(sliceToImageButton,
                     &QPushButton::clicked,
                     this,
                     &SurfaceGraph::renderSliceToImage);
    QObject::connect(m_modifier,
                     &SurfaceGraphModifier::updateSliceImage,
                     this,
                     &SurfaceGraph::applySliceImage);
    QObject::connect(m_rowRadioButton,
                     &QRadioButton::clicked,
                     this,
                     &SurfaceGraph::changeSelectionMode);
    QObject::connect(columnRadioButton,
                     &QRadioButton::clicked,
                     this,
                     &SurfaceGraph::changeSelectionMode);
    QObject::connect(m_pickCheckBox, &QCheckBox::clicked, this, &SurfaceGraph::changeSelectionMode);
}

void SurfaceGraph::renderSliceToImage()
{
    int index = m_lineSelectText->text().isEmpty() ? -1 : m_lineSelectText->text().toInt();
    QtGraphs3D::SliceCaptureType sliceType = QtGraphs3D::SliceCaptureType::RowImage;
    if (!m_rowRadioButton->isChecked())
        sliceType = QtGraphs3D::SliceCaptureType::ColumnImage;

    m_modifier->renderSliceToImage(sliceType, index);
}

void SurfaceGraph::applySliceImage(QImage image)
{
    m_sliceResultLabel->setPixmap(QPixmap::fromImage(image));
}

void SurfaceGraph::changeSelectionMode(bool checked)
{
    Q_UNUSED(checked)
    bool mode = m_invertCheckBox->isChecked() ? !m_rowRadioButton->isChecked()
                                              : m_rowRadioButton->isChecked();
    if (m_pickCheckBox->isChecked())
        m_modifier->changeSelectionMode(mode, false);
    else
        m_modifier->changeSelectionMode(false);
}
