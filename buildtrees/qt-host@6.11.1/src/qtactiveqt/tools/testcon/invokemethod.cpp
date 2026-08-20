// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "invokemethod.h"

#include <qt_windows.h>
#include <QtAxContainer/QAxBase>
#include <QtWidgets/QCompleter>
#include <QtCore/QMetaObject>
#include <QtCore/QMetaMethod>

QT_BEGIN_NAMESPACE

InvokeMethod::InvokeMethod(QWidget *parent)
: QDialog(parent), activex(nullptr)
{
    setupUi(this);
    auto completer = new QCompleter(comboMethods->model(), comboMethods);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    completer->setCompletionMode(QCompleter::InlineCompletion);
    comboMethods->setCompleter(completer);

    listParameters->setColumnCount(3);
    listParameters->headerItem()->setText(0, tr("Parameter"));
    listParameters->headerItem()->setText(1, tr("Type"));
    listParameters->headerItem()->setText(2, tr("Value"));
}

void InvokeMethod::setControl(QAxBase *ax)
{
    activex = ax;
    bool hasControl = activex && !activex->isNull();
    labelMethods->setEnabled(hasControl);
    comboMethods->setEnabled(hasControl);
    buttonInvoke->setEnabled(hasControl);
    boxParameters->setEnabled(hasControl);

    comboMethods->clear();
    listParameters->clear();

    if (!hasControl) {
        editValue->clear();
        return;
    }

    const QMetaObject *mo = activex->axBaseMetaObject();
    if (mo->methodCount()) {
        for (int i = mo->methodOffset(); i < mo->methodCount(); ++i) {
            const QMetaMethod method = mo->method(i);
            if (method.methodType() == QMetaMethod::Slot)
                comboMethods->addItem(QString::fromLatin1(method.methodSignature()));
        }
        comboMethods->model()->sort(0);

        on_comboMethods_textActivated(comboMethods->currentText());
    }
}

void InvokeMethod::on_buttonInvoke_clicked()
{
    if (!activex)
        return;

    on_buttonSet_clicked();
    QString method = comboMethods->currentText();
    QVariantList vars;

    int itemCount = listParameters->topLevelItemCount();
    for (int i = 0; i < itemCount; ++i) {
        QTreeWidgetItem *parameter = listParameters->topLevelItem(i);
        vars << parameter->text(2);
    }
    QVariant result = activex->dynamicCall(method.toLatin1(), vars);

    int v = 0;
    for (int i = 0; i < itemCount; ++i) {
        QTreeWidgetItem *parameter = listParameters->topLevelItem(i);
        parameter->setText(2, vars[v++].toString());
    }

    editReturn->setText(QString::fromLatin1(result.typeName())
                        + QLatin1Char(' ') + result.toString());
}

void InvokeMethod::on_comboMethods_textActivated(const QString &method)
{
    if (!activex)
        return;
    listParameters->clear();

    const QMetaObject *mo = activex->axBaseMetaObject();
    const QMetaMethod slot = mo->method(mo->indexOfSlot(method.toLatin1()));
    QString signature = QString::fromLatin1(slot.methodSignature());
    signature.remove(0, signature.indexOf(QLatin1Char('(')) + 1);
    signature.truncate(signature.length()-1);

    const auto pnames = slot.parameterNames();
    const auto ptypes = slot.parameterTypes();

    for (qsizetype p = 0; p < ptypes.size(); ++p) {
        QString ptype(QString::fromLatin1(ptypes.at(p)));
        if (ptype.isEmpty())
            continue;
        QString pname(QString::fromLatin1(pnames.at(p).constData()));
        if (pname.isEmpty())
            pname = QString::fromLatin1("<unnamed %1>").arg(p);
        QTreeWidgetItem *item = new QTreeWidgetItem(listParameters);
        item->setText(0, pname);
        item->setText(1, ptype);
    }

    if (listParameters->topLevelItemCount())
        listParameters->setCurrentItem(listParameters->topLevelItem(0));
    editReturn->setText(QString::fromLatin1(slot.typeName()));
}

void InvokeMethod::on_listParameters_currentItemChanged(QTreeWidgetItem *item)
{
    if (!activex)
        return;
    editValue->setEnabled(item != nullptr);
    buttonSet->setEnabled(item != nullptr);
    if (!item)
        return;
    editValue->setText(item->text(2));
}

void InvokeMethod::on_buttonSet_clicked()
{
    if (!activex)
        return;
    QTreeWidgetItem *item = listParameters->currentItem();
    if (!item)
        return;
    item->setText(2, editValue->text());
}

QT_END_NAMESPACE
