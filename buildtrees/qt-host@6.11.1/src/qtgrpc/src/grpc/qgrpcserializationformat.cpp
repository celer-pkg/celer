// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtGrpc/qgrpcserializationformat.h>

#include <QtProtobuf/qprotobufjsonserializer.h>
#include <QtProtobuf/qprotobufserializer.h>

#include <QtCore/qbytearray.h>
#include <QtCore/qdebug.h>
#include <QtCore/qvariant.h>

using namespace QtGrpc;

QT_BEGIN_NAMESPACE

/*!
    \class QGrpcSerializationFormat
    \inmodule QtGrpc
    \compares equality
    \since 6.8
    \brief The QGrpcSerializationFormat class holds the protobuf message
    serializer and the associated content-type suffix.

    The QGrpcSerializationFormat class contains the \l{serializer} used for
    serializing and deserializing protobuf messages, as well as the associated
    content-type \l{suffix}, which indicates the message encoding in transport.
    For HTTP/2 specific details see the \l{Content-Type} section.

    \note The content-type is transport, and therefore implementation specific.

    The class can be constructed using one of the
    \l{QtGrpc::}{SerializationFormat} presets or a custom suffix and
    serializer:

    \code
        QGrpcSerializationFormat jsonFormat(QtGrpc::SerializationFormat::Json);
    \endcode

    This creates a QProtobufJsonSerializer with the \c{json} suffix. For HTTP/2
    transportation this results in the \c{application/grpc+json} content-type.

//! [custom-serializer-code]
    \code
        class DummySerializer : public QAbstractProtobufSerializer
        {
            ...
        };
        QGrpcSerializationFormat dummyFormat("dummy", std::make_shared<DummySerializer>());
    \endcode
//! [custom-serializer-code]

//! [custom-serializer-desc]
    This uses \c{DummySerializer} for encoding and decoding messages with the
    \c{dummy} suffix. For HTTP/2 transportation this results in the
    \c{application/grpc+dummy} content-type.

    \note Custom serializers require server support for the specified format.
//! [custom-serializer-desc]

    \sa QGrpcChannelOptions::serializationFormat
*/

class QGrpcSerializationFormatPrivate : public QSharedData
{
public:
    QGrpcSerializationFormatPrivate(QByteArrayView suffix_,
                                    std::shared_ptr<QAbstractProtobufSerializer> serializer_)
        : suffix(suffix_.toByteArray()), serializer(std::move(serializer_))
    {
    }

    QByteArray suffix;
    std::shared_ptr<QAbstractProtobufSerializer> serializer;
};

QT_DEFINE_QESDP_SPECIALIZATION_DTOR(QGrpcSerializationFormatPrivate)

/*!
    Constructs a new QGrpcSerializationFormat with the specified preset \a
    format.

    The default format used is \l {QtGrpc::} {SerializationFormat::Default}.
*/
QGrpcSerializationFormat::QGrpcSerializationFormat(SerializationFormat format)
    : d_ptr(format == SerializationFormat::Json
                ? new QGrpcSerializationFormatPrivate("json",
                                                      std::make_shared<QProtobufJsonSerializer>())
                : new QGrpcSerializationFormatPrivate(format == SerializationFormat::Protobuf
                                                          ? "proto"
                                                          : "",
                                                      std::make_shared<QProtobufSerializer>()))
{
}

/*!
    Destroys the QGrpcSerializationFormat.
*/
QGrpcSerializationFormat::~QGrpcSerializationFormat() = default;

/*!
    Constructs a new QGrpcSerializationFormat with a custom content type
    specified by \a suffix and a protobuf message \a serializer.
*/
QGrpcSerializationFormat::QGrpcSerializationFormat(QByteArrayView suffix,
                                                   std::shared_ptr<QAbstractProtobufSerializer>
                                                       serializer)
    : d_ptr(new QGrpcSerializationFormatPrivate(suffix, std::move(serializer)))
{
}

/*!
    Constructs a copy of \a other.
*/
QGrpcSerializationFormat::QGrpcSerializationFormat(const QGrpcSerializationFormat &other) = default;

/*!
    Assigns the \a other QGrpcSerializationFormat object to this one.
*/
QGrpcSerializationFormat &
QGrpcSerializationFormat::operator=(const QGrpcSerializationFormat &other) = default;

/*!
    \fn QGrpcSerializationFormat::QGrpcSerializationFormat(QGrpcSerializationFormat &&other) noexcept
    Move-constructs a new QGrpcSerializationFormat from \a other.

    \include qtgrpc-shared.qdocinc move-note-desc
*/

/*!
    \fn QGrpcSerializationFormat &QGrpcSerializationFormat::operator=(QGrpcSerializationFormat &&other) noexcept
    Move-assigns \a other to this QGrpcSerializationFormat instance and returns
    a reference to it.

    \include qtgrpc-shared.qdocinc move-note-desc
*/

/*!
    \since 6.8
    \fn void QGrpcSerializationFormat::swap(QGrpcSerializationFormat &other) noexcept

    \include qtgrpc-shared.qdocinc swap-desc
*/

/*!
    \since 6.8

    \include qtgrpc-shared.qdocinc qvariant-desc
*/
QGrpcSerializationFormat::operator QVariant() const
{
    return QVariant::fromValue(*this);
}

/*!
    Returns the content type suffix for this serialization format.
*/
QByteArrayView QGrpcSerializationFormat::suffix() const noexcept
{
    Q_D(const QGrpcSerializationFormat);
    return d->suffix;
}

/*!
    Returns the serializer for this serialization format.

    /sa QAbstractProtobufSerializer
*/
std::shared_ptr<QAbstractProtobufSerializer> QGrpcSerializationFormat::serializer() const
{
    Q_D(const QGrpcSerializationFormat);
    return d->serializer;
}

#ifndef QT_NO_DEBUG_STREAM
/*!
    \since 6.8
    \fn QDebug QGrpcSerializationFormat::operator<<(QDebug debug, const QGrpcSerializationFormat &sfmt)
    Writes \a sfmt to the specified stream \a debug.
*/
QDebug operator<<(QDebug debug, const QGrpcSerializationFormat &sfmt)
{
    const QDebugStateSaver save(debug);
    debug.nospace().noquote();
    debug << "QGrpcSerializationFormat(suffix: " << sfmt.suffix() << ", serializer: { "
          << static_cast<void *>(sfmt.serializer().get())
          << ", useCount: " << sfmt.serializer().use_count() << " })";
    return debug;
}
#endif

/*
//! [compares]
    Returns \c true if the \l{suffix} and \l{serializer}, in \a lhs and \a rhs are
//! [compares]
*/
bool comparesEqual(const QGrpcSerializationFormat &lhs,
                   const QGrpcSerializationFormat &rhs) noexcept
{
    return lhs.d_func()->suffix == rhs.d_func()->suffix
        && lhs.d_func()->serializer == rhs.d_func()->serializer;
}

/*!
    \since 6.8
    \fn bool QGrpcSerializationFormat::operator==(const QGrpcSerializationFormat &lhs, const QGrpcSerializationFormat &rhs)
    \include qgrpcserializationformat.cpp compares
    equal.
*/

/*!
    \since 6.8
    \fn bool QGrpcSerializationFormat::operator!=(const QGrpcSerializationFormat &lhs, const QGrpcSerializationFormat &rhs)
    \include qgrpcserializationformat.cpp compares
    not equal.
*/

/*!
    \since 6.8
    \fn size_t QGrpcSerializationFormat::qHash(const QGrpcSerializationFormat &key)

    Returns the hash value of \a key, using \c{0} to seed the calculation.
*/

/*!
    \since 6.8
    \fn size_t QGrpcSerializationFormat::qHash(const QGrpcSerializationFormat &key, size_t seed)

    \include qtgrpc-shared.qdocinc qhash-desc
*/
size_t qHash(const QGrpcSerializationFormat &key, size_t seed) noexcept
{
    return qHashMulti(seed, key.d_func()->suffix, key.d_func()->serializer.get());
}

QT_END_NAMESPACE
