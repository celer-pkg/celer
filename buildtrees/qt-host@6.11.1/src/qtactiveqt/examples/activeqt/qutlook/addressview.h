// Copyright (C) 2015 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef ADDRESSVIEW_H
#define ADDRESSVIEW_H

#include <QWidget>

class AddressBookModel;
QT_BEGIN_NAMESPACE
class QLineEdit;
class QModelIndex;
class QPushButton;
class QTreeView;
QT_END_NAMESPACE

//! [0]
class AddressView : public QWidget
{
    Q_OBJECT

public:
    explicit AddressView(QWidget *parent = nullptr);

protected slots:
    void addEntry();
    void changeEntry();
    void itemSelected(const QModelIndex &index);

    void updateOutlook();

protected:
    AddressBookModel *model;

    QTreeView *m_treeView;
    QPushButton *m_addButton;
    QPushButton *m_changeButton;
    QLineEdit *m_firstName;
    QLineEdit *m_lastName;
    QLineEdit *m_address;
    QLineEdit *m_email;
};
//! [0]

#endif // ADDRESSVIEW_H
