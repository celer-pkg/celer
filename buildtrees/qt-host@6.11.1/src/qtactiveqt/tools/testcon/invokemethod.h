// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef INVOKEMETHOD_H
#define INVOKEMETHOD_H

#include <QtCore/qglobal.h>
#include "ui_invokemethod.h"

QT_BEGIN_NAMESPACE

class QAxBase;

class InvokeMethod : public QDialog, Ui::InvokeMethod
{
    Q_OBJECT
public:
    InvokeMethod(QWidget *parent);

    void setControl(QAxBase *ax);

protected slots:
    void on_buttonInvoke_clicked();
    void on_buttonSet_clicked();

    void on_comboMethods_textActivated(const QString &method);
    void on_listParameters_currentItemChanged(QTreeWidgetItem *item);

private:
    QAxBase *activex;
};

QT_END_NAMESPACE

#endif // INVOKEMETHOD_H
