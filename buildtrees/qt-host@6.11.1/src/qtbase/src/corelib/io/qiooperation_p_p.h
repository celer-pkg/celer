// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QIOOPERATION_P_P_H
#define QIOOPERATION_P_P_H

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

#include "qiooperation_p.h"
#include "qrandomaccessasyncfile_p.h"

#include <QtCore/private/qobject_p.h>

#include <QtCore/qspan.h>
#include <QtCore/qvarlengtharray.h>

#ifdef QT_RANDOMACCESSASYNCFILE_QIORING
#include <QtCore/private/qioring_p.h>
#endif

#include <variant>

QT_BEGIN_NAMESPACE

namespace QtPrivate {

class QIOOperationDataStorage
{
public:
    // When passing QSpan<QSpan<T>>, we'd better have an underlying storage
    // for an outer span, so that users could pass in a temporary object.
    // We'd use QVarLengthArray for that. Having 256 elements (the default)
    // seems to be unneeded for vectored IO. For now I picked 10 as a reasonable
    // default. But maybe even less?
    static constexpr qsizetype DefaultNumOfBuffers = 10;
    using ReadSpans = QVarLengthArray<QSpan<std::byte>, DefaultNumOfBuffers>;
    using WriteSpans = QVarLengthArray<QSpan<const std::byte>, DefaultNumOfBuffers>;

    explicit QIOOperationDataStorage()
        : data(std::monostate{})
    {}
    explicit QIOOperationDataStorage(QSpan<const QSpan<std::byte>> s)
        : data(ReadSpans(s.begin(), s.end()))
    {}
    explicit QIOOperationDataStorage(QSpan<const QSpan<const std::byte>> s)
        : data(WriteSpans(s.begin(), s.end()))
    {}
    explicit QIOOperationDataStorage(const QByteArray &a)
        : data(a)
    {}
    explicit QIOOperationDataStorage(QByteArray &&a)
        : data(std::move(a))
    {}

    bool isEmpty() const
    { return std::holds_alternative<std::monostate>(data); }

    bool containsReadSpans() const
    { return std::holds_alternative<ReadSpans>(data); }

    bool containsWriteSpans() const
    { return std::holds_alternative<WriteSpans>(data); }

    bool containsByteArray() const
    { return std::holds_alternative<QByteArray>(data); }

    ReadSpans &getReadSpans()
    {
        Q_ASSERT(containsReadSpans());
        return *std::get_if<ReadSpans>(&data);
    }
    const ReadSpans &getReadSpans() const
    {
        Q_ASSERT(containsReadSpans());
        return *std::get_if<ReadSpans>(&data);
    }

    WriteSpans &getWriteSpans()
    {
        Q_ASSERT(containsWriteSpans());
        return *std::get_if<WriteSpans>(&data);
    }
    const WriteSpans &getWriteSpans() const
    {
        Q_ASSERT(containsWriteSpans());
        return *std::get_if<WriteSpans>(&data);
    }

    QByteArray &getByteArray()
    {
        Q_ASSERT(containsByteArray());
        return *std::get_if<QByteArray>(&data);
    }
    const QByteArray &getByteArray() const
    {
        Q_ASSERT(containsByteArray());
        return *std::get_if<QByteArray>(&data);
    }

    // Potentially can be extended to return a QVariant::value<T>().
    template <typename T>
    T getValue() const = delete;

private:
    std::variant<std::monostate, ReadSpans, WriteSpans, QByteArray> data;
};

template <>
inline QSpan<const QSpan<std::byte>> QIOOperationDataStorage::getValue() const
{
    Q_ASSERT(std::holds_alternative<ReadSpans>(data));
    const auto *val = std::get_if<ReadSpans>(&data);
    if (val)
        return QSpan(*val);
    return {};
}

template <>
inline QSpan<const QSpan<const std::byte>> QIOOperationDataStorage::getValue() const
{
    Q_ASSERT(std::holds_alternative<WriteSpans>(data));
    const auto *val = std::get_if<WriteSpans>(&data);
    if (val)
        return QSpan(*val);
    return {};
}

template <>
inline QByteArray QIOOperationDataStorage::getValue() const
{
    Q_ASSERT(std::holds_alternative<QByteArray>(data));
    const auto *val = std::get_if<QByteArray>(&data);
    if (val)
        return *val;
    return {};
}

} // namespace QtPrivate

class QIOOperationPrivate : public QObjectPrivate
{
public:
    Q_DECLARE_PUBLIC(QIOOperation)

    enum class State : quint8
    {
        Running,
        Finished,
    };

    explicit QIOOperationPrivate(QtPrivate::QIOOperationDataStorage *storage);
    ~QIOOperationPrivate();

    static QIOOperationPrivate *get(QIOOperation *op)
    { return op->d_func(); }

    void appendBytesProcessed(qint64 num);
    void operationComplete(QIOOperation::Error err);
    void setError(QIOOperation::Error err);

    QPointer<QRandomAccessAsyncFile> file;

    qint64 offset = 0;
    qint64 processed = 0;

    QIOOperation::Error error = QIOOperation::Error::None;
    QIOOperation::Type type = QIOOperation::Type::Unknown;

    State state = State::Running;

    // takes ownership
    std::unique_ptr<QtPrivate::QIOOperationDataStorage> dataStorage;
};

QT_END_NAMESPACE

#endif // QIOOPERATION_P_P_H
