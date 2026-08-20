// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QQMLGRPCCHANNELOPTIONS_P_H
#define QQMLGRPCCHANNELOPTIONS_P_H

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

#include <QtGrpcQuick/qtgrpcquickexports.h>

#include <QtGrpc/qgrpcchanneloptions.h>
#include <QtGrpc/qtgrpcnamespace.h>

#include <QtQmlIntegration/qqmlintegration.h>
#if QT_CONFIG(ssl)
#  include <QtQmlNetwork/private/qqmlsslconfiguration_p.h>
#endif

#include <QtCore/qobject.h>

QT_BEGIN_NAMESPACE

class QQmlGrpcMetadata;
class QQmlGrpcChannelOptionsPrivate;
class Q_GRPCQUICK_EXPORT QQmlGrpcChannelOptions : public QObject
{
    Q_OBJECT
    QML_NAMED_ELEMENT(GrpcChannelOptions)
    QML_ADDED_IN_VERSION(6, 7)

    Q_PROPERTY(qint64 deadlineTimeout READ deadlineTimeout WRITE setDeadlineTimeout NOTIFY
                   deadlineTimeoutChanged)
    Q_PROPERTY(QQmlGrpcMetadata *metadata READ metadata WRITE setMetadata NOTIFY metadataChanged)
    Q_PROPERTY(QtGrpc::SerializationFormat serializationFormat
                   READ serializationFormat WRITE setSerializationFormat
                       NOTIFY serializationFormatChanged)
#if QT_CONFIG(ssl)
    Q_PROPERTY(QQmlSslConfiguration sslConfiguration READ sslConfiguration
                       WRITE setSslConfiguration NOTIFY sslConfigurationChanged)
#endif // QT_CONFIG(ssl)

public:
    explicit QQmlGrpcChannelOptions(QObject *parent = nullptr);
    ~QQmlGrpcChannelOptions() override;

    const QGrpcChannelOptions &options() const & noexcept;
    void options() const && = delete;

    qint64 deadlineTimeout() const;
    void setDeadlineTimeout(qint64 value);

    QQmlGrpcMetadata *metadata() const;
    void setMetadata(QQmlGrpcMetadata *value);

    QtGrpc::SerializationFormat serializationFormat() const;
    void setSerializationFormat(QtGrpc::SerializationFormat format);

#if QT_CONFIG(ssl)
    QQmlSslConfiguration sslConfiguration() const;
    void setSslConfiguration(const QQmlSslConfiguration &config);
#endif // QT_CONFIG(ssl)

Q_SIGNALS:
    void deadlineTimeoutChanged();
    void metadataChanged();
    void serializationFormatChanged();
#if QT_CONFIG(ssl)
    void sslConfigurationChanged();
#endif // QT_CONFIG(ssl)

private:
    Q_DECLARE_PRIVATE(QQmlGrpcChannelOptions)
    Q_DISABLE_COPY_MOVE(QQmlGrpcChannelOptions)
};

QT_END_NAMESPACE

#endif // QQMLGRPCCHANNELOPTIONS_P_H
