// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "bargraph.h"
#include "bargraphmodifier.h"
#include "bargraphwidget.h"

#include <QtCore/qregularexpression.h>
#include <QtCore/qtimer.h>
#include <QtGui/qvalidator.h>
#include <QtQuick/qquickitemgrabresult.h>
#include <QtWidgets/qboxlayout.h>
#include <QtWidgets/qcheckbox.h>
#include <QtWidgets/qlabel.h>
#include <QtWidgets/qlineedit.h>
#include <QtWidgets/qpushbutton.h>
#include <QtWidgets/qradiobutton.h>

using namespace Qt::StringLiterals;

BarGraph::BarGraph(QWidget *parent)
{
    m_barWidget = new QWidget(parent);
    initialize();
}

BarGraph::~BarGraph()
{
    delete m_barWidget;
}

void BarGraph::initialize()
{
    m_barGraphWidget = new BarGraphWidget();
    m_barGraphWidget->initialize();
    QSize screenSize = m_barGraphWidget->screen()->size();
    m_barGraphWidget->setMinimumSize(QSize(screenSize.width() / 2, screenSize.height() / 1.75));
    m_barGraphWidget->setMaximumSize(screenSize);
    m_barGraphWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_barGraphWidget->setFocusPolicy(Qt::StrongFocus);

    QHBoxLayout *hLayout = new QHBoxLayout;
    hLayout->addWidget(m_barGraphWidget, 1);
    m_barWidget->setLayout(hLayout);

    QVBoxLayout *vLayout = new QVBoxLayout;
    hLayout->addLayout(vLayout);
    vLayout->setAlignment(Qt::AlignCenter);

    m_pickCheckBox = new QCheckBox(m_barWidget);
    m_pickCheckBox->setChecked(false);
    m_pickCheckBox->setText(u"Use for Picking"_s);

    m_invertCheckBox = new QCheckBox(m_barWidget);
    m_invertCheckBox->setChecked(false);
    m_invertCheckBox->setText(u"Invert Picking (Row <-> Column)"_s);

    m_rowRadioButton = new QRadioButton(m_barWidget);
    m_rowRadioButton->setText(u"Row"_s);
    m_rowRadioButton->setChecked(true);

    QRadioButton *columnRadioButton = new QRadioButton(m_barWidget);
    columnRadioButton->setText(u"Column"_s);
    columnRadioButton->setChecked(false);

    m_lineSelectText = new QLineEdit(m_barWidget);
    QRegularExpression re("\\d+");
    QRegularExpressionValidator *reValidator = new QRegularExpressionValidator(re, m_barWidget);
    m_lineSelectText->setValidator(reValidator);

    QPushButton *sliceToImageButton = new QPushButton(m_barWidget);
    sliceToImageButton->setText(u"Slice To Image"_s);

    m_sliceResultLabel = new QLabel(m_barWidget);

    vLayout->addWidget(m_pickCheckBox);
    vLayout->addWidget(m_invertCheckBox);
    vLayout->addWidget(m_rowRadioButton);
    vLayout->addWidget(columnRadioButton);
    vLayout->addWidget(m_lineSelectText);
    vLayout->addWidget(sliceToImageButton);
    vLayout->addWidget(m_sliceResultLabel);

    m_barGraphWidget->raise();

    m_modifier = new BarGraphModifier(m_barGraphWidget->barGraph(), this);

    QObject::connect(sliceToImageButton, &QPushButton::clicked, this, &BarGraph::renderSliceToImage);

    QObject::connect(m_rowRadioButton, &QRadioButton::clicked, this, &BarGraph::changeSelectionMode);
    QObject::connect(columnRadioButton,
                     &QRadioButton::clicked,
                     this,
                     &BarGraph::changeSelectionMode);
    QObject::connect(m_pickCheckBox, &QCheckBox::clicked, this, &BarGraph::changeSelectionMode);
}

void BarGraph::renderSliceToImage()
{
    int index = m_lineSelectText->text().isEmpty() ? -1 : m_lineSelectText->text().toInt();
    QtGraphs3D::SliceCaptureType sliceType = QtGraphs3D::SliceCaptureType::RowImage;
    if (!m_rowRadioButton->isChecked())
        sliceType = QtGraphs3D::SliceCaptureType::ColumnImage;

    connect(m_modifier, &BarGraphModifier::sliceImageChanged, this, [this](const QImage &image) {
        m_sliceResultLabel->setPixmap(QPixmap::fromImage(image));
    });
    m_modifier->renderSliceToImage(sliceType, index);
}

void BarGraph::changeSelectionMode(bool checked)
{
    Q_UNUSED(checked)
    bool mode = m_invertCheckBox->isChecked() ? !m_rowRadioButton->isChecked()
                                              : m_rowRadioButton->isChecked();
    if (m_pickCheckBox->isChecked())
        m_modifier->changeSelectionMode(mode, false);
    else
        m_modifier->changeSelectionMode(false);
}
