// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QPLACECONTENT_P_H
#define QPLACECONTENT_P_H

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

#include "qplacecontent.h"
#include "qplacesupplier.h"
#include "qplaceuser.h"

#include <QtCore/QSharedData>
#include <QtCore/QString>
#include <QtCore/QVariant>

QT_BEGIN_NAMESPACE

class QPlaceContentPrivate : public QSharedData
{
public:
    QPlaceContentPrivate(QPlaceContent::Type type)
        : m_type(type)
    {}
    bool compare(const QPlaceContentPrivate *other) const;
    QPlaceContent::Type type() const { return m_type; }

    QMap<QPlaceContent::DataTag, QVariant> data;
    const QPlaceContent::Type m_type = QPlaceContent::NoType;
};

#define Q_IMPLEMENT_CONTENT_COPY_CTOR(ContentClass, ContentType)            \
ContentClass::ContentClass(const QPlaceContent &other)                      \
    : QPlaceContent(other.type() == ContentType ? other : QPlaceContent(ContentType)) \
{                                                                           \
}                                                                           \


QT_END_NAMESPACE

#endif

