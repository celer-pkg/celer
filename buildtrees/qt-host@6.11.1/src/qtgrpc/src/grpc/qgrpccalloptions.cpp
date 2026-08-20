// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtGrpc/private/qgrpccommonoptions_p.h>
#include <QtGrpc/private/qtgrpclogging_p.h>
#include <QtGrpc/qgrpccalloptions.h>

#include <QtCore/qbytearray.h>
#include <QtCore/qdebug.h>
#include <QtCore/qvariant.h>

QT_BEGIN_NAMESPACE

using namespace Qt::StringLiterals;

/*!
    \class QGrpcCallOptions
    \inmodule QtGrpc
    \compares equality
    \brief The QGrpcCallOptions class offers various options for fine-tuning
    individual RPCs.
    \since 6.6

    QGrpcCallOptions lets you customize individual remote procedure calls (RPCs).
    The generated client interface provides access points to pass the QGrpcCallOptions.
    These options supersede the ones set via QGrpcChannelOptions.

    To configure the default options shared by RPCs, use QGrpcChannelOptions.

    \code
        QGrpcCallOptions callOpts;
        // Set the metadata for an individial RPC
        callOpts.setMetadata({
            { "header" , "value1" },
            { "header" , "value2" },
        });
        const auto &md = callOpts.metadata(QtGrpc::MultiValue);
        qDebug() << "Call Metadata: " << md;

        // Set a 2-second deadline for an individial RPC
        callOpts.setDeadlineTimeout(2s);
        qDebug() << "Call timeout: " << callOpts.deadlineTimeout();
    \endcode
*/

class QGrpcCallOptionsPrivate : public QGrpcCommonOptions
{
public:
    QGrpcCallOptionsPrivate() = default;
};

QT_DEFINE_QESDP_SPECIALIZATION_DTOR(QGrpcCallOptionsPrivate)

/*!
    Default-constructs an empty QGrpcCallOptions.
*/
QGrpcCallOptions::QGrpcCallOptions() : d_ptr(new QGrpcCallOptionsPrivate())
{
}

/*!
    Destroys the QGrpcCallOptions.
*/
QGrpcCallOptions::~QGrpcCallOptions() = default;

/*!
    Copy-constructs a QGrpcCallOptions from \a other.
*/
QGrpcCallOptions::QGrpcCallOptions(const QGrpcCallOptions &other) = default;

/*!
    Assigns \a other to this QGrpcCallOptions and returns a reference to the
    updated object.
*/
QGrpcCallOptions &QGrpcCallOptions::operator=(const QGrpcCallOptions &other) = default;

/*!
    \fn QGrpcCallOptions::QGrpcCallOptions(QGrpcCallOptions &&other)

    Move-constructs a new QGrpcCallOptions from \a other.

    \include qtgrpc-shared.qdocinc move-note-desc
*/

/*!
    \fn QGrpcCallOptions &QGrpcCallOptions::operator=(QGrpcCallOptions &&other)

    Move-assigns \a other to this QGrpcCallOptions and returns a reference to
    the updated object.

    \include qtgrpc-shared.qdocinc move-note-desc
*/

/*!
    \since 6.8

    \include qtgrpc-shared.qdocinc qvariant-desc
*/
QGrpcCallOptions::operator QVariant() const
{
    return QVariant::fromValue(*this);
}

/*!
    \since 6.8
    \fn void QGrpcCallOptions::swap(QGrpcCallOptions &other)

    \include qtgrpc-shared.qdocinc swap-desc
*/

/*!
    \include qgrpccommonoptions.cpp set-deadline-timeout

    \note Setting this field \b{overrides} the corresponding channel options field
    — see \l{QGrpcChannelOptions::setDeadlineTimeout()}

    \sa deadlineTimeout()
*/
QGrpcCallOptions &QGrpcCallOptions::setDeadlineTimeout(std::chrono::milliseconds timeout)
{
    if (d_ptr->deadlineTimeout() == timeout)
        return *this;
    d_ptr.detach();
    Q_D(QGrpcCallOptions);
    d->setDeadlineTimeout(timeout);
    return *this;
}

/*!
    \include qgrpccommonoptions.cpp deadline-timeout
*/
std::optional<std::chrono::milliseconds> QGrpcCallOptions::deadlineTimeout() const noexcept
{
    Q_D(const QGrpcCallOptions);
    return d->deadlineTimeout();
}

#if QT_DEPRECATED_SINCE(6, 13)

/*!
    \fn const QHash<QByteArray, QByteArray> &QGrpcCallOptions::metadata() const &
    \fn QHash<QByteArray, QByteArray> QGrpcCallOptions::metadata() &&
    \deprecated [6.13] Use the QMultiHash overload instead.

    \include qgrpccommonoptions.cpp metadata

    \sa setMetadata()
*/
const QHash<QByteArray, QByteArray> &QGrpcCallOptions::metadata() const & noexcept
{
    Q_D(const QGrpcCallOptions);
    return d->metadata();
}
QHash<QByteArray, QByteArray> QGrpcCallOptions::metadata() &&
{
    Q_D(QGrpcCallOptions);
    if (d->ref.loadRelaxed() != 1)
        return d->metadata();
    return std::move(*d_ptr).metadata();
}

/*!
    \fn QGrpcCallOptions &QGrpcCallOptions::setMetadata(const QHash<QByteArray, QByteArray> &metadata)
    \fn QGrpcCallOptions &QGrpcCallOptions::setMetadata(QHash<QByteArray, QByteArray> &&metadata)
    \deprecated [6.13] Use the QMultiHash overload instead.

    \include qgrpccommonoptions.cpp set-metadata

//! [merge-md-note]
    \note Call metadata is \b{merged} with any channel-level metadata when the
    RPC starts — see
//! [merge-md-note]
    \l{QGrpcChannelOptions::setMetadata(const QMultiHash<QByteArray,
    QByteArray>&)}{QGrpcChannelOptions::setMetadata(QMultiHash)}.

    \sa metadata()
*/
QGrpcCallOptions &QGrpcCallOptions::setMetadata(const QHash<QByteArray, QByteArray> &metadata)
{
    if (d_ptr->metadata() == metadata)
        return *this;
    d_ptr.detach();
    Q_D(QGrpcCallOptions);
    d->setMetadata(metadata);
    return *this;
}
QGrpcCallOptions &QGrpcCallOptions::setMetadata(QHash<QByteArray, QByteArray> &&metadata)
{
    if (d_ptr->metadata() == metadata)
        return *this;
    d_ptr.detach();
    Q_D(QGrpcCallOptions);
    d->setMetadata(std::move(metadata));
    return *this;
}

#endif // QT_DEPRECATED_SINCE(6, 13)

/*!
    \since 6.10
    \fn const QMultiHash<QByteArray, QByteArray> &QGrpcCallOptions::metadata(QtGrpc::MultiValue_t) const &
    \fn QMultiHash<QByteArray, QByteArray> QGrpcCallOptions::metadata(QtGrpc::MultiValue_t) &&

    \include qgrpccommonoptions.cpp metadata-multi

    \sa {setMetadata(const QMultiHash<QByteArray, QByteArray>&)}{setMetadata}
*/
const QMultiHash<QByteArray, QByteArray> &
QGrpcCallOptions::metadata(QtGrpc::MultiValue_t tag) const & noexcept
{
    Q_D(const QGrpcCallOptions);
    return d->metadata(tag);
}
QMultiHash<QByteArray, QByteArray> QGrpcCallOptions::metadata(QtGrpc::MultiValue_t tag) &&
{
    Q_D(QGrpcCallOptions);
    if (d->ref.loadRelaxed() != 1)
        return d->metadata(tag);
    return std::move(*d_ptr).metadata(tag);
}

/*!
    \since 6.10
    \fn QGrpcCallOptions &QGrpcCallOptions::setMetadata(const QMultiHash<QByteArray, QByteArray> &metadata)
    \fn QGrpcCallOptions &QGrpcCallOptions::setMetadata(QMultiHash<QByteArray, QByteArray> &&metadata)
    \fn QGrpcCallOptions &QGrpcCallOptions::setMetadata(std::initializer_list<std::pair<QByteArray, QByteArray>> metadata)

    \include qgrpccommonoptions.cpp set-metadata-multi

    \include qgrpccalloptions.cpp merge-md-note
    \l{QGrpcChannelOptions::setMetadata(const QMultiHash<QByteArray,
    QByteArray>&)}{QGrpcChannelOptions::setMetadata(QMultiHash)}.

    \sa metadata(QtGrpc::MultiValue_t)
*/
QGrpcCallOptions &QGrpcCallOptions::setMetadata(const QMultiHash<QByteArray, QByteArray> &metadata)
{
    if (d_ptr->metadata(QtGrpc::MultiValue) == metadata)
        return *this;
    d_ptr.detach();
    Q_D(QGrpcCallOptions);
    d->setMetadata(metadata);
    return *this;
}
QGrpcCallOptions &QGrpcCallOptions::setMetadata(QMultiHash<QByteArray, QByteArray> &&metadata)
{
    if (d_ptr->metadata(QtGrpc::MultiValue) == metadata)
        return *this;
    d_ptr.detach();
    Q_D(QGrpcCallOptions);
    d->setMetadata(std::move(metadata));
    return *this;
}
QGrpcCallOptions &
QGrpcCallOptions::setMetadata(std::initializer_list<std::pair<QByteArray, QByteArray>> list)
{
    return setMetadata(QMultiHash<QByteArray, QByteArray>(list));
}

/*!
    \include qgrpccommonoptions.cpp add-metadata

    \include qgrpccalloptions.cpp merge-md-note
    \l{QGrpcChannelOptions::addMetadata()}
*/
QGrpcCallOptions &QGrpcCallOptions::addMetadata(QByteArrayView key, QByteArrayView value)
{
    if (d_ptr->containsMetadata(key, value))
        return *this;
    d_ptr.detach();
    Q_D(QGrpcCallOptions);
    d->addMetadata(key.toByteArray(), value.toByteArray());
    return *this;
}

/*!
    \include qgrpccommonoptions.cpp filterServerMetadata
    \sa QGrpcChannelOptions::filterServerMetadata()
*/
std::optional<bool> QGrpcCallOptions::filterServerMetadata() const noexcept
{
    Q_D(const QGrpcCallOptions);
    return d->filterServerMetadata();
}

/*!
    \include qgrpccommonoptions.cpp setFilterServerMetadata
    \note Setting this field \b{overrides} the corresponding channel options
    field — see \l{QGrpcChannelOptions::setFilterServerMetadata()}

    \sa QGrpcChannelOptions::setFilterServerMetadata()
*/
QGrpcCallOptions &QGrpcCallOptions::setFilterServerMetadata(bool value)
{
    if (d_ptr->filterServerMetadata() == value)
        return *this;
    d_ptr.detach();
    Q_D(QGrpcCallOptions);
    d->setFilterServerMetadata(value);
    return *this;
}

/*
//! [compares]
    Returns \c true if the \l{deadlineTimeout}, \l{filterServerMetadata}
    and \l{metadata(QtGrpc::MultiValue_t)} in \a lhs and \a rhs are
//! [compares]
*/
bool comparesEqual(const QGrpcCallOptions &lhs, const QGrpcCallOptions &rhs)
{
    return lhs.deadlineTimeout() == rhs.deadlineTimeout()
        && lhs.filterServerMetadata() == rhs.filterServerMetadata()
        && lhs.metadata(QtGrpc::MultiValue) == rhs.metadata(QtGrpc::MultiValue);
}

/*!
    \since 6.11
    \fn bool QGrpcCallOptions::operator==(const QGrpcCallOptions &lhs, const QGrpcCallOptions &rhs)
    \include qgrpccalloptions.cpp compares
    equal.
*/

/*!
    \since 6.11
    \fn bool QGrpcCallOptions::operator!=(const QGrpcCallOptions &lhs, const QGrpcCallOptions &rhs)
    \include qgrpccalloptions.cpp compares
    not equal.
*/

#ifndef QT_NO_DEBUG_STREAM
/*!
    \since 6.8
    \fn QDebug QGrpcCallOptions::operator<<(QDebug debug, const QGrpcCallOptions &callOpts)

    Writes \a callOpts to the specified stream \a debug.
*/
QDebug operator<<(QDebug debug, const QGrpcCallOptions &callOpts)
{
    const QDebugStateSaver save(debug);
    debug.nospace().noquote();
    debug << "QGrpcCallOptions(deadline: " << callOpts.deadlineTimeout()
          << ", metadata: " << callOpts.metadata(QtGrpc::MultiValue) << ", filterServerMetadata: "
          << callOpts.filterServerMetadata() << ')';
    return debug;
}
#endif

QT_END_NAMESPACE
