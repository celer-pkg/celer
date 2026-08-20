// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QAtomicInt>

struct Data
{
    QAtomicInt atomicInt;
};

struct MySharedType
{
    Data *data;
    Data *d;
    MySharedType &operator=(const MySharedType &other);
};

//! [0]
MySharedType &MySharedType::operator=(const MySharedType &other)
{
    (void) other.data->atomicInt.ref();
    if (!data->atomicInt.deref()) {
        // The last reference has been released
        delete d;
    }
    d = other.d;
    return *this;
}
//! [0]

bool examples(int currentValue, int expectedValue, int newValue)
{
    {
        //! [1]
        if (currentValue == expectedValue) {
            currentValue = newValue;
            return true;
        }
        return false;
        //! [1]
    }

    {
        //! [4]
        if (currentValue == expectedValue) {
            currentValue = newValue;
            return true;
        }
        return false;
        //! [4]
    }
}

int wrapInFunction(int currentValue, int newValue, int valueToAdd)
{
    {
        //! [2]
        int originalValue = currentValue;
        currentValue = newValue;
        return originalValue;
        //! [2]
    }

    {
        //! [3]
        int originalValue = currentValue;
        currentValue += valueToAdd;
        return originalValue;
        //! [3]
    }
}

template <typename T>

T *wrapInTFunction(T *currentValue, T *newValue, T valueToAdd)
{
    {
        //! [5]
        T *originalValue = currentValue;
        currentValue = newValue;
        return originalValue;
        //! [5]
    }

    {
        //! [6]
        T *originalValue = currentValue;
        currentValue += valueToAdd;
        return originalValue;
        //! [6]
    }
}
