// Copyright (C) 2015 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef AX2_H
#define AX2_H

#include <QWidget>
#include <QPainter>

//! [0]
class QAxWidget2 : public QWidget
{
    Q_OBJECT
    Q_CLASSINFO("ClassID", "{58139D56-6BE9-4b17-937D-1B1EDEDD5B71}")
    Q_CLASSINFO("InterfaceID", "{B66280AB-08CC-4dcc-924F-58E6D7975B7D}")
    Q_CLASSINFO("EventsID", "{D72BACBA-03C4-4480-B4BB-DE4FE3AA14A0}")
    Q_CLASSINFO("ToSuperClass", "QAxWidget2")
    Q_CLASSINFO("StockEvents", "yes")
    Q_CLASSINFO("Insertable", "yes")

    Q_PROPERTY(int lineWidth READ lineWidth WRITE setLineWidth)
public:
    using QWidget::QWidget;

    int lineWidth() const
    {
        return m_lineWidth;
    }

    void setLineWidth(int lw)
    {
        m_lineWidth = lw;
        repaint();
    }

protected:
    void paintEvent(QPaintEvent *e) override
    {
        QPainter paint(this);
        QPen pen = paint.pen();
        pen.setWidth(m_lineWidth);
        paint.setPen(pen);

        QRect r = rect();
        r.adjust(10, 10, -10, -10);
        paint.drawEllipse(r);
    }

private:
    int m_lineWidth = 1;
};
//! [0]

#endif // AX2_H
