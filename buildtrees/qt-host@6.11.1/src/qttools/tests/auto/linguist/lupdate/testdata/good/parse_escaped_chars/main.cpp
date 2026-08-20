// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtWidgets/QDialog>

class Dialog2 : public QDialog
{
    Q_OBJECT
    void func();
};

void Dialog2::func()
{
    tr("prefix\u00A0\u00A0postfix");
    tr("before\U000000A0middle\U000000A0after");
    tr("before\u00A0\U000000A0middle\x1F\U000000A0\u00A0after");
}
