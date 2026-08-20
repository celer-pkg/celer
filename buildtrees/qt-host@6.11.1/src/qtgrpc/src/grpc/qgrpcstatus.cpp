// Copyright (C) 2022 The Qt Company Ltd.
// Copyright (C) 2019 Alexey Edelev <semlanik@gmail.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtGrpc/qgrpcstatus.h>

#include <QtCore/qdatastream.h>
#include <QtCore/qdebug.h>
#include <QtCore/qvariant.h>

QT_BEGIN_NAMESPACE

/*!
    \class QGrpcStatus
    \inmodule QtGrpc
    \compares equality
    \compareswith equality QtGrpc::StatusCode
    \endcompareswith
    \brief The QGrpcStatus class combines a \l {QtGrpc::} {StatusCode} and a
    string message.

    The QGrpcStatus class usually provides information about a finished \gRPC
    operation, as returned by the server.

    If a RPC operation failed, contains a \l {QtGrpc::} {StatusCode} other than
    \l {QtGrpc::StatusCode::} {Ok}.
*/

/*!
    \property QGrpcStatus::code
    \brief \l {QtGrpc::} {StatusCode} received for prior \gRPC call.
*/

/*!
    \property QGrpcStatus::message
    \brief Status message received for prior \gRPC call.
*/

/*!
    Constructs a QGrpcStatus with the status code \a code and the string \a
    message.
*/
QGrpcStatus::QGrpcStatus(QtGrpc::StatusCode code, QAnyStringView message)
    : m_code(code), m_message(message.toString())
{
}

/*!
    Destroys the QGrpcStatus.
*/
QGrpcStatus::~QGrpcStatus() = default;

/*!
    Copy-constructs a QGrpcStatus from \a other
*/
QGrpcStatus::QGrpcStatus(const QGrpcStatus &other) = default;

/*!
    Assigns the data of the \a other object to this status object and returns
    a reference to it.
*/
QGrpcStatus &QGrpcStatus::operator=(const QGrpcStatus &other) = default;

/*!
    \fn QGrpcStatus::QGrpcStatus(QGrpcStatus &&other) noexcept

    Move-constructs a new QGrpcStatus from \a other.

    \include qtgrpc-shared.qdocinc move-note-desc
*/

/*!
    \fn QGrpcStatus& QGrpcStatus::operator=(QGrpcStatus &&other) noexcept

    Move-assigns \a other to this QGrpcStatus instance and returns a reference
    to it.

    \include qtgrpc-shared.qdocinc move-note-desc
*/

/*!
    \since 6.8

    \include qtgrpc-shared.qdocinc qvariant-desc
*/
QGrpcStatus::operator QVariant() const
{
    return QVariant::fromValue(*this);
}

/*!
    \since 6.8
    \fn void QGrpcStatus::swap(QGrpcStatus &other) noexcept

    \include qtgrpc-shared.qdocinc swap-desc
*/

/*!
    \fn QtGrpc::StatusCode QGrpcStatus::code() const noexcept

    Returns the contained \l {QtGrpc::} {StatusCode}.
*/

/*!
    \fn const QString &QGrpcStatus::message() const & noexcept
    \fn QString QGrpcStatus::message() && noexcept

    Returns the contained status message.
*/

/*!
    \since 6.8
    \fn bool QGrpcStatus::isOk() const noexcept

    Returns \c true if code() is equal to \l {QtGrpc::StatusCode::} {Ok}.
*/

/*!
    \fn bool QGrpcStatus::operator==(const QGrpcStatus &lhs, const QtGrpc::StatusCode &rhs) noexcept

    Returns \c true if the status codes in \a lhs and \a rhs are equal.
*/

/*!
    \fn bool QGrpcStatus::operator!=(const QGrpcStatus &lhs, const QtGrpc::StatusCode &rhs) noexcept

    Returns \c true if the status codes in \a lhs and \a rhs are not equal.
*/

/*!
    \fn bool QGrpcStatus::operator==(const QGrpcStatus &lhs, const QGrpcStatus &rhs) noexcept

    Returns \c true if the status codes in \a lhs and \a rhs are equal.
*/

/*!
    \fn bool QGrpcStatus::operator!=(const QGrpcStatus &lhs, const QGrpcStatus &rhs) noexcept

    Returns \c true if the status codes in \a lhs and \a rhs are not equal.
*/

/*!
    \since 6.8
    \fn size_t QGrpcStatus::qHash(const QGrpcStatus &key, size_t seed) noexcept

    \include qtgrpc-shared.qdocinc qhash-desc
*/

#ifndef QT_NO_DEBUG_STREAM

/*!
    \since 6.8
    \fn QDebug QGrpcStatus::operator<<(QDebug debug, const QGrpcStatus& status)

    Writes \a status to the specified stream \a debug.
*/
QDebug operator<<(QDebug debug, const QGrpcStatus &status)
{
    const QDebugStateSaver save(debug);
    debug.nospace() << "QGrpcStatus( code: " << status.code() << ", message: " << status.message()
                    << " )";
    return debug;
}

#endif // QT_NO_DEBUG_STREAM

#ifndef QT_NO_DATASTREAM

/*!
    \since 6.8
    \fn QDataStream &QGrpcStatus::operator<<(QDataStream &out, const QGrpcStatus &status)

    Writes the given \a status to the specified stream \a out.
*/
QDataStream &operator<<(QDataStream &out, const QGrpcStatus &status)
{
    out << status.m_code << status.m_message;
    return out;
}

/*!
    \since 6.8
    \fn QDataStream &QGrpcStatus::operator>>(QDataStream &in, QGrpcStatus &status)

    Reads a QGrpcStatus from stream \a in into \a status.
*/
QDataStream &operator>>(QDataStream &in, QGrpcStatus &status)
{
    in >> status.m_code;
    in >> status.m_message;
    return in;
}

#endif // QT_NO_DATASTREAM

QT_END_NAMESPACE

#include "moc_qgrpcstatus.cpp"
