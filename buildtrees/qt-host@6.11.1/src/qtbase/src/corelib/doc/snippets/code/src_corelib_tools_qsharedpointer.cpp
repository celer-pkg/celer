// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QSharedPointer>
#include <QWeakPointer>
#include <QObject>
#include <QDebug>

//! [0]
    class Y: public QEnableSharedFromThis<Y>
    {
    public:
        QSharedPointer<Y> f()
        {
            return sharedFromThis();
        }
    };

    int main()
    {
        QSharedPointer<Y> p(new Y());
        QSharedPointer<Y> y = p->f();
        Q_ASSERT(p == y); // p and q must share ownership
    }
//! [0]

//! [1]
    class ScriptInterface : public QObject
    {
        Q_OBJECT

        // ...

    public slots:
        void slotCalledByScript(Y *managedBySharedPointer)
        {
            QSharedPointer<Y> yPtr = managedBySharedPointer->sharedFromThis();
            // Some other code unrelated to scripts that expects a QSharedPointer<Y> ...
        }
    };
//! [1]

class MyObject : public QObject
{
        Q_OBJECT
    public:
        MyObject() { /* ... */ }
        ~MyObject() { /* ... */ }

//! [2]
static void doDeleteLater(MyObject *obj)
{
    obj->deleteLater();
}

void otherFunction()
{
    QSharedPointer<MyObject> obj =
        QSharedPointer<MyObject>(new MyObject, doDeleteLater);

    // continue using obj
    obj.clear();    // calls obj->deleteLater();
}
//! [2]

    template <typename T>
    void someFunc()
    {
        T *t = new T;
        auto deleter = [](T *p) { delete p; };
        {
            //! [6]
            QSharedPointer<T> other(t); this->swap(other);
            //! [6]
        }

        {
            //! [7]
            QSharedPointer<T> other(t, deleter); this->swap(other);
            //! [7]
        }
    }

    template <typename T>
    void swap(QSharedPointer<T> &other) {}
};

void examples()
{
    QSharedPointer<int> sharedptr;
    QWeakPointer<int> weakref;

    {
        //! [2]
        QSharedPointer<int> sharedptr(new int(42));
        //! [2]
    }

    {
        //! [1]
        QSharedPointer<int> sharedptr(new int(42), [](int *p) { delete p; });
        //! [1]
    }

    {
        //! [3]
        QSharedPointer<MyObject> obj =
            QSharedPointer<MyObject>(new MyObject, &QObject::deleteLater);
        //! [3]
    }

    {
        //! [4]
        if (sharedptr) { /*...*/ }
        //! [4]
    }

    {
        //! [5]
        if (!sharedptr) { /*...*/ }
        //! [5]
    }

    {
        //! [8]
        if (weakref) { /*...*/ }
        //! [8]
    }

    {
        //! [9]
        if (!weakref) { /*...*/ }
        //! [9]
    }

    {
        //! [12]
        QWeakPointer<int> weakref;

        // ...

        QSharedPointer<int> strong = weakref.toStrongRef();
        if (strong)
            qDebug() << "The value is:" << *strong;
        else
            qDebug() << "The value has already been deleted";
        //! [12]
    }
}
