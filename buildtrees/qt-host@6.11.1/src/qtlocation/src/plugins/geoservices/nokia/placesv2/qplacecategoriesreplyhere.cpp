// Copyright (C) 2015 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qplacecategoriesreplyhere.h"

QT_BEGIN_NAMESPACE

QPlaceCategoriesReplyHere::QPlaceCategoriesReplyHere(QObject *parent)
:   QPlaceReply(parent)
{
}

QPlaceCategoriesReplyHere::~QPlaceCategoriesReplyHere()
{
}

void QPlaceCategoriesReplyHere::emitFinished()
{
    setFinished(true);
    emit finished();
}

void QPlaceCategoriesReplyHere::setError(QPlaceReply::Error error_, const QString &errorString)
{
    QPlaceReply::setError(error_, errorString);
    emit errorOccurred(error_, errorString);
}

QT_END_NAMESPACE
