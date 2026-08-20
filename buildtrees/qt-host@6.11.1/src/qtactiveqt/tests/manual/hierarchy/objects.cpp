// Copyright (C) 2015 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "objects.h"
#include <QLayout>
#include <QPainter>

/* Implementation of QParentWidget */
//! [0]
QParentWidget::QParentWidget(QWidget *parent)
: QWidget(parent),
  m_vbox(new QVBoxLayout(this))
{
}

//! [0] //! [1]
void QParentWidget::createSubWidget(const QString &name)
{
    QSubWidget *sw = new QSubWidget(this, name);
    m_vbox->addWidget(sw);
    sw->setLabel(name);
    sw->show();
}

//! [1] //! [2]
QSubWidget *QParentWidget::subWidget(const QString &name)
{
    return findChild<QSubWidget *>(name);
}

//! [2]
QSize QParentWidget::sizeHint() const
{
    return QWidget::sizeHint().expandedTo(QSize(100, 100));
}

/* Implementation of QSubWidget */
//! [3]
QSubWidget::QSubWidget(QWidget *parent, const QString &name)
: QWidget(parent)
{
    setObjectName(name);
}

void QSubWidget::setLabel(const QString &text)
{
    m_label = text;
    setObjectName(text);
    update();
}

QString QSubWidget::label() const
{
    return m_label;
}

QSize QSubWidget::sizeHint() const
{
    QFontMetrics fm(font());
    return QSize(fm.horizontalAdvance(m_label), fm.height());
}

void QSubWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setPen(palette().text().color());
    painter.drawText(rect(), Qt::AlignCenter, m_label);
//! [3] //! [4]
}
//! [4]
