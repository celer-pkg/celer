// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtGrpc/qtgrpcnamespace.h>

#include <QtCore/qdebug.h>

QT_BEGIN_NAMESPACE

namespace QtGrpc {

#ifndef QT_NO_DEBUG_STREAM

/*!
    \fn QDebug QtGrpc::RpcDescriptor::operator<<(QDebug debug, const QtGrpc::RpcDescriptor& description)
    Writes \a description to the specified stream \a debug.
*/
QDebug operator<<(QDebug debug, const QtGrpc::RpcDescriptor &description)
{
    const QDebugStateSaver save(debug);
    debug.nospace() << "QtGrpc::RpcDescriptor( service: " << description.service
                    << ", method: " << description.method << ", type: " << description.type << " )";
    return debug;
}

#endif // QT_NO_DEBUG_STREAM

} // namespace QtGrpc

QT_END_NAMESPACE
