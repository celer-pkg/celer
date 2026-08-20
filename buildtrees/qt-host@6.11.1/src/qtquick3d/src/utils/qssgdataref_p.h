// Copyright (C) 2008-2012 NVIDIA Corporation.
// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QSSGDATAREF_H
#define QSSGDATAREF_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtQuick3DUtils/private/qtquick3dutilsglobal_p.h>

#include <QtCore/qlist.h>
#include <QtCore/qbytearray.h>

QT_BEGIN_NAMESPACE

template<typename T>
struct QSSGDataView
{
    const T *mData;
    qsizetype mSize;

    explicit QSSGDataView(const QList<T> &data) : mData(data.constData()), mSize(data.size()) { Q_ASSERT(mSize >= 0); }
    template <qsizetype N>
    explicit QSSGDataView(const QVarLengthArray<T, N> &data) : mData(data.constData()), mSize(data.size()) { Q_ASSERT(mSize >= 0); }
    QSSGDataView(const T *inData, qsizetype inSize) : mData(inData), mSize(inSize) { Q_ASSERT(mSize >= 0); }
    constexpr QSSGDataView() : mData(nullptr), mSize(0) {}

    qsizetype size() const { return mSize; }

    const T *begin() const { return mData; }
    const T *end() const { return mData + mSize; }

    const T& first() const { Q_ASSERT(!isEmpty()); return *begin(); }
    const T& last() const { Q_ASSERT(!isEmpty()); return *(end()-1); }

    const T &operator[](int index) const
    {
        Q_ASSERT(index > -1);
        Q_ASSERT(index < mSize);
        return mData[index];
    }

    bool isEmpty() const { return (mSize == 0); }

    void clear()
    {
        mData = nullptr;
        mSize = 0;
    }

    operator const void *() { return reinterpret_cast<const void *>(mData); }
};

template<>
struct QSSGDataView<quint8>
{
    const quint8 *mData;
    qsizetype mSize;

    explicit QSSGDataView(const QByteArray &data)
        : mData(reinterpret_cast<const quint8 *>(data.constBegin())), mSize(data.size())
    { Q_ASSERT(mSize >= 0); }
    template<typename T>
    explicit QSSGDataView(const QList<T> &data)
        : mData(reinterpret_cast<const quint8 *>(data.constData())), mSize(data.size()*sizeof(T))
    { Q_ASSERT(mSize >= 0); }
    QSSGDataView(const quint8 *inData, qsizetype inSize) : mData(inData), mSize(inSize) { Q_ASSERT(mSize >= 0); }
    template<typename T>
    QSSGDataView(const T *inData, qsizetype inSize)
        : mData(reinterpret_cast<const quint8 *>(inData)), mSize(inSize*sizeof(T))
    { Q_ASSERT(mSize >= 0); }
    constexpr QSSGDataView() : mData(nullptr), mSize(0) {}

    qsizetype size() const { return mSize; }
    bool isEmpty() const { return (mSize == 0); }

    const quint8 *begin() const { return mData; }
    const quint8 *end() const { return mData + mSize; }

    const quint8 &operator[](int index) const
    {
        Q_ASSERT(index > -1);
        Q_ASSERT(index < mSize);
        return mData[index];
    }

    void clear()
    {
        mData = nullptr;
        mSize = 0;
    }

    operator const void *() { return reinterpret_cast<const void *>(mData); }
};

using QSSGByteView = QSSGDataView<quint8>;

template<typename T>
inline QSSGDataView<T> toDataView(const T &type)
{
    return QSSGDataView<T>(&type, 1);
}

template<typename T>
inline QSSGDataView<T> toDataView(const QList<T> &type)
{
    return QSSGDataView<T>(type);
}

template<typename T>
inline QSSGByteView toByteView(const T &type)
{
    return QSSGByteView(&type, 1);
}

template<typename T>
inline QSSGByteView toByteView(const QList<T> &type)
{
    return QSSGByteView(type);
}

template<>
inline QSSGByteView toByteView(const QByteArray &type)
{
    return QSSGByteView(type);
}

inline QSSGByteView toByteView(const char *str)
{
    return QSSGByteView(str, qstrlen(str));
}

template<typename T>
inline QSSGDataView<T> toDataView(const T *type, qsizetype count)
{
    return QSSGDataView<T>(type, count);
}

template<typename T>
inline QSSGByteView toByteView(const T *type, qsizetype count)
{
    return QSSGByteView(type, count);
}

template<typename T>
struct QSSGDataRef
{
    T *mData;
    qsizetype mSize;

    QSSGDataRef(T *inData, qsizetype inSize) : mData(inData), mSize(inSize) { Q_ASSERT(inSize >= 0); }
    QSSGDataRef() : mData(nullptr), mSize(0) {}
    qsizetype size() const { return mSize; }

    T *begin() { return mData; }
    T *end() { return mData + mSize; }

    T *begin() const { return mData; }
    T *end() const { return mData + mSize; }

    T& first() { Q_ASSERT(!isEmpty()); return *begin(); }
    T& last() { Q_ASSERT(!isEmpty()); return *(end()-1); }

    const T &first() const { Q_ASSERT(!isEmpty()); return *begin(); }
    const T &last() const { Q_ASSERT(!isEmpty()); return *(end()-1); }

    bool isEmpty() const { return (mSize == 0); }

    T &operator[](qsizetype index)
    {
        Q_ASSERT(index >= 0);
        Q_ASSERT(index < mSize);
        return mData[index];
    }

    const T &operator[](qsizetype index) const
    {
        Q_ASSERT(index >= 0);
        Q_ASSERT(index < mSize);
        return mData[index];
    }

    void clear()
    {
        mData = nullptr;
        mSize = 0;
    }

    operator QSSGDataView<T>() const { return QSSGDataView<T>(mData, mSize); }
    operator void *() { return reinterpret_cast<void *>(mData); }
};

using QSSGByteRef = QSSGDataRef<quint8>;

template<typename T>
inline QSSGDataRef<T> toDataRef(T &type)
{
    return QSSGDataRef<T>(&type, 1);
}

template<typename T>
inline QSSGByteRef toByteRef(T &type)
{
    return QSSGByteRef(reinterpret_cast<quint8 *>(&type), sizeof(T));
}

template<typename T>
inline QSSGDataRef<T> toDataRef(T *type, qsizetype count)
{
    return QSSGDataRef<T>(type, count);
}

template<typename T>
inline QSSGByteRef toByteRef(T *type, qsizetype count)
{
    return QSSGByteRef(reinterpret_cast<quint8 *>(type), sizeof(T) * count);
}

QT_END_NAMESPACE

#endif // QSSGDATAREF_H
