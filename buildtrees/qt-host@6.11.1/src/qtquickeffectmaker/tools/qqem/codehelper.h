// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef CODEHELPER_H
#define CODEHELPER_H

#include <QObject>
#include <QTimer>
#include <QtQuick/private/qquicktextedit_p_p.h>
#include "codecompletionmodel.h"

class EffectManager;

class CodeHelper : public QObject
{
    Q_OBJECT
    Q_PROPERTY(CodeCompletionModel *codeCompletionModel READ codeCompletionModel NOTIFY codeCompletionModelChanged)
    Q_PROPERTY(bool codeCompletionVisible READ codeCompletionVisible WRITE setCodeCompletionVisible NOTIFY codeCompletionVisibleChanged)

public:
    explicit CodeHelper(QObject *parent = nullptr);

    CodeCompletionModel *codeCompletionModel() const;
    bool codeCompletionVisible() const;

public Q_SLOTS:
    bool processKey(QQuickTextEdit *textEdit, int keyCode, int modifiers);
    QString autoIndentGLSLCode(const QString &code);
    QString autoIndentGLSLNextLine(const QString &codeLine, bool multilineComment);
    QString getCurrentWord(QQuickTextEdit *textEdit);
    void removeCurrentWord(QQuickTextEdit *textEdit);
    void updateCodeCompletion(QQuickTextEdit *textEdit, bool force = false);
    void setCodeCompletionVisible(bool visible);
    void applyCodeCompletion(QQuickTextEdit *textEdit);

signals:
    void codeCompletionModelChanged();
    void codeCompletionVisibleChanged();

private:
    friend class EffectManager;

    void showCodeCompletion();

    EffectManager *m_effectManager = nullptr;
    CodeCompletionModel *m_codeCompletionModel = nullptr;
    QQuickTextEdit *m_textEdit = nullptr;
    bool m_codeCompletionVisible = false;
    QTimer m_codeCompletionTimer;

    QList<QString> m_reservedArgumentNames;
    QList<QString> m_reservedTagNames;
    QList<QString> m_reservedFunctionNames;

};

#endif // CODEHELPER_H
