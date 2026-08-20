// Copyright (C) 2025 Jarek Kobus
// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef TASKWIDGET_H
#define TASKWIDGET_H

#include <QTaskTree>

#include <QWidget>

class StateIndicator;

QT_BEGIN_NAMESPACE
class QCheckBox;
class QComboBox;
class QLabel;
class QSpinBox;
QT_END_NAMESPACE

enum class State {
    Initial,
    Running,
    Success,
    Error,
    Canceled,
};

enum class ExecuteMode {
    Sequential, // default
    Parallel,
};

class StateWidget : public QWidget
{
public:
    explicit StateWidget(State initialState = State::Initial);
    void setState(State state);

protected:
    StateIndicator *m_stateIndicator;
};

class TaskWidget : public QWidget
{
public:
    TaskWidget(int busyTime, QtTaskTree::DoneResult result);

    void setState(State state) { m_stateWidget->setState(state); }

    int busyTime() const;
    QtTaskTree::DoneResult desiredResult() const;

private:
    StateWidget *m_stateWidget;
    QLabel *m_infoLabel;
    QSpinBox *m_spinBox;
    QCheckBox *m_checkBox;
};

class GroupWidget : public QWidget
{
public:
    GroupWidget();

    void setState(State state) { m_stateWidget->setState(state); }

    void setExecuteMode(ExecuteMode mode);
    QtTaskTree::GroupItem executeModeItem() const;

    void setWorkflowPolicy(QtTaskTree::WorkflowPolicy policy);
    QtTaskTree::GroupItem workflowPolicyItem() const;

private:
    void updateExecuteMode();
    void updateWorkflowPolicy();

    StateWidget *m_stateWidget;
    QComboBox *m_executeCombo;
    QComboBox *m_workflowCombo;

    ExecuteMode m_executeMode = ExecuteMode::Sequential;
    QtTaskTree::WorkflowPolicy m_workflowPolicy = QtTaskTree::WorkflowPolicy::StopOnError;
};

class StateLabel : public QWidget
{
public:
    explicit StateLabel(State state);
};

#endif // TASKWIDGET_H
