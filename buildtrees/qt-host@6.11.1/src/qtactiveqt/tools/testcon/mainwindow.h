// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtAxContainer/QAxSelect>
#include <QList>

#include "ui_mainwindow.h"

QT_BEGIN_NAMESPACE

class InvokeMethod;
class ChangeProperties;
class AmbientProperties;
class QAxScriptManager;
class QAxWidget;
class QMdiArea;

QT_END_NAMESPACE

QT_USE_NAMESPACE


class MainWindow : public QMainWindow, public Ui::MainWindow
{
    Q_OBJECT

    Q_CLASSINFO("ClassID", "{5f5ce700-48a8-47b1-9b06-3b7f79e41d7c}")
    Q_CLASSINFO("InterfaceID", "{3fc86f5f-8b15-4428-8f6b-482bae91f1ae}")
    Q_CLASSINFO("EventsID", "{02a268cd-24b4-4fd9-88ff-b01b683ef39d}")

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    static MainWindow *instance() { return m_instance; }

    bool addControlFromClsid(const QString &clsid, QAxSelect::SandboxingLevel sandboxing);
    bool addControlFromFile(const QString &fileName);
    bool loadScript(const QString &file);

protected:
    void closeEvent(QCloseEvent *) override;

public slots:
    void appendLogText(const QString &);

protected slots:
    void on_actionFileNew_triggered();
    void on_actionFileLoad_triggered();
    void on_actionFileSave_triggered();

    void on_actionContainerSet_triggered();
    void on_actionContainerClear_triggered();
    void on_actionContainerProperties_triggered();

    void on_actionControlInfo_triggered();
    void on_actionControlDocumentation_triggered();
    void on_actionControlPixmap_triggered();
    void on_actionControlProperties_triggered();
    void on_actionControlMethods_triggered();
    void on_VerbMenu_aboutToShow();

    void on_actionScriptingLoad_triggered();
    void on_actionScriptingRun_triggered();

    void on_actionFreeUnusedDLLs_triggered();
    void on_actionAbout_Qt_triggered();
    void on_actionAbout_Testcon_triggered();

private:
    QAxWidget *activeAxWidget() const;
    QList<QAxWidget *> axWidgets() const;

    static MainWindow *m_instance;

    InvokeMethod *m_dlgInvoke = nullptr;
    ChangeProperties *m_dlgProperties = nullptr;
    AmbientProperties *m_dlgAmbient = nullptr;
    QAxScriptManager *m_scripts = nullptr;
    QMdiArea *m_mdiArea = nullptr;

private slots:
    void updateGUI();
    void logPropertyChanged(const QString &prop);
    void logSignal(const QString &signal, int argc, void *argv);
    void logException(int code, const QString&source, const QString&desc, const QString&help);
    void logMacro(int code, const QString &description, int sourcePosition, const QString &sourceText);

    void on_VerbMenu_triggered(QAction *action);
};

#endif // MAINWINDOW_H
