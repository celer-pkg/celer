// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#undef QT_NO_FOREACH // this file contains unported legacy Q_FOREACH uses

#include <QCoreApplication>
#include <QMouseEvent>
#include <QPointF>
#include <QObject>

#if __has_include(<QtWidgets>)
#  include <QPushButton>
void example(QPointF &localPos, QPointF &globalPos, QObject *mainWindow, QObject app)
{
    //! [0]
    QMouseEvent event(QEvent::MouseButtonPress, localPos, globalPos, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QCoreApplication::sendEvent(mainWindow, &event);
    //! [0]

    //! [1]
    QPushButton *quitButton = new QPushButton("Quit");
    QObject::connect(quitButton, &QPushButton::clicked, &app, &QCoreApplication::quit, Qt::QueuedConnection);
    //! [1]
}
#endif

class MyDebugTool : public QObject
{
    public:
        MyDebugTool(QObject *parent = nullptr) : QObject(parent) {}
};

//! [3]
// Called once QCoreApplication exists
static void preRoutineMyDebugTool()
{
    MyDebugTool* tool = new MyDebugTool(QCoreApplication::instance());
    QCoreApplication::instance()->installEventFilter(tool);
}

Q_COREAPP_STARTUP_FUNCTION(preRoutineMyDebugTool)
//! [3]


//! [4]
static int *global_ptr = nullptr;

static void cleanup_ptr()
{
    delete [] global_ptr;
    global_ptr = nullptr;
}

void init_ptr()
{
    global_ptr = new int[100];      // allocate data
    qAddPostRoutine(cleanup_ptr);   // delete later
}
//! [4]


//! [5]
class MyPrivateInitStuff : public QObject
{
public:
    static MyPrivateInitStuff *initStuff(QObject *parent)
    {
        if (!p)
            p = new MyPrivateInitStuff(parent);
        return p;
    }

    ~MyPrivateInitStuff()
    {
        // cleanup goes here
    }

private:
    MyPrivateInitStuff(QObject *parent)
        : QObject(parent)
    {
        // initialization goes here
    }

    static MyPrivateInitStuff *p;
};
//! [5]


//! [6]
static inline QString tr(const char *sourceText,
                         const char *comment = nullptr);
//! [6]

class CView {};

//! [7]
class MyMfcView : public CView
{
    Q_DECLARE_TR_FUNCTIONS(MyMfcView)

public:
    MyMfcView();
    //...
};
//! [7]
