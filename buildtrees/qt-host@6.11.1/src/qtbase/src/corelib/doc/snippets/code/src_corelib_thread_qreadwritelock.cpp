// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QReadWriteLock>
#include <QByteArray>

class ReaderThread
{
public:
    void run();
};

class WriterThread
{
public:
    void run();
};

void read_file();
void write_file();

//! [lock]
QReadWriteLock lock;

//! [lock]

//! [0]
void ReaderThread::run()
{
    //...
    lock.lockForRead();
    read_file();
    lock.unlock();
    //...
}

void WriterThread::run()
{
    //...
    lock.lockForWrite();
    write_file();
    lock.unlock();
    //...
}
//! [0]

QByteArray data;

//! [1]
QByteArray readData()
{
    QReadLocker locker(&lock);
    //...
    return data;
}
//! [1]

namespace duplicate_examples
{
    QByteArray data;
    QReadWriteLock lock;

    //! [2]
    QByteArray readData()
    {
        lock.lockForRead();
        //...
        lock.unlock();
        return data;
    }
    //! [2]


    //! [3]
    void writeData(const QByteArray &data)
    {
        QWriteLocker locker(&lock);
        //...
    }
    //! [3]
}

//! [4]
void writeData(const QByteArray &data)
{
    lock.lockForWrite();
    //...
    lock.unlock();
}
//! [4]
