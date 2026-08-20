// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QFutureSynchronizer>
#include <QtConcurrent>
#include <QList>

auto anotherFunction = []() {};
auto mapFunction = [](int &value) { value *= 2; };
QList<int> list = {1, 2, 3, 4, 5};

//! [0]
void someFunction()
{
    QFutureSynchronizer<void> synchronizer;

    //...

    synchronizer.addFuture(QtConcurrent::run(anotherFunction));
    synchronizer.addFuture(QtConcurrent::map(list, mapFunction));

    return; // QFutureSynchronizer waits for all futures to finish
}
//! [0]
