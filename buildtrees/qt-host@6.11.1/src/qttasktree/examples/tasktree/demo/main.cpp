// Copyright (C) 2025 Jarek Kobus
// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "taskwidget.h"

#include <QSingleTaskTreeRunner>
#include <QTaskTree>

#include <QApplication>
#include <QBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QProgressBar>
#include <QScrollArea>
#include <QTimer>
#include <QToolButton>

using namespace QtTaskTree;

using namespace std::chrono;

static QWidget *hr()
{
    auto frame = new QFrame;
    frame->setFrameShape(QFrame::HLine);
    frame->setFrameShadow(QFrame::Sunken);
    return frame;
}

static State resultToState(DoneWith result)
{
    switch (result) {
    case DoneWith::Success: return State::Success;
    case DoneWith::Error: return State::Error;
    case DoneWith::Cancel: return State::Canceled;
    }
    return State::Initial;
}

struct GroupSetup
{
    WorkflowPolicy policy = WorkflowPolicy::StopOnError;
    ExecuteMode mode = ExecuteMode::Sequential;
};

//! [0]
class GlueItem
{
public:
    virtual ExecutableItem recipe() const = 0;
    virtual QWidget *widget() const = 0;
    virtual void reset() const = 0;

    virtual ~GlueItem() = default;
};
//! [0]

class GroupGlueItem final : public GlueItem
{
public:
    GroupGlueItem(const GroupSetup &setup, const QList<GlueItem *> &children)
        : m_children(children)
        , m_groupWidget(new GroupWidget)
        , m_widget(new QWidget)
    {
        QBoxLayout *layout = new QHBoxLayout(m_widget);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->addWidget(m_groupWidget);
        QGroupBox *groupBox = new QGroupBox;
        QBoxLayout *subLayout = new QVBoxLayout(groupBox);
        for (int i = 0; i < children.size(); ++i) {
            if (i > 0)
                subLayout->addWidget(hr());
            subLayout->addWidget(children.at(i)->widget());
        }
        layout->addWidget(groupBox);
        m_groupWidget->setWorkflowPolicy(setup.policy);
        m_groupWidget->setExecuteMode(setup.mode);
    }

    ExecutableItem recipe() const final;
    QWidget *widget() const final { return m_widget; }
    void reset() const final {
        m_groupWidget->setState(State::Initial);
        for (GlueItem *child : m_children)
            child->reset();
    }

    ~GroupGlueItem() override { qDeleteAll(m_children); }

private:
    QList<GlueItem *> m_children;
    GroupWidget *m_groupWidget = nullptr;
    QWidget *m_widget = nullptr;
};

//! [1]
ExecutableItem GroupGlueItem::recipe() const
{
    GroupItems childRecipes;
    for (GlueItem *child : m_children)
        childRecipes.append(child->recipe());

    return Group {
        m_groupWidget->executeModeItem(),
        m_groupWidget->workflowPolicyItem(),
        onGroupSetup([this] { m_groupWidget->setState(State::Running); }),
        childRecipes,
        onGroupDone([this](DoneWith result) { m_groupWidget->setState(resultToState(result)); })
    };
}
//! [1]

class TaskGlueItem final : public GlueItem
{
public:
    TaskGlueItem(int busyTime, DoneResult result)
        : m_taskWidget(new TaskWidget(busyTime, result))
    { }

    ExecutableItem recipe() const final;
    QWidget *widget() const final { return m_taskWidget; }
    void reset() const final { m_taskWidget->setState(State::Initial); }

private:
    TaskWidget *m_taskWidget = nullptr;
};

//! [2]
ExecutableItem TaskGlueItem::recipe() const
{
    return Group {
        onGroupSetup([this] { m_taskWidget->setState(State::Running); }),
        timeoutTask(seconds(m_taskWidget->busyTime()), m_taskWidget->desiredResult()),
        onGroupDone([this](DoneWith result) { m_taskWidget->setState(resultToState(result)); })
    };
}
//! [2]

//! [3]
static GlueItem *group(const GroupSetup &groupSetup, const QList<GlueItem *> children)
{
    return new GroupGlueItem(groupSetup, children);
}

static GlueItem *task(int busyTime = 1, DoneResult result = DoneResult::Success)
{
    return new TaskGlueItem(busyTime, result);
}
//! [3]

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QWidget mainWidget;
    mainWidget.setWindowTitle(QApplication::tr("Task Tree Demo"));

    QToolButton *startButton = new QToolButton;
    startButton->setText(QApplication::tr("Start"));
    QToolButton *cancelButton = new QToolButton;
    cancelButton->setText(QApplication::tr("Cancel"));
    cancelButton->setEnabled(false);
    QToolButton *resetButton = new QToolButton;
    resetButton->setText(QApplication::tr("Reset"));
    QProgressBar *progressBar = new QProgressBar;

//! [4]
    std::unique_ptr<GlueItem> tree {
        group({WorkflowPolicy::ContinueOnSuccess}, {
            group({}, {
                task(),
                task(2, DoneResult::Error),
                task(3)
            }),
            task(),
            task(),
            group({WorkflowPolicy::FinishAllAndSuccess}, {
                task(),
                task(),
                group({WorkflowPolicy::StopOnError, ExecuteMode::Parallel}, {
                    task(4),
                    task(2),
                    task(1),
                    task(3, DoneResult::Error)
                }),
                task(2),
                task(3)
            }),
            task()
        })
    };
//! [4]

    {
        QScrollArea *scrollArea = new QScrollArea;
        scrollArea->setWidgetResizable(true);
        QWidget *scrollAreaWidget = new QWidget;
        QBoxLayout *scrollLayout = new QVBoxLayout(scrollAreaWidget);
        scrollLayout->addWidget(tree->widget());
        scrollLayout->addStretch();
        scrollArea->setWidget(scrollAreaWidget);

        QBoxLayout *mainLayout = new QVBoxLayout(&mainWidget);
        QBoxLayout *subLayout = new QHBoxLayout;
        subLayout->addWidget(startButton);
        subLayout->addWidget(cancelButton);
        subLayout->addWidget(resetButton);
        subLayout->addWidget(progressBar);
        mainLayout->addLayout(subLayout);
        mainLayout->addWidget(hr());
        mainLayout->addWidget(scrollArea);
        mainLayout->addWidget(hr());
        QBoxLayout *footerLayout = new QHBoxLayout;
        footerLayout->addWidget(new StateLabel(State::Initial));
        footerLayout->addWidget(new StateLabel(State::Running));
        footerLayout->addWidget(new StateLabel(State::Success));
        footerLayout->addWidget(new StateLabel(State::Error));
        footerLayout->addWidget(new StateLabel(State::Canceled));
        mainLayout->addLayout(footerLayout);

        const int margin = 4;
        scrollArea->setMinimumSize(scrollAreaWidget->minimumSizeHint().grownBy(
            {0, 0, margin, margin}));
        QTimer::singleShot(0, scrollArea, [scrollArea] { scrollArea->setMinimumSize({0, 0}); });
    }

//! [5]
    QSingleTaskTreeRunner taskTreeRunner;

    const auto resetTaskTree = [&] {
        taskTreeRunner.reset();
        tree->reset();
        progressBar->setValue(0);
        cancelButton->setEnabled(false);
    };

    const auto cancelTaskTree = [&] { taskTreeRunner.cancel(); };

    const auto startTaskTree = [&] {
        resetTaskTree();
        cancelButton->setEnabled(true);

        const auto onTaskTreeSetup = [progressBar](QTaskTree &taskTree) {
            progressBar->setMaximum(taskTree.progressMaximum());
            QObject::connect(&taskTree, &QTaskTree::progressValueChanged,
                             progressBar, &QProgressBar::setValue);
        };

        const auto onTaskTreeDone = [cancelButton] { cancelButton->setEnabled(false); };

        taskTreeRunner.start({tree->recipe()}, onTaskTreeSetup, onTaskTreeDone);
    };

    QObject::connect(startButton, &QAbstractButton::clicked, &app, startTaskTree);
    QObject::connect(cancelButton, &QAbstractButton::clicked, &app, cancelTaskTree);
    QObject::connect(resetButton, &QAbstractButton::clicked, &app, resetTaskTree);

    mainWidget.show();

    return app.exec();
//! [5]
}
