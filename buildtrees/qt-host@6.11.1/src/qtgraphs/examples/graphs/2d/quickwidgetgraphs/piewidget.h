// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef PIEWIDGET_H
#define PIEWIDGET_H

#include <QWidget>
#include <QQuickWidget>
#include <QHBoxLayout>
#include <QPushButton>
#include <QQmlContext>

class PieGraph;

class PieWidget : public QWidget
{
    Q_OBJECT
public:
    explicit PieWidget(QWidget *parent = nullptr);
    ~PieWidget();

    void initializeButtons();
    void initializeQuickWidget();

    QWidget *containerWidget() const;
private:
    QWidget *m_widget;
    QQuickWidget *m_quickWidget;
    QVBoxLayout *m_vLayout;
    QHBoxLayout *m_hLayout;

    PieGraph *m_pieGraph;
};

#endif // PIEWIDGET_H
