// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QColorDialog>
#include <QFileDialog>
#include <QPainter>
#include <QSvgGenerator>
#include "window.h"
#include "displaywidget.h"

Window::Window(QWidget *parent)
    : QWidget(parent)
{
    setupUi(this);
    connect(shapeComboBox, &QComboBox::currentIndexChanged, this, &Window::updateShape);
    connect(colorButton, &QToolButton::clicked, this, &Window::updateColor);
    connect(shapeComboBox_2, &QComboBox::currentIndexChanged, this, &Window::updateBackground);
    connect(toolButton_2, &QToolButton::clicked, this, &Window::saveSvg);
}

void Window::updateBackground(int background)
{
    displayWidget->setBackground(DisplayWidget::Background(background));
}

void Window::updateColor()
{
    QColor color = QColorDialog::getColor(displayWidget->color());
    if (color.isValid())
        displayWidget->setColor(color);
}

void Window::updateShape(int shape)
{
    displayWidget->setShape(DisplayWidget::Shape(shape));
}

//! [save SVG]
void Window::saveSvg()
{
    QString newPath = QFileDialog::getSaveFileName(this, tr("Save SVG"),
        path, tr("SVG files (*.svg)"));

    if (newPath.isEmpty())
        return;

    path = newPath;

//![configure SVG generator]
    QSvgGenerator generator;
    generator.setFileName(path);
    generator.setSize(QSize(200, 200));
    generator.setViewBox(QRect(0, 0, 200, 200));
    generator.setTitle(tr("SVG Generator Example Drawing"));
    generator.setDescription(tr("An SVG drawing created by the SVG Generator "
                                "Example provided with Qt."));
//![configure SVG generator]
//![begin painting]
    QPainter painter;
    painter.begin(&generator);
//![begin painting]
    displayWidget->paint(painter);
//![end painting]
    painter.end();
//![end painting]
}
//! [save SVG]
