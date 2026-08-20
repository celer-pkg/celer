// Copyright (C) 2015 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef AX1_H
#define AX1_H

#include <QWidget>
#include <QPainter>

//! [0]
class QAxWidget1 : public QWidget
{
    Q_OBJECT
    Q_CLASSINFO("ClassID", "{1D9928BD-4453-4bdd-903D-E525ED17FDE5}")
    Q_CLASSINFO("InterfaceID", "{99F6860E-2C5A-42ec-87F2-43396F4BE389}")
    Q_CLASSINFO("EventsID", "{0A3E9F27-E4F1-45bb-9E47-63099BCCD0E3}")

    Q_PROPERTY(QColor fillColor READ fillColor WRITE setFillColor)
public:
    explicit QAxWidget1(QWidget *parent = nullptr)
        : QWidget(parent)
    {
    }

    QColor fillColor() const
    {
        return m_fillColor;
    }

    void setFillColor(const QColor &fc)
    {
        m_fillColor = fc;
        repaint();
    }

protected:
    void paintEvent(QPaintEvent *e) override
    {
        QPainter paint(this);
        QRect r = rect();
        r.adjust(10, 10, -10, -10);
        paint.fillRect(r, m_fillColor);
    }

private:
    QColor m_fillColor = Qt::red;
};
//! [0]

#endif // AX1_H
