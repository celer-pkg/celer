// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QVOICESELECTORATTACHED_H
#define QVOICESELECTORATTACHED_H

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
#include <QtQml/qqml.h>
#include <QtTextToSpeech/qvoice.h>

#include <QtCore/qmap.h>

QT_BEGIN_NAMESPACE

class QDeclarativeTextToSpeech;

class QVoiceSelectorAttached : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QVariant name READ name WRITE setName NOTIFY nameChanged FINAL)
    Q_PROPERTY(QVoice::Gender gender READ gender WRITE setGender NOTIFY genderChanged FINAL)
    Q_PROPERTY(QVoice::Age age READ age WRITE setAge NOTIFY ageChanged FINAL)
    Q_PROPERTY(QLocale locale READ locale WRITE setLocale NOTIFY localeChanged FINAL)
    Q_PROPERTY(QLocale language READ language WRITE setLanguage NOTIFY languageChanged FINAL)

    QML_NAMED_ELEMENT(VoiceSelector)
    QML_ADDED_IN_VERSION(6, 6)
    QML_UNCREATABLE("VoiceSelector is only available via attached properties.")
    QML_ATTACHED(QVoiceSelectorAttached)

public:
    static QVoiceSelectorAttached *qmlAttachedProperties(QObject *obj);

    QVariant name() const;
    void setName(const QVariant &name);

    QVoice::Gender gender() const;
    void setGender(QVoice::Gender gender);

    QVoice::Age age() const;
    void setAge(QVoice::Age age);

    QLocale locale() const;
    void setLocale(const QLocale &locale);

    QLocale language() const;
    void setLanguage(const QLocale &language);

    QVariantMap selectionCriteria() const { return m_criteria; }

public Q_SLOTS:
    void select();

Q_SIGNALS:
    void nameChanged();
    void genderChanged();
    void ageChanged();
    void localeChanged();
    void languageChanged();

private:
    explicit QVoiceSelectorAttached(QDeclarativeTextToSpeech *tts = nullptr);

    QVariantMap m_criteria;
    QDeclarativeTextToSpeech *m_tts;
};

QT_END_NAMESPACE

#endif // QVOICESELECTORATTACHED_H
