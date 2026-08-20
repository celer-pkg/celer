// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QStyleOptionFocusRect>
#include <QPainter>

class MyWidget : public QWidget
{
    Q_OBJECT

public:
    void paintEvent(QPaintEvent *event) override;
};

void snippet(const QStyleOptionFocusRect *option)
{
    //! [0]
    const QStyleOptionFocusRect *focusRectOption =
            qstyleoption_cast<const QStyleOptionFocusRect *>(option);
    if (focusRectOption) {
        //...
    }
    //! [0]
}

//! [1]
void MyWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    //...

    QStyleOptionFocusRect option;
    option.initFrom(this);
    option.backgroundColor = palette().color(QPalette::Window);

    style()->drawPrimitive(QStyle::PE_FrameFocusRect, &option, &painter,
                          this);
}
//! [1]
