// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtWidgets/qwidget.h>

QT_BEGIN_NAMESPACE
class QToolButton;
class QScxmlStateMachine;
class QLabel;
class QComboBox;
QT_END_NAMESPACE

class MainWindow : public QWidget
{
    Q_OBJECT

public:
    explicit MainWindow(QScxmlStateMachine *machine, QWidget *parent = nullptr);

private:
    QScxmlStateMachine *m_machine = nullptr;
    QList<QList<QToolButton *>> m_buttons;
    QToolButton *m_startButton = nullptr;
    QToolButton *m_undoButton = nullptr;
    QLabel *m_label = nullptr;
    QComboBox *m_chooser = nullptr;
};

#endif // MAINWINDOW_H
