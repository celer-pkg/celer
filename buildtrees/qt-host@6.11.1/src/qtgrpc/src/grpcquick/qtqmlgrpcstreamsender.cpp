// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtGrpcQuick/qtqmlgrpcstreamsender.h>

#include <QtCore/private/qobject_p.h>

QT_BEGIN_NAMESPACE

namespace {
bool isValidStream(const QGrpcOperation *stream)
{
    if (!stream || stream->isFinished()) {
        qWarning("Unable to write message; stream is finished");
        return false;
    }
    return true;
}
}

/*!
    \class QQmlGrpcClientStreamSender
    \internal
*/

class QQmlGrpcClientStreamSenderPrivate : public QObjectPrivate
{
public:
    Q_DECLARE_PUBLIC(QQmlGrpcClientStreamSender)
    explicit QQmlGrpcClientStreamSenderPrivate(QGrpcClientStream *stream) : m_stream(stream) { }
    QGrpcClientStream *m_stream = nullptr;
};

QQmlGrpcClientStreamSender::QQmlGrpcClientStreamSender(QGrpcClientStream *stream, QObject *parent)
    : QObject(*new QQmlGrpcClientStreamSenderPrivate(stream), parent)
{
    QObject::connect(stream, &QObject::destroyed, this, [this]() {
        Q_D(QQmlGrpcClientStreamSender);
        d->m_stream = nullptr;
    });
}

QQmlGrpcClientStreamSender::~QQmlGrpcClientStreamSender()
    = default;

void QQmlGrpcClientStreamSender::writeMessageImpl(const QProtobufMessage &message) const
{
    Q_D(const QQmlGrpcClientStreamSender);
    if (!isValidStream(d->m_stream))
        return;
    d->m_stream->writeMessage(message);
}

/*!
    \class QQmlGrpcBidiStreamSender
    \internal
*/

class QQmlGrpcBidiStreamSenderPrivate : public QObjectPrivate
{
public:
    explicit QQmlGrpcBidiStreamSenderPrivate(QGrpcBidiStream *stream) : m_stream(stream) { }
    QGrpcBidiStream *m_stream = nullptr;
};

QQmlGrpcBidiStreamSender::QQmlGrpcBidiStreamSender(QGrpcBidiStream *stream, QObject *parent)
    : QObject(*new QQmlGrpcBidiStreamSenderPrivate(stream), parent)
{
    QObject::connect(stream, &QObject::destroyed, this, [this]() {
        Q_D(QQmlGrpcBidiStreamSender);
        d->m_stream = nullptr;
    });
}

QQmlGrpcBidiStreamSender::~QQmlGrpcBidiStreamSender()
    = default;

void QQmlGrpcBidiStreamSender::writeMessageImpl(const QProtobufMessage &message) const
{
    Q_D(const QQmlGrpcBidiStreamSender);
    if (!isValidStream(d->m_stream))
        return;
    d->m_stream->writeMessage(message);
}

QT_END_NAMESPACE

#include "moc_qtqmlgrpcstreamsender.cpp"
