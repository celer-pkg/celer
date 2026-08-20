// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QVOICESELECTOR_H
#define QVOICESELECTOR_H

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

#include <QtCore/qobject.h>
#include <QtCore/qregularexpression.h>
#include <QtQml/qqml.h>
#include <QtTextToSpeech/qvoice.h>

QT_BEGIN_NAMESPACE

struct QVoiceSelector
{
    Q_GADGET
    QML_VALUE_TYPE(voiceSelector)
    QML_STRUCTURED_VALUE

    Q_PROPERTY(QRegularExpression name READ name WRITE setName);
    Q_PROPERTY(QLocale locale READ locale WRITE setLocale);
    Q_PROPERTY(QVoice::Age age READ age WRITE setAge);
    Q_PROPERTY(QVoice::Gender gender READ gender WRITE setGender);

public:
    QRegularExpression name() const { return m_name; }
    void setName(const QRegularExpression &name) { m_name = name; }
    QLocale locale() const { return m_locale; }
    void setLocale(const QLocale &locale) { m_locale = locale; }
    QVoice::Age age() const { return m_age; }
    void setAge(QVoice::Age age) { m_age = age; }
    QVoice::Gender gender() const { return m_gender; }
    void setGender(QVoice::Gender gender) { m_gender = gender; }

    QRegularExpression m_name;
    QLocale m_locale = QLocale::C;
    QVoice::Age m_age = QVoice::Age(-1);
    QVoice::Gender m_gender = QVoice::Gender(-1);
};

QT_END_NAMESPACE

#endif // QVOICESELECTOR_H
