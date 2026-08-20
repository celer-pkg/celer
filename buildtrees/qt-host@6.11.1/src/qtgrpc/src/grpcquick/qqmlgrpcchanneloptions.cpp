// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtGrpcQuick/private/qqmlgrpcchanneloptions_p.h>
#include <QtGrpcQuick/private/qqmlgrpcmetadata_p.h>

#include <QtGrpc/qgrpcserializationformat.h>

#include <QtCore/private/qobject_p.h>

#include <chrono>

using namespace std::chrono_literals;

QT_BEGIN_NAMESPACE

class QQmlGrpcChannelOptionsPrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(QQmlGrpcChannelOptions)

public:
    QQmlGrpcChannelOptionsPrivate();

    QGrpcChannelOptions options;
    QtGrpc::SerializationFormat format = QtGrpc::SerializationFormat::Default;
    QQmlGrpcMetadata *metadata = nullptr;
#if QT_CONFIG(ssl)
    QQmlSslConfiguration configuration;
#endif // QT_CONFIG(ssl)
    QMetaObject::Connection metadataUpdateConnection;
};

QQmlGrpcChannelOptionsPrivate::QQmlGrpcChannelOptionsPrivate() : QObjectPrivate()
{
}

QQmlGrpcChannelOptions::QQmlGrpcChannelOptions(QObject *parent)
    : QObject(*(new QQmlGrpcChannelOptionsPrivate()), parent)
{
}

QQmlGrpcChannelOptions::~QQmlGrpcChannelOptions() = default;

qint64 QQmlGrpcChannelOptions::deadlineTimeout() const
{
    std::chrono::milliseconds ms = d_func()->options.deadlineTimeout().value_or(0ms);
    return ms.count();
}

void QQmlGrpcChannelOptions::setDeadlineTimeout(qint64 value)
{
    Q_D(QQmlGrpcChannelOptions);
    std::chrono::milliseconds ms(value);
    d->options.setDeadlineTimeout(ms);
    emit deadlineTimeoutChanged();
}

const QGrpcChannelOptions &QQmlGrpcChannelOptions::options() const & noexcept
{
    return d_func()->options;
}

QQmlGrpcMetadata *QQmlGrpcChannelOptions::metadata() const
{
    return d_func()->metadata;
}

void QQmlGrpcChannelOptions::setMetadata(QQmlGrpcMetadata *value)
{
    Q_D(QQmlGrpcChannelOptions);
    if (d->metadata != value) {
        if (d->metadataUpdateConnection) {
            disconnect(d->metadataUpdateConnection);
            d->metadataUpdateConnection = {};
        }

        d->metadata = value;
        if (d->metadata) {
            const auto updateMetadata = [this]() {
                Q_D(QQmlGrpcChannelOptions);
                d->options.setMetadata(d->metadata->metadata());
                emit metadataChanged();
            };
            d->metadataUpdateConnection = connect(d->metadata, &QQmlGrpcMetadata::dataChanged, this,
                                                  updateMetadata);
            updateMetadata();
        }
    }
}

QtGrpc::SerializationFormat QQmlGrpcChannelOptions::serializationFormat() const
{
    return d_func()->format;
}

void QQmlGrpcChannelOptions::setSerializationFormat(QtGrpc::SerializationFormat format)
{
    Q_D(QQmlGrpcChannelOptions);
    if (d->format != format) {
        d->format = format;
        d->options.setSerializationFormat(format);
        emit serializationFormatChanged();
    }
}

#if QT_CONFIG(ssl)
QQmlSslConfiguration QQmlGrpcChannelOptions::sslConfiguration() const
{
    return d_func()->configuration;
}

void QQmlGrpcChannelOptions::setSslConfiguration(const QQmlSslConfiguration &config)
{
    Q_D(QQmlGrpcChannelOptions);
    if (d->configuration != config) {
        d->configuration = config;
        d->options.setSslConfiguration(d->configuration.configuration());
        emit sslConfigurationChanged();
    }
}
#endif // QT_CONFIG(ssl)

QT_END_NAMESPACE

#include "moc_qqmlgrpcchanneloptions_p.cpp"
