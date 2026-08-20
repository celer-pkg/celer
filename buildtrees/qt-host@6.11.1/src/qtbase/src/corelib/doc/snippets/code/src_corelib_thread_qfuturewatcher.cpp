// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QFutureWatcher>
#include <QtConcurrent>

class MyClass : public QObject
{
    Q_OBJECT
public:
signals:
    void handleFinished();
};

void examples()
{
    int result;

    //! [0]
    // Instantiate the objects and connect to the finished signal.
    MyClass myObject;
    QFutureWatcher<int> watcher;
    QObject::connect(&watcher, &QFutureWatcher<int>::finished, &myObject, &MyClass::handleFinished);

    // Start the computation.
    QFuture<int> future = QtConcurrent::run([result](){ /*...*/ return result;});
    watcher.setFuture(future);
    //! [0]
}
