// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QIODevice>
#include <QScopedPointer>
#include <QScopedPointerArrayDeleter>
#include <QScopedPointerPodDeleter>

class MyClass {};
class MySubClass : public MyClass {};

QIODevice *handsOverOwnership();
void process(QIODevice *device);
int m_value;

//! [0]
void myFunction(bool useSubClass)
{
    MyClass *p = useSubClass ? new MyClass() : new MySubClass;
    QIODevice *device = handsOverOwnership();

    if (m_value > 3) {
        delete p;
        delete device;
        return;
    }

    try {
        process(device);
    }
    catch (...) {
        delete p;
        delete device;
        throw;
    }

    delete p;
    delete device;
}
//! [0]

namespace repetition
{
    class MyClass {};
    class MySubClass : public MyClass {};

    //! [1]
    void myFunction(bool useSubClass)
    {
        // assuming that MyClass has a virtual destructor
        QScopedPointer<MyClass> p(useSubClass ? new MyClass() : new MySubClass);
        QScopedPointer<QIODevice> device(handsOverOwnership());

        if (m_value > 3)
            return;

        process(device.data());
    }
    //! [1]
}

#if __has_include(<QWidget>)
#include <QWidget>

void QWidget_snippets()
{
    {
        //! [2.0]
            const QWidget *const p = new QWidget();
            // is equivalent to:
            const QScopedPointer<const QWidget> p1(new QWidget());

        //! [2.0]
    }

    {
        //! [2.1]
            QWidget *const p = new QWidget();
            // is equivalent to:
            const QScopedPointer<QWidget> p1(new QWidget());

        //! [2.1]
    }

    {
        //! [2.2]
            const QWidget *p = new QWidget();
            // is equivalent to:
            QScopedPointer<const QWidget> p1(new QWidget());
        //! [2.2]
    }
    bool scopedPointer;

    //! [3]
    if (scopedPointer) {
        //...
    }
    //! [3]
}
#endif

namespace class_repetition
{
    //! [4]
    class MyPrivateClass; // forward declare MyPrivateClass

    class MyClass
    {
    private:
        QScopedPointer<MyPrivateClass> privatePtr; // QScopedPointer to forward declared class

    public:
        MyClass(); // OK
        inline ~MyClass() {} // VIOLATION - Destructor must not be inline

    private:
        Q_DISABLE_COPY(MyClass) // OK - copy constructor and assignment operators
                                // are now disabled, so the compiler won't implicitly
                                // generate them.
    };
    //! [4]

    class MyPrivateClass {};
}

class MyCustomClass {};
void myCustomDeallocator(MyCustomClass *pointer) {}

//! [5]
// this QScopedPointer deletes its data using the delete[] operator:
QScopedPointer<int, QScopedPointerArrayDeleter<int> > arrayPointer(new int[42]);

// this QScopedPointer frees its data using free():
QScopedPointer<int, QScopedPointerPodDeleter> podPointer(reinterpret_cast<int *>(malloc(42)));

// this struct calls "myCustomDeallocator" to delete the pointer
struct ScopedPointerCustomDeleter
{
    static inline void cleanup(MyCustomClass *pointer)
    {
        myCustomDeallocator(pointer);
    }
};

// QScopedPointer using a custom deleter:
QScopedPointer<MyCustomClass, ScopedPointerCustomDeleter> customPointer(new MyCustomClass);
//! [5]
