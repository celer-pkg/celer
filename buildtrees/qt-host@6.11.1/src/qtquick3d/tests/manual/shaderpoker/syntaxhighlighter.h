// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef SYNTAXHIGHLIGHTER_H
#define SYNTAXHIGHLIGHTER_H

#include <QtQml/qqmlregistration.h>

#include <QtGui/qsyntaxhighlighter.h>
#include <QtQuick/qquicktextdocument.h>

#include <QtCore/qpointer.h>

QT_BEGIN_NAMESPACE

class SyntaxHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT
    Q_PROPERTY(QQuickTextDocument * document READ document WRITE setDocument NOTIFY documentChanged)
    QML_ELEMENT
public:
    enum BlockState
    {
        None,
        Comment
    };

    explicit SyntaxHighlighter(QObject *p = nullptr);

    // Shadows
    QQuickTextDocument *document() const;
    void setDocument(QQuickTextDocument *newDocument);

signals:
    void documentChanged();

protected:
    void highlightBlock(const QString &text) final;
    QPointer<QQuickTextDocument> m_quickTextDocument;
    QSet<QByteArrayView> m_keywords;
    QSet<QByteArrayView> m_argumentKeywords;

private:
    void highlightLine(const QString &text, int position, int length, const QTextCharFormat &format);
};

QT_END_NAMESPACE

#endif // SYNTAXHIGHLIGHTER_H
