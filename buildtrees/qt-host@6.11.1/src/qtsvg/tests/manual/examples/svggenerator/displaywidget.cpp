// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QtWidgets>
#include "displaywidget.h"

DisplayWidget::DisplayWidget(QWidget *parent)
    : QWidget(parent)
{
    QPainterPath car;
    QPainterPath house;

    QFile file(":resources/shapes.dat");
    file.open(QFile::ReadOnly);
    QDataStream stream(&file);
    stream >> car >> house >> tree >> moon;
    file.close();

    shapeMap[Car] = car;
    shapeMap[House] = house;

    background = Sky;
    shapeColor = Qt::darkYellow;
    shape = House;
}

//! [paint event]
void DisplayWidget::paintEvent(QPaintEvent * /* event */)
{
    QPainter painter;
    painter.begin(this);
    painter.setRenderHint(QPainter::Antialiasing);
    paint(painter);
    painter.end();
}
//! [paint event]

//! [paint function]
void DisplayWidget::paint(QPainter &painter)
{
//![paint picture]
    painter.setClipRect(QRect(0, 0, 200, 200));
    painter.setPen(Qt::NoPen);

    switch (background) {
    case Sky:
    default:
        painter.fillRect(QRect(0, 0, 200, 200), Qt::darkBlue);
        painter.translate(145, 10);
        painter.setBrush(Qt::white);
        painter.drawPath(moon);
        painter.translate(-145, -10);
        break;
    case Trees:
    {
        painter.fillRect(QRect(0, 0, 200, 200), Qt::darkGreen);
        painter.setBrush(Qt::green);
        painter.setPen(Qt::black);
        for (int y = -55, row = 0; y < 200; y += 50, ++row) {
            int xs;
            if (row == 2 || row == 3)
                xs = 150;
            else
                xs = 50;
            for (int x = 0; x < 200; x += xs) {
                painter.save();
                painter.translate(x, y);
                painter.drawPath(tree);
                painter.restore();
            }
        }
        break;
    }
    case Road:
        painter.fillRect(QRect(0, 0, 200, 200), Qt::gray);
        painter.setPen(QPen(Qt::white, 4, Qt::DashLine));
        painter.drawLine(QLine(0, 35, 200, 35));
        painter.drawLine(QLine(0, 165, 200, 165));
        break;
    }

    painter.setBrush(shapeColor);
    painter.setPen(Qt::black);
    painter.translate(100, 100);
    painter.drawPath(shapeMap[shape]);
//![paint picture]
}
//! [paint function]

QColor DisplayWidget::color() const
{
    return shapeColor;
}

void DisplayWidget::setBackground(Background background)
{
    this->background = background;
    update();
}

void DisplayWidget::setColor(const QColor &color)
{
    this->shapeColor = color;
    update();
}

void DisplayWidget::setShape(Shape shape)
{
    this->shape = shape;
    update();
}
