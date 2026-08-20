// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QQueue>
#include <iostream>

using namespace std;

void example()
{
    //! [0]
    QQueue<int> queue;
    queue.enqueue(1);
    queue.enqueue(2);
    queue.enqueue(3);
    while (!queue.isEmpty())
        cout << queue.dequeue() << endl;
    //! [0]
}
