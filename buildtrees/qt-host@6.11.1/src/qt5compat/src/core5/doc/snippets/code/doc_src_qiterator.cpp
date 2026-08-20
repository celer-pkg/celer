// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
// Qt-Security score:insignificant reason:docs

//! [2]
QLinkedList<float> list;
...
QLinkedListIterator<float> i(list);
while (i.hasNext())
    qDebug() << i.next();
//! [2]


//! [3]
QLinkedListIterator<float> i(list);
i.toBack();
while (i.hasPrevious())
    qDebug() << i.previous();
//! [3]

//! [11]
QLinkedList<float> list;
...
QMutableLinkedListIterator<float> i(list);
while (i.hasNext())
    qDebug() << i.next();
//! [11]


//! [12]
QMutableLinkedListIterator<float> i(list);
i.toBack();
while (i.hasPrevious())
    qDebug() << i.previous();
//! [12]


//! [13]
QMutableLinkedListIterator<int> i(list);
while (i.hasNext()) {
    int val = i.next();
    if (val < 0) {
        i.setValue(-val);
    } else if (val == 0) {
        i.remove();
    }
}
//! [13]

//! [20]
QMutableLinkedListIterator<int> i(list);
while (i.hasNext()) {
    int val = i.next();
    if (val < -32768 || val > 32767)
        i.remove();
}
//! [20]

//! [24]
QMutableLinkedListIterator<double> i(list);
while (i.hasNext()) {
    double val = i.next();
    i.setValue(std::sqrt(val));
}
//! [24]
