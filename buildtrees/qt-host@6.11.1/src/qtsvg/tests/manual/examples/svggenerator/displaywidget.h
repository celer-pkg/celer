// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef DISPLAYWIDGET_H
#define DISPLAYWIDGET_H

#include <QColor>
#include <QHash>
#include <QPainterPath>
#include <QWidget>

//! [DisplayWidget class definition]
class DisplayWidget : public QWidget
{
    Q_OBJECT

public:
    enum Shape { House = 0, Car = 1 };
    enum Background { Sky = 0, Trees = 1, Road = 2 };

    DisplayWidget(QWidget *parent = 0);
    QColor color() const;
    void paint(QPainter &painter);

public slots:
    void setBackground(Background background);
    void setColor(const QColor &color);
    void setShape(Shape shape);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    Background background;
    QColor shapeColor;
    Shape shape;
    QHash<Shape,QPainterPath> shapeMap;
    QPainterPath moon;
    QPainterPath tree;
};
//! [DisplayWidget class definition]

#endif
