// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef TRANSLATOR_H
#define TRANSLATOR_H

#include "abstractviewerglobal.h"

#include <QLocale>
#include <QTranslator>

class ABSTRACTVIEWER_EXPORT Translator
{
public:
    ~Translator();
    void setBaseName(const QString &baseName) {m_baseName = baseName;}
    void setLanguage(QLocale::Language lang);
    void install();
private:
    QTranslator m_translator;
    QString m_baseName;
    QLocale m_trLocale;
};

#endif // TRANSLATOR_H
