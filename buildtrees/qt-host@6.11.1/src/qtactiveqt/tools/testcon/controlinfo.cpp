// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "controlinfo.h"

#include <QtCore/QMetaClassInfo>

QT_BEGIN_NAMESPACE

ControlInfo::ControlInfo(QWidget *parent)
    : QDialog(parent)
{
    setupUi(this);

    listInfo->setColumnCount(2);
    listInfo->headerItem()->setText(0, tr("Item"));
    listInfo->headerItem()->setText(1, tr("Details"));
}

void ControlInfo::setControl(QWidget *activex)
{
    listInfo->clear();

    const QMetaObject *mo = activex->metaObject();
    QTreeWidgetItem *group = new QTreeWidgetItem(listInfo);
    group->setText(0, tr("Class Info"));
    group->setText(1, QString::number(mo->classInfoCount()));

    QTreeWidgetItem *item = nullptr;
    int i;
    int count;
    for (i = mo->classInfoOffset(); i < mo->classInfoCount(); ++i) {
        const QMetaClassInfo info = mo->classInfo(i);
        item = new QTreeWidgetItem(group);
        item->setText(0, QString::fromLatin1(info.name()));
        item->setText(1, QString::fromLatin1(info.value()));
    }
    group = new QTreeWidgetItem(listInfo);
    group->setText(0, tr("Signals"));

    count = 0;
    for (i = mo->methodOffset(); i < mo->methodCount(); ++i) {
        const QMetaMethod method = mo->method(i);
        if (method.methodType() == QMetaMethod::Signal) {
            ++count;
            item = new QTreeWidgetItem(group);
            item->setText(0, QString::fromLatin1(method.methodSignature()));
        }
    }
    group->setText(1, QString::number(count));

    group = new QTreeWidgetItem(listInfo);
    group->setText(0, tr("Slots"));

    count = 0;
    for (i = mo->methodOffset(); i < mo->methodCount(); ++i) {
        const QMetaMethod method = mo->method(i);
        if (method.methodType() == QMetaMethod::Slot) {
            ++count;
            item = new QTreeWidgetItem(group);
            item->setText(0, QString::fromLatin1(method.methodSignature()));
        }
    }
    group->setText(1, QString::number(count));

    group = new QTreeWidgetItem(listInfo);
    group->setText(0, tr("Properties"));

    count = 0;
    for (i = mo->propertyOffset(); i < mo->propertyCount(); ++i) {
        ++count;
        const QMetaProperty property = mo->property(i);
        item = new QTreeWidgetItem(group);
        item->setText(0, QString::fromLatin1(property.name()));
        item->setText(1, QString::fromLatin1(property.typeName()));
        if (!property.isDesignable()) {
            item->setForeground(0, Qt::gray);
            item->setForeground(1, Qt::gray);
        }
    }
    group->setText(1, QString::number(count));
}

QT_END_NAMESPACE
