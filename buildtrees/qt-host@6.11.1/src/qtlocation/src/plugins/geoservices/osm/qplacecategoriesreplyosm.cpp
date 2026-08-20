// Copyright (C) 2016 Aaron McCarthy <mccarthy.aaron@gmail.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qplacecategoriesreplyosm.h"

QT_BEGIN_NAMESPACE

QPlaceCategoriesReplyOsm::QPlaceCategoriesReplyOsm(QObject *parent)
:   QPlaceReply(parent)
{
}

QPlaceCategoriesReplyOsm::~QPlaceCategoriesReplyOsm()
{
}

void QPlaceCategoriesReplyOsm::emitFinished()
{
    setFinished(true);
    emit finished();
}

void QPlaceCategoriesReplyOsm::setError(QPlaceReply::Error errorCode, const QString &errorString)
{
    QPlaceReply::setError(errorCode, errorString);
    emit errorOccurred(errorCode, errorString);
}

QT_END_NAMESPACE
