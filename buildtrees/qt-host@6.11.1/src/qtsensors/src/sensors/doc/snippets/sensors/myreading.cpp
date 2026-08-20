// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QSensorReading>

//! [MyReading-Declaration]
class MyReadingPrivate;

class MyReading : public QSensorReading
{
    Q_OBJECT
    Q_PROPERTY(qreal myprop READ myprop)
    DECLARE_READING(MyReading)
public:
    qreal myprop() const;
    void setMyprop(qreal myprop);
};
//! [MyReading-Declaration]

qreal MyReading::myprop() const { return 0.0; }
void setMyprop(qreal) { }

class MyReadingPrivate
{
};

//! [IMPLEMENT_READING_MyReading]
IMPLEMENT_READING(MyReading)
//! [IMPLEMENT_READING_MyReading]

#include "myreading.moc"
