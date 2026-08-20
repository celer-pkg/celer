// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "ieframe.h" // generated header

#include <metaobjectdump.h>
#include <textdialog.h>

#include <QAction>
#include <QApplication>
#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QToolBar>
#include <QLibraryInfo>

#include <QScreen>

#include <QDebug>

class MainWindow :public QMainWindow
{
    Q_OBJECT
public:
    MainWindow();

public slots:
    void navigate(const QString &);
    void showMetaObject();

private:
    SHDocVw::WebBrowser *m_browser;
};

MainWindow::MainWindow() : m_browser(new SHDocVw::WebBrowser)
{
    setCentralWidget(m_browser);

    auto fileMenu = menuBar()->addMenu("File");
    auto toolbar = new QToolBar;
    addToolBar(Qt::TopToolBarArea, toolbar);

    auto action = fileMenu->addAction("Dump MetaObject...",
                                      this, &MainWindow::showMetaObject);
    action->setShortcut(Qt::CTRL + Qt::Key_D);
    toolbar->addAction(action);

    action = fileMenu->addAction("Quit", this, &QWidget::close);
    action->setShortcut(Qt::CTRL + Qt::Key_Q);
    toolbar->addAction(action);
}

void  MainWindow::navigate(const QString &url)
{
    m_browser->Navigate(url);
}

void MainWindow::showMetaObject()
{
    auto mo = m_browser->metaObject();
    QString dump;
    {
        QTextStream str(&dump);
        str << *mo;
    }
    auto dialog = new TextDialog(dump, this);
    dialog->setWindowTitle(QLatin1String("MetaObject of ") + QLatin1String(mo->className()));
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->resize(screen()->geometry().size() * 2 / 3);
    dialog->show();
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    MainWindow w;
    w.setWindowTitle(qVersion());
    w.show();
    w.navigate("https://qt.io/");

    return a.exec();
}

#include "main.moc"
