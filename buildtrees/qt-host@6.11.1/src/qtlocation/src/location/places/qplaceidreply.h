// Copyright (C) 2015 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QPLACEIDREPLY_H
#define QPLACEIDREPLY_H

#include <QtLocation/QPlaceReply>

QT_BEGIN_NAMESPACE

class QPlaceIdReplyPrivate;
class Q_LOCATION_EXPORT QPlaceIdReply : public QPlaceReply
{
    Q_OBJECT
public:
    enum OperationType
    {
        SavePlace,
        SaveCategory,
        RemovePlace,
        RemoveCategory
    };

    explicit QPlaceIdReply(OperationType operationType, QObject *parent = nullptr);
    virtual ~QPlaceIdReply();

    QPlaceReply::Type type() const override;
    OperationType operationType() const;

    QString id() const;

protected:
    void setId(const QString &identifier);
private:
    Q_DISABLE_COPY(QPlaceIdReply)
    Q_DECLARE_PRIVATE(QPlaceIdReply)
};

QT_END_NAMESPACE

#endif
