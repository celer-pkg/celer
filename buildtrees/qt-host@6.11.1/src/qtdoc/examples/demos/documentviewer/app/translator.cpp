// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "translator.h"
#include <QApplication>
#include <QDir>

using namespace Qt::StringLiterals;

void Translator::setLanguage(QLocale::Language lang)
{
    m_trLocale = QLocale(lang);
}

void Translator::install()
{
    if (m_baseName.isEmpty()) {
        qWarning() << "The basename of the translation is not set. Ignoring.";
        return;
    }
    if (!m_translator.isEmpty())
        qApp->removeTranslator(&m_translator);

    if (m_translator.load(m_trLocale, m_baseName, "_"_L1, ":/i18n/"_L1)
        && qApp->installTranslator(&m_translator)) {
        qInfo() << "Loaded translation" << m_translator.filePath();
    } else {
        if (m_trLocale.language() != QLocale::English) {
            qWarning() << "Failed to load translation" << m_baseName <<
                    "for locale" << m_trLocale.name() << ". Falling back to English translation";
            setLanguage(QLocale::English);
        }
    }
}

Translator::~Translator()
{
    qApp->removeTranslator(&m_translator);
}
