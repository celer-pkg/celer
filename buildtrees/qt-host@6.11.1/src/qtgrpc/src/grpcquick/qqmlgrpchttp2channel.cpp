// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtGrpcQuick/qqmlabstractgrpcchannel.h>

#include <QtGrpcQuick/private/qqmlgrpchttp2channel_p.h>
#include <QtGrpcQuick/private/qqmlabstractgrpcchannel_p.h>

#include <QtGrpc/qgrpchttp2channel.h>

#include <QtCore/qdebug.h>

QT_BEGIN_NAMESPACE

namespace {
#if QT_CONFIG(ssl)
constexpr size_t QQmlGrpcChannelOptionsPropertiesCount = 4;
#else
constexpr size_t QQmlGrpcChannelOptionsPropertiesCount = 3;
#endif
}

class QQmlGrpcHttp2ChannelPrivate : public QQmlAbstractGrpcChannelPrivate
{
public:
    QQmlGrpcChannelOptions *options = nullptr;
    QUrl hostUri;
    std::shared_ptr<QAbstractGrpcChannel> channel;
    std::array<QMetaObject::Connection, QQmlGrpcChannelOptionsPropertiesCount> optionsConnections;
};

QQmlGrpcHttp2Channel::QQmlGrpcHttp2Channel(QObject *parent)
    : QQmlAbstractGrpcChannel(*(new QQmlGrpcHttp2ChannelPrivate), parent)
{
}

QQmlGrpcHttp2Channel::~QQmlGrpcHttp2Channel() = default;

std::shared_ptr<QAbstractGrpcChannel> QQmlGrpcHttp2Channel::channel() const
{
    Q_D(const QQmlGrpcHttp2Channel);
    return d->channel;
}

void QQmlGrpcHttp2Channel::setOptions(QQmlGrpcChannelOptions *options)
{
    Q_D(QQmlGrpcHttp2Channel);
    if (d->options == options)
        return;

    for (auto &connection : d->optionsConnections) {
        if (connection) {
            disconnect(connection);
            connection = {};
        }
    }

    d->options = options;
    if (d->options) {
        auto updateChannelOptions = [this]() {
            Q_D(QQmlGrpcHttp2Channel);
            if (d->channel) {
                d->channel->setChannelOptions(d->options != nullptr ? d->options->options()
                                                                    : QGrpcChannelOptions{});
            }
            emit optionsChanged();
        };

        d->optionsConnections[0] = connect(d->options,
                                           &QQmlGrpcChannelOptions::deadlineTimeoutChanged, this,
                                           updateChannelOptions);
        d->optionsConnections[1] = connect(d->options, &QQmlGrpcChannelOptions::metadataChanged,
                                           this, updateChannelOptions);
        d->optionsConnections[2] = connect(d->options,
                                           &QQmlGrpcChannelOptions::serializationFormatChanged,
                                           this, updateChannelOptions);
#if QT_CONFIG(ssl)
        d->optionsConnections[3] = connect(d->options,
                                           &QQmlGrpcChannelOptions::sslConfigurationChanged, this,
                                           updateChannelOptions);
#endif
        updateChannelOptions();
    }
}

void QQmlGrpcHttp2Channel::updateChannel()
{
    Q_D(QQmlGrpcHttp2Channel);
    if (d->hostUri.isEmpty())
        return;

    if (!d->hostUri.isValid()) {
        qWarning() << "Unable to initialize the channel. The host URI is not valid.";
        return;
    }

    if (d->channel)
        d->channel.reset();

    if (d->hostUri.isValid()) {
        d->channel = d->options
            ? std::make_shared<QGrpcHttp2Channel>(d->hostUri, d->options->options())
            : std::make_shared<QGrpcHttp2Channel>(d->hostUri);
    }
    emit channelUpdated();
}

void QQmlGrpcHttp2Channel::setHostUri(const QUrl &hostUri)
{
    Q_D(QQmlGrpcHttp2Channel);
    if (hostUri == d->hostUri)
        return;

    if (d->channel) {
        qWarning() << "Changing the host URI is not supported.";
        return;
    }

    d->hostUri = hostUri;

    emit hostUriChanged();
    updateChannel();
}

QQmlGrpcChannelOptions *QQmlGrpcHttp2Channel::options() const noexcept
{
    Q_D(const QQmlGrpcHttp2Channel);
    return d->options;
}

QUrl QQmlGrpcHttp2Channel::hostUri() const noexcept
{
    Q_D(const QQmlGrpcHttp2Channel);
    return d->hostUri;
}

QT_END_NAMESPACE

#include "moc_qqmlgrpchttp2channel_p.cpp"
