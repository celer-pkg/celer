// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtGrpc/private/qgrpccommonoptions_p.h>
#include <QtGrpc/qgrpcchanneloptions.h>
#include <QtGrpc/qgrpcserializationformat.h>
#include <QtGrpc/qtgrpcnamespace.h>

#include <QtCore/qbytearray.h>
#include <QtCore/qdebug.h>
#include <QtCore/qvariant.h>

QT_BEGIN_NAMESPACE

using namespace Qt::StringLiterals;
using namespace QtGrpc;

/*!
    \class QGrpcChannelOptions
    \inmodule QtGrpc
    \since 6.6
    \compares equality
    \brief The QGrpcChannelOptions class offers various options for fine-tuning
    a gRPC channel.

    QGrpcChannelOptions lets you customize a \gRPC channel. Some options apply
    to all remote procedure calls (RPCs) that operate on the associated
    channel, which is used to communicate with services.

    Override options for specific RPCs with QGrpcCallOptions.

    \code
        QGrpcChannelOptions channelOpts;
        // Apply common metadata to every RPC
        channelOpts.setMetadata({
            { "header" , "value1" },
            { "header" , "value2" },
        });
        const auto &md = channelOpts.metadata(QtGrpc::MultiValue);
        qDebug() << "Channel Metadata: " << md;

        // Apply a 2-second deadline to every RPC
        channelOpts.setDeadlineTimeout(2s);
        qDebug() << "Channel timeout: " << channelOpts.deadlineTimeout();

        // Configure SSL/TLS configuration
        channelOpts.setSslConfiguration(QSslConfiguration());
    \endcode


    \note It is up to the channel's implementation to determine the specifics
    of these options.
*/

class QGrpcChannelOptionsPrivate : public QGrpcCommonOptions
{
public:
    QGrpcSerializationFormat serializationFormat;
#if QT_CONFIG(ssl)
    std::optional<QSslConfiguration> sslConfiguration;
#endif
};

QT_DEFINE_QESDP_SPECIALIZATION_DTOR(QGrpcChannelOptionsPrivate)

/*!
    Default-constructs an empty QGrpcChannelOptions.
*/
QGrpcChannelOptions::QGrpcChannelOptions() : d_ptr(new QGrpcChannelOptionsPrivate())
{
}

/*!
    Copy-constructs a QGrpcChannelOptions from \a other.
*/
QGrpcChannelOptions::QGrpcChannelOptions(const QGrpcChannelOptions &other) = default;

/*!
    Assigns \a other to this QGrpcChannelOptions and returns a reference to the
    updated object.
*/
QGrpcChannelOptions &QGrpcChannelOptions::operator=(const QGrpcChannelOptions &other) = default;

/*!
    \fn QGrpcChannelOptions::QGrpcChannelOptions(QGrpcChannelOptions &&other)

    Move-constructs a new QGrpcChannelOptions from \a other.

    \include qtgrpc-shared.qdocinc move-note-desc
*/

/*!
    \fn QGrpcChannelOptions &QGrpcChannelOptions::operator=(QGrpcChannelOptions &&other)

    Move-assigns \a other to this QGrpcChannelOptions and returns a reference
    to the updated object.

    \include qtgrpc-shared.qdocinc move-note-desc
*/

/*!
    \since 6.8
    \fn void QGrpcChannelOptions::swap(QGrpcChannelOptions &other)

    \include qtgrpc-shared.qdocinc swap-desc
*/

/*!
    Destroys the QGrpcChannelOptions.
*/
QGrpcChannelOptions::~QGrpcChannelOptions() = default;

/*!
    \since 6.8
    \include qtgrpc-shared.qdocinc qvariant-desc
*/
QGrpcChannelOptions::operator QVariant() const
{
    return QVariant::fromValue(*this);
}

/*!
    \include qgrpccommonoptions.cpp set-deadline-timeout

//! [channel-note]
    \note Setting this field applies to all RPCs that operate on the channel,
    except those overriden by
//! [channel-note]
    \l{QGrpcCallOptions::setDeadlineTimeout()}

    \sa deadlineTimeout()
*/
QGrpcChannelOptions &QGrpcChannelOptions::setDeadlineTimeout(std::chrono::milliseconds timeout)
{
    if (d_ptr->deadlineTimeout() == timeout)
        return *this;
    d_ptr.detach();
    Q_D(QGrpcChannelOptions);
    d->setDeadlineTimeout(timeout);
    return *this;
}

#if QT_DEPRECATED_SINCE(6, 13)

/*!
    \fn const QHash<QByteArray, QByteArray> &QGrpcChannelOptions::metadata() const &
    \fn QHash<QByteArray, QByteArray> QGrpcChannelOptions::metadata() &&
    \deprecated [6.13] Use \l{metadata(QtGrpc::MultiValue_t)}{metadata(QtGrpc::MultiValue)} instead.

    \include qgrpccommonoptions.cpp metadata

    \sa metadata(QtGrpc::MultiValue_t), setMetadata()
*/
const QHash<QByteArray, QByteArray> &QGrpcChannelOptions::metadata() const & noexcept
{
    Q_D(const QGrpcChannelOptions);
    return d->metadata();
}
QHash<QByteArray, QByteArray> QGrpcChannelOptions::metadata() &&
{
    Q_D(QGrpcChannelOptions);
    if (d->ref.loadRelaxed() != 1) // return copy if shared
        return d->metadata();
    return std::move(*d_ptr).metadata();
}

/*!
    \fn QGrpcChannelOptions &QGrpcChannelOptions::setMetadata(const QHash<QByteArray, QByteArray> &metadata)
    \fn QGrpcChannelOptions &QGrpcChannelOptions::setMetadata(QHash<QByteArray, QByteArray> &&metadata)
    \deprecated [6.13] Use the QMultiHash overload instead.

    \include qgrpccommonoptions.cpp set-metadata

//! [merge-md-note]
    \note This metadata is included in every RPC made through the channel.
    Channel metadata is \b{merged} with any call-specific metadata when the RPC
    starts — see
//! [merge-md-note]
    \l{QGrpcCallOptions::setMetadata(const QMultiHash<QByteArray,
    QByteArray>&)}{QGrpcCallOptions::setMetadata(QMultiHash)}

    \sa metadata()
*/
QGrpcChannelOptions &QGrpcChannelOptions::setMetadata(const QHash<QByteArray, QByteArray> &metadata)
{
    if (d_ptr->metadata() == metadata)
        return *this;
    d_ptr.detach();
    Q_D(QGrpcChannelOptions);
    d->setMetadata(metadata);
    return *this;
}
QGrpcChannelOptions &QGrpcChannelOptions::setMetadata(QHash<QByteArray, QByteArray> &&metadata)
{
    if (d_ptr->metadata() == metadata)
        return *this;
    d_ptr.detach();
    Q_D(QGrpcChannelOptions);
    d->setMetadata(std::move(metadata));
    return *this;
}

#endif // QT_DEPRECATED_SINCE(6, 13)

/*!
    \since 6.10
    \fn const QMultiHash<QByteArray, QByteArray> &QGrpcChannelOptions::metadata(QtGrpc::MultiValue_t) const &
    \fn QMultiHash<QByteArray, QByteArray> QGrpcChannelOptions::metadata(QtGrpc::MultiValue_t) &&

    \include qgrpccommonoptions.cpp metadata-multi

    \sa {setMetadata(const QMultiHash<QByteArray, QByteArray>&)}{setMetadata}
*/
const QMultiHash<QByteArray, QByteArray> &
QGrpcChannelOptions::metadata(QtGrpc::MultiValue_t tag) const & noexcept
{
    Q_D(const QGrpcChannelOptions);
    return d->metadata(tag);
}

QMultiHash<QByteArray, QByteArray>
QGrpcChannelOptions::metadata(QtGrpc::MultiValue_t tag) &&
{
    Q_D(QGrpcChannelOptions);
    if (d->ref.loadRelaxed() != 1)
        return d->metadata(tag);
    return std::move(*d_ptr).metadata(tag);
}

/*!
    \since 6.10
    \fn QGrpcChannelOptions &QGrpcChannelOptions::setMetadata(const QMultiHash<QByteArray, QByteArray> &metadata)
    \fn QGrpcChannelOptions &QGrpcChannelOptions::setMetadata(QMultiHash<QByteArray, QByteArray> &&metadata)
    \fn QGrpcChannelOptions &QGrpcChannelOptions::setMetadata(std::initializer_list<std::pair<QByteArray, QByteArray>> list)

    \include qgrpccommonoptions.cpp set-metadata-multi

    \include qgrpcchanneloptions.cpp merge-md-note
    \l{QGrpcCallOptions::setMetadata(const QMultiHash<QByteArray,
    QByteArray>&)}{QGrpcCallOptions::setMetadata(QMultiHash)}

    \sa metadata(QtGrpc::MultiValue_t)
*/
QGrpcChannelOptions &
QGrpcChannelOptions::setMetadata(const QMultiHash<QByteArray, QByteArray> &metadata)
{
    if (d_ptr->metadata(QtGrpc::MultiValue) == metadata)
        return *this;
    d_ptr.detach();
    Q_D(QGrpcChannelOptions);
    d->setMetadata(metadata);
    return *this;
}
QGrpcChannelOptions &QGrpcChannelOptions::setMetadata(QMultiHash<QByteArray, QByteArray> &&metadata)
{
    if (d_ptr->metadata(QtGrpc::MultiValue) == metadata)
        return *this;
    d_ptr.detach();
    Q_D(QGrpcChannelOptions);
    d->setMetadata(std::move(metadata));
    return *this;
}
QGrpcChannelOptions &
QGrpcChannelOptions::setMetadata(std::initializer_list<std::pair<QByteArray, QByteArray>> list)
{
    return setMetadata(QMultiHash<QByteArray, QByteArray>(list));
}

/*!
    \include qgrpccommonoptions.cpp add-metadata

    \include qgrpcchanneloptions.cpp merge-md-note
    \l{QGrpcCallOptions::addMetadata()}
*/
QGrpcChannelOptions &QGrpcChannelOptions::addMetadata(QByteArrayView key, QByteArrayView value)
{
    if (d_ptr->containsMetadata(key, value))
        return *this;
    d_ptr.detach();
    Q_D(QGrpcChannelOptions);
    d->addMetadata(key.toByteArray(), value.toByteArray());
    return *this;
}

/*!
    \include qgrpccommonoptions.cpp filterServerMetadata
    \sa QGrpcCallOptions::filterServerMetadata()
*/
std::optional<bool> QGrpcChannelOptions::filterServerMetadata() const noexcept
{
    Q_D(const QGrpcChannelOptions);
    return d->filterServerMetadata();
}

/*!
    \include qgrpccommonoptions.cpp setFilterServerMetadata

    \include qgrpcchanneloptions.cpp channel-note
    \l{QGrpcCallOptions::filterServerMetadata}

    \sa QGrpcCallOptions::setFilterServerMetadata()
*/
QGrpcChannelOptions &QGrpcChannelOptions::setFilterServerMetadata(bool value)
{
    if (filterServerMetadata() == value)
        return *this;
    d_ptr.detach();
    Q_D(QGrpcChannelOptions);
    d->setFilterServerMetadata(value);
    return *this;
}

/*!
    \since 6.8

    Sets the serialization \a format for the channel and returns a reference to
    the updated object.
*/
QGrpcChannelOptions &
QGrpcChannelOptions::setSerializationFormat(const QGrpcSerializationFormat &format)
{
    if (d_ptr->serializationFormat == format)
        return *this;
    d_ptr.detach();
    Q_D(QGrpcChannelOptions);
    d->serializationFormat = format;
    return *this;
}

/*!
    Returns the timeout duration that is used to calculate the deadline for the
    channel.

    If this field is unset, returns an empty \c {std::optional}.
*/
std::optional<std::chrono::milliseconds> QGrpcChannelOptions::deadlineTimeout() const noexcept
{
    Q_D(const QGrpcChannelOptions);
    return d->deadlineTimeout();
}

/*!
    \since 6.8

    Returns the serialization format used by the channel.

    If this field is unset, returns a \l {QtGrpc::SerializationFormat::}
    {Default} constructed serialization format.
 */
QGrpcSerializationFormat QGrpcChannelOptions::serializationFormat() const
{
    Q_D(const QGrpcChannelOptions);
    return d->serializationFormat;
}

#if QT_CONFIG(ssl)
/*!
    Sets the \a sslConfiguration for the channel and returns a reference to the
    updated object.
*/
QGrpcChannelOptions &
QGrpcChannelOptions::setSslConfiguration(const QSslConfiguration &sslConfiguration)
{
    if (d_ptr->sslConfiguration == sslConfiguration)
        return *this;
    d_ptr.detach();
    Q_D(QGrpcChannelOptions);
    d->sslConfiguration = sslConfiguration;
    return *this;
}

/*!
    Returns the SSL configuration for the channel.

    If this field is unset, returns an empty \c {std::optional}.
*/
std::optional<QSslConfiguration> QGrpcChannelOptions::sslConfiguration() const
{
    Q_D(const QGrpcChannelOptions);
    return d->sslConfiguration;
}
#endif

/*
//! [compares]
    Returns \c true if the \l{deadlineTimeout}, \l{filterServerMetadata},
    \l{metadata(QtGrpc::MultiValue_t)}, \l{serializationFormat} and
    \l{sslConfiguration} in \a lhs and \a rhs are
//! [compares]
*/
bool comparesEqual(const QGrpcChannelOptions &lhs, const QGrpcChannelOptions &rhs)
{
    return lhs.deadlineTimeout() == rhs.deadlineTimeout()
        && lhs.filterServerMetadata() == rhs.filterServerMetadata()
        && lhs.metadata(QtGrpc::MultiValue) == rhs.metadata(QtGrpc::MultiValue)
        && lhs.serializationFormat() == rhs.serializationFormat()
#if QT_CONFIG(ssl)
        && lhs.sslConfiguration() == rhs.sslConfiguration()
#endif
        ;
}

/*!
    \since 6.11
    \fn bool QGrpcChannelOptions::operator==(const QGrpcChannelOptions &lhs, const QGrpcChannelOptions &rhs)
    \include qgrpcchanneloptions.cpp compares
    equal.
*/

/*!
    \since 6.11
    \fn bool QGrpcChannelOptions::operator!=(const QGrpcChannelOptions &lhs, const QGrpcChannelOptions &rhs)
    \include qgrpcchanneloptions.cpp compares
    not equal.
*/

#ifndef QT_NO_DEBUG_STREAM
/*!
    \since 6.8
    \fn QDebug QGrpcChannelOptions::operator<<(QDebug debug, const QGrpcChannelOptions &chOpts)

    Writes \a chOpts to the specified stream \a debug.
*/
QDebug operator<<(QDebug debug, const QGrpcChannelOptions &chOpts)
{
    const QDebugStateSaver save(debug);
    debug.nospace().noquote();
    debug << "QGrpcChannelOptions(deadline: " << chOpts.deadlineTimeout()
          << ", metadata: " << chOpts.metadata(QtGrpc::MultiValue)
          << ", filterServerMetadata: " << chOpts.filterServerMetadata()
          << ", serializationFormat: " << chOpts.serializationFormat().suffix()
          << ", sslConfiguration: ";
#  if QT_CONFIG(ssl)
    if (chOpts.sslConfiguration())
        debug << "available";
    else
        debug << std::nullopt;
#  else
    debug << "unsupported";
#  endif
    debug << ')';
    return debug;
}
#endif

QT_END_NAMESPACE
