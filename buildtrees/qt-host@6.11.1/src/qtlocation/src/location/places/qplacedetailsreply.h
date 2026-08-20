// Copyright (C) 2015 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QPLACEDETAILSREPLY_H
#define QPLACEDETAILSREPLY_H

#include <QtLocation/QPlaceReply>

QT_BEGIN_NAMESPACE

class QPlace;
class QPlaceDetailsReplyPrivate;
class Q_LOCATION_EXPORT QPlaceDetailsReply : public QPlaceReply
{
    Q_OBJECT
public:
    explicit QPlaceDetailsReply(QObject *parent = nullptr);
    virtual ~QPlaceDetailsReply();

    QPlaceReply::Type type() const override;

    QPlace place() const;

protected:
    void setPlace(const QPlace &place);

private:
    Q_DISABLE_COPY(QPlaceDetailsReply)
    Q_DECLARE_PRIVATE(QPlaceDetailsReply)
};

QT_END_NAMESPACE

#endif // QPLACEDETAILSREPLY_H
