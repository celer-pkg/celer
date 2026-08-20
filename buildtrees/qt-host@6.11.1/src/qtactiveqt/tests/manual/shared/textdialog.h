// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtWidgets/QDialog>

#ifndef TEXTDIALOG_H
#define TEXTDIALOG_H

class TextDialog : public QDialog
{
public:
    explicit TextDialog(const QString &text, QWidget *parent = nullptr);
};

#endif
