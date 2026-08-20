// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QQMLGRPCCMETADATA_P_H
#define QQMLGRPCCMETADATA_P_H

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

#include <QtQmlIntegration/qqmlintegration.h>

#include <QtCore/qbytearray.h>
#include <QtCore/qhash.h>
#include <QtCore/qobject.h>
#include <QtCore/qvariantmap.h>

QT_BEGIN_NAMESPACE

class Q_GRPCQUICK_EXPORT QQmlGrpcMetadata : public QObject
{
    Q_OBJECT
    QML_NAMED_ELEMENT(GrpcMetadata)
    QML_ADDED_IN_VERSION(6, 7)

    Q_PROPERTY(QVariantMap data READ data WRITE setData NOTIFY dataChanged REQUIRED)
public:
    explicit QQmlGrpcMetadata(QObject *parent = nullptr);
    ~QQmlGrpcMetadata() override;

    const QMultiHash<QByteArray, QByteArray> &metadata() const & noexcept { return m_metadata; }
    void metadata() const && = delete;

    const QVariantMap &data() const { return m_variantdata; }
    void setData(const QVariantMap &data);

Q_SIGNALS:
    void dataChanged();

private:
    QVariantMap m_variantdata;
    QMultiHash<QByteArray, QByteArray> m_metadata;

    Q_DISABLE_COPY_MOVE(QQmlGrpcMetadata)
};

QT_END_NAMESPACE

#endif // QQMLGRPCCMETADATA_P_H
