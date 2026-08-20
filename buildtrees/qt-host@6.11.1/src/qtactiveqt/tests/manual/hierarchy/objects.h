// Copyright (C) 2015 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef OBJECTS_H
#define OBJECTS_H

#include <QWidget>

QT_BEGIN_NAMESPACE
class QVBoxLayout;
QT_END_NAMESPACE
class QSubWidget;

//! [0]
class QParentWidget : public QWidget
{
    Q_OBJECT
    Q_CLASSINFO("ClassID", "{d574a747-8016-46db-a07c-b2b4854ee75c}");
    Q_CLASSINFO("InterfaceID", "{4a30719d-d9c2-4659-9d16-67378209f822}");
    Q_CLASSINFO("EventsID", "{4a30719d-d9c2-4659-9d16-67378209f823}");
public:
    explicit QParentWidget(QWidget *parent = nullptr);

    QSize sizeHint() const override;

public slots:
    void createSubWidget(const QString &name);

    QSubWidget *subWidget(const QString &name);

private:
    QVBoxLayout *m_vbox;
};
//! [0]

//! [1]
class QSubWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QString label READ label WRITE setLabel)

    Q_CLASSINFO("ClassID", "{850652f4-8f71-4f69-b745-bce241ccdc30}");
    Q_CLASSINFO("InterfaceID", "{2d76cc2f-3488-417a-83d6-debff88b3c3f}");
    Q_CLASSINFO("ToSuperClass", "QSubWidget");

public:
    QSubWidget(QWidget *parent = nullptr, const QString &name = QString());

    void setLabel(const QString &text);
    QString label() const;

    QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent *e) override;

private:
    QString m_label;
};
//! [1]

#endif // OBJECTS_H
