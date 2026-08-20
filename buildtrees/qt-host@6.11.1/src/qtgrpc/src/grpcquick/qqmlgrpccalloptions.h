// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QQMLGRPCCALLOPTIONS_H
#define QQMLGRPCCALLOPTIONS_H

#include <QtGrpcQuick/qtgrpcquickexports.h>

#include <QtGrpc/qgrpccalloptions.h>

#include <QtQmlIntegration/qqmlintegration.h>

#include <QtCore/qobject.h>

QT_BEGIN_NAMESPACE

class QQmlGrpcMetadata;

namespace QtGrpcQuickPrivate {

class QQmlGrpcCallOptionsPrivate;
class Q_GRPCQUICK_EXPORT QQmlGrpcCallOptions : public QObject
{
    Q_OBJECT
    QML_NAMED_ELEMENT(GrpcCallOptions)
    QML_ADDED_IN_VERSION(6, 7)

    Q_PROPERTY(qint64 deadlineTimeout READ deadlineTimeout WRITE setDeadlineTimeout NOTIFY
                   deadlineTimeoutChanged)
    Q_PROPERTY(QQmlGrpcMetadata *metadata READ metadata WRITE setMetadata NOTIFY metadataChanged)

public:
    explicit QQmlGrpcCallOptions(QObject *parent = nullptr);
    ~QQmlGrpcCallOptions() override;

    const QGrpcCallOptions &options() const & noexcept;
    void options() const && = delete;

    qint64 deadlineTimeout() const;
    void setDeadlineTimeout(qint64 value);

    QQmlGrpcMetadata *metadata() const;
    void setMetadata(QQmlGrpcMetadata *value);

Q_SIGNALS:
    void deadlineTimeoutChanged();
    void metadataChanged();

private:
    Q_DECLARE_PRIVATE(QQmlGrpcCallOptions)
};

}
QT_END_NAMESPACE

#endif // QQMLGRPCCALLOPTIONS_H
