// Copyright (C) 2015 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QPLACECATEGORIESREPLYHERE_H
#define QPLACECATEGORIESREPLYHERE_H

#include <QtLocation/QPlaceReply>

QT_BEGIN_NAMESPACE

class QPlaceCategoriesReplyHere : public QPlaceReply
{
    Q_OBJECT

public:
    explicit QPlaceCategoriesReplyHere(QObject *parent = nullptr);
    ~QPlaceCategoriesReplyHere();

    void emitFinished();

private slots:
    void setError(QPlaceReply::Error error_, const QString &errorString);
};

QT_END_NAMESPACE

#endif // QPLACECATEGORIESREPLYHERE_H
