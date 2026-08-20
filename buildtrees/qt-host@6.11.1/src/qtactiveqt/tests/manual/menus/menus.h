// Copyright (C) 2015 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef MENUS_H
#define MENUS_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
class QTextEdit;
QT_END_NAMESPACE

class QMenus : public QMainWindow
{
    Q_OBJECT
    Q_CLASSINFO("ClassID",     "{4dc3f340-a6f7-44e4-a79b-3e9217695fbd}")
    Q_CLASSINFO("InterfaceID", "{9ee49617-7d5c-441a-b833-4b068d40d751}")
    Q_CLASSINFO("EventsID",    "{13eca64b-ee2a-4f3c-aa04-5d9d975979a7}")

public:
    explicit QMenus(QWidget *parent = nullptr);

public slots:
    void fileOpen();
    void fileSave();

    void editNormal();
    void editBold();
    void editUnderline();

    void editAdvancedFont();
    void editAdvancedStyle();

    void helpAbout();
    void helpAboutQt();

private:
    QTextEdit *m_editor;
};

#endif // MENUS_H
