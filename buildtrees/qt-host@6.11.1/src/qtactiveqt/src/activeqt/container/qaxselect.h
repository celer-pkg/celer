// Copyright (C) 2015 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
// Qt-Security score:significant reason:default


#ifndef QAXSELECT_H
#define QAXSELECT_H
#include <QtWidgets/qdialog.h>

QT_BEGIN_NAMESPACE

class QAxSelectPrivate;
class QModelIndex;

class QAxSelect : public QDialog
{
    Q_OBJECT
public:
    enum SandboxingLevel {
        SandboxingNone = 0,
        SandboxingProcess,
        SandboxingLowIntegrity,
        SandboxingAppContainer,
    };

    explicit QAxSelect(QWidget *parent = nullptr, Qt::WindowFlags flags = Qt::WindowFlags());
    ~QAxSelect() override;

    QString clsid() const;
    SandboxingLevel sandboxingLevel() const;

private Q_SLOTS:
    void onActiveXListCurrentChanged(const QModelIndex &);
    void onActiveXListActivated();
    void onFilterLineEditChanged(const QString &);

private:
    QScopedPointer<QAxSelectPrivate> d;
};

QT_END_NAMESPACE

#endif // QAXSELECT_H
