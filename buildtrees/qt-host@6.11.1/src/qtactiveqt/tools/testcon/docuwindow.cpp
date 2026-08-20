// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "docuwindow.h"
#include <QTextBrowser>
#include <QTextDocument>
#include <QToolBar>
#include <QToolButton>
#include <QFileDialog>
#include <QFile>
#include <QStatusBar>
#include <QPrinter>
#include <QPainter>
#include <QPrintDialog>
#include <QTextStream>

QT_BEGIN_NAMESPACE

static const char *filesave[] = {
"    14    14        4            1",
". c #040404",
"# c #808304",
"a c #bfc2bf",
"b c None",
"..............",
".#.aaaaaaaa.a.",
".#.aaaaaaaa...",
".#.aaaaaaaa.#.",
".#.aaaaaaaa.#.",
".#.aaaaaaaa.#.",
".#.aaaaaaaa.#.",
".##........##.",
".############.",
".##.........#.",
".##......aa.#.",
".##......aa.#.",
".##......aa.#.",
"b............."
};

static const char *fileprint[] = {
"    16    14        6            1",
". c #000000",
"# c #848284",
"a c #c6c3c6",
"b c #ffff00",
"c c #ffffff",
"d c None",
"ddddd.........dd",
"dddd.cccccccc.dd",
"dddd.c.....c.ddd",
"ddd.cccccccc.ddd",
"ddd.c.....c....d",
"dd.cccccccc.a.a.",
"d..........a.a..",
".aaaaaaaaaa.a.a.",
".............aa.",
".aaaaaa###aa.a.d",
".aaaaaabbbaa...d",
".............a.d",
"d.aaaaaaaaa.a.dd",
"dd...........ddd"
};


DocuWindow::DocuWindow(const QString& docu, QWidget *parent)
    : QMainWindow(parent)
{
    browser = new QTextBrowser(this);
    browser->setHtml(docu);

    setCentralWidget(browser);

    QToolBar *fileTools = new QToolBar(tr("File Operations"), this);
    fileTools->addAction(QPixmap(filesave), tr("Save File"), this, SLOT(save()));
    fileTools->addAction(QPixmap(fileprint), tr("Print"), this, SLOT(print()));

    addToolBar(fileTools);
    statusBar();
}

void DocuWindow::save()
{
    QString filename = QFileDialog::getSaveFileName(this);

    if (filename.isEmpty())
        return;

    QString text = browser->document()->toHtml();
    QFile f(filename);
    if (!f.open(QIODevice::WriteOnly)) {
        statusBar()->showMessage(tr("Could not write to %1").arg(filename), 2000);
        return;
    }

    QTextStream t(&f);
    t << text;
    f.close();

    statusBar()->showMessage(tr("File %1 saved").arg(filename), 2000);
}

void DocuWindow::print()
{
#ifdef QT_NO_PRINTER
    Q_UNIMPLEMENTED();
#else
    QPrinter printer;
    if (printer.printerName().isEmpty()) {
        statusBar()->showMessage(tr("No printer installed"), 2000);
        return;
    }

    QPrintDialog printDialog(&printer, this);
    if (printDialog.exec() == QDialog::Rejected) {
        statusBar()->showMessage(tr("Printing aborted"), 2000);
        return;
    }

    browser->document()->print(&printer);
#endif
}

QT_END_NAMESPACE
