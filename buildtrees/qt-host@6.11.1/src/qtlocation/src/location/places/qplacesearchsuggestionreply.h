// Copyright (C) 2015 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QPLACESEARCHSUGGESTIONREPLY_H
#define QPLACESEARCHSUGGESTIONREPLY_H

#include <QtLocation/QPlaceReply>

#include <QtCore/QStringList>

QT_BEGIN_NAMESPACE

class QPlaceSearchSuggestionReplyPrivate;

class Q_LOCATION_EXPORT QPlaceSearchSuggestionReply : public QPlaceReply
{
    Q_OBJECT
public:
    explicit QPlaceSearchSuggestionReply(QObject *parent = nullptr);
    ~QPlaceSearchSuggestionReply();

    QStringList suggestions() const;
    Type type() const override;

protected:
    void setSuggestions(const QStringList &suggestions);

private:
    Q_DISABLE_COPY(QPlaceSearchSuggestionReply)
    Q_DECLARE_PRIVATE(QPlaceSearchSuggestionReply)
};

QT_END_NAMESPACE

#endif
