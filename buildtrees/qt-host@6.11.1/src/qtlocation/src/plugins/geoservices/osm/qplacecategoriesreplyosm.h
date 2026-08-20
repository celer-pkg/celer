// Copyright (C) 2016 Aaron McCarthy <mccarthy.aaron@gmail.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QPLACECATEGORIESREPLYOSM_H
#define QPLACECATEGORIESREPLYOSM_H

#include <QtLocation/QPlaceReply>

QT_BEGIN_NAMESPACE

class QPlaceCategoriesReplyOsm : public QPlaceReply
{
    Q_OBJECT

public:
    explicit QPlaceCategoriesReplyOsm(QObject *parent = nullptr);
    ~QPlaceCategoriesReplyOsm();

    void emitFinished();
    void setError(QPlaceReply::Error errorCode, const QString &errorString);
};

QT_END_NAMESPACE

#endif // QPLACECATEGORIESREPLYOSM_H
