// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef PLAINTEXTEDIT_H
#define PLAINTEXTEDIT_H

#include <QMenu>
#include <QPlainTextEdit>

class PlainTextEdit : public QPlainTextEdit
{
    Q_OBJECT
    Q_DISABLE_COPY(PlainTextEdit)

public Q_SLOT:
    void setFocus() {
        QWidget::setFocus();
    }

public:
    explicit PlainTextEdit(QWidget *parent = nullptr)
        : QPlainTextEdit(parent)
    {}

    void keyPressEvent(QKeyEvent *e)
    {
        switch (e->key()) {
        case Qt::Key_Delete:
        case Qt::Key_Backspace:
            setTextInteractionFlags(textInteractionFlags() | Qt::TextEditable);
            QPlainTextEdit::keyPressEvent(e);
            setTextInteractionFlags(textInteractionFlags() &~ Qt::TextEditable);
            break;
        default:
            QPlainTextEdit::keyPressEvent(e);
        }
    }

    void contextMenuEvent(QContextMenuEvent *event)
    {
        QMenu menu(this);
        menu.addAction(tr("Clear"), this, &QPlainTextEdit::clear);
#ifndef QT_NO_CLIPBOARD
        menu.addAction(tr("Copy"), QKeySequence::Copy, this, &QPlainTextEdit::copy);
#endif
        menu.addSeparator();
        menu.addAction(tr("Select All"), QKeySequence::SelectAll,
                       this, &QPlainTextEdit::selectAll);
        menu.exec(event->globalPos());
    }
};

#endif // PLAINTEXTEDIT_H
