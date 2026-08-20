// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

//! [init]
#include "txtviewer.h"

#include <QFileDialog>
#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QPlainTextEdit>
#include <QScrollBar>
#include <QToolBar>

#include <QGuiApplication>
#include <QPainter>
#include <QTextDocument>

#include <QDir>

#ifdef DOCUMENTVIEWER_PRINTSUPPORT
#include <QPrinter>
#include <QPrintDialog>
#endif

using namespace Qt::StringLiterals;

TxtViewer::TxtViewer()
    : m_cutAct(new QAction(this)),
      m_copyAct(new QAction(this)),
      m_pasteAct(new QAction(this))
{
    connect(this, &AbstractViewer::uiInitialized, this, &TxtViewer::setupTxtUi);

    const QIcon cutIcon = QIcon::fromTheme(QIcon::ThemeIcon::EditCut,
                                           QIcon(":/demos/documentviewer/images/cut.png"_L1));
    m_cutAct->setIcon(cutIcon);
    m_cutAct->setShortcuts(QKeySequence::Cut);

    const QIcon copyIcon = QIcon::fromTheme(QIcon::ThemeIcon::EditCopy,
                                            QIcon(":/demos/documentviewer/images/copy.png"_L1));
    m_copyAct->setIcon(copyIcon);
    m_copyAct->setShortcuts(QKeySequence::Copy);

    const QIcon pasteIcon = QIcon::fromTheme(QIcon::ThemeIcon::EditPaste,
                                             QIcon(":/demos/documentviewer/images/paste.png"_L1));
    m_pasteAct->setIcon(pasteIcon);
    m_pasteAct->setShortcuts(QKeySequence::Paste);
}

TxtViewer::~TxtViewer() = default;

void TxtViewer::init(QFile *file, QWidget *parent, QMainWindow *mainWindow)
{
    AbstractViewer::init(file, new QPlainTextEdit(parent), mainWindow);
    m_textEdit = qobject_cast<QPlainTextEdit *>(widget());
    setTranslationBaseName("txtviewer"_L1);
}

QStringList TxtViewer::supportedMimeTypes() const
{
    return {"text/plain"_L1};
}

void TxtViewer::setupTxtUi()
{
    QMenu *editMenu = addMenu();
    QToolBar *editToolBar = addToolBar();
#if QT_CONFIG(clipboard)
    connect(m_cutAct, &QAction::triggered, m_textEdit, &QPlainTextEdit::cut);
    editMenu->addAction(m_cutAct);
    editToolBar->addAction(m_cutAct);

    connect(m_copyAct, &QAction::triggered, m_textEdit, &QPlainTextEdit::copy);
    editMenu->addAction(m_copyAct);
    editToolBar->addAction(m_copyAct);

    connect(m_pasteAct, &QAction::triggered, m_textEdit, &QPlainTextEdit::paste);
    editMenu->addAction(m_pasteAct);
    editToolBar->addAction(m_pasteAct);

    menuBar()->addSeparator();

    m_cutAct->setEnabled(false);
    m_copyAct->setEnabled(false);
    connect(m_textEdit, &QPlainTextEdit::copyAvailable, m_cutAct, &QAction::setEnabled);
    connect(m_textEdit, &QPlainTextEdit::copyAvailable, m_copyAct, &QAction::setEnabled);
#endif // QT_CONFIG(clipboard)

    openFile();

    connect(m_textEdit, &QPlainTextEdit::textChanged, this, [&](){
        maybeSetPrintingEnabled(hasContent());
    });

    connect(m_uiAssets.back, &QAction::triggered, m_textEdit, [&](){
        auto *bar = m_textEdit->verticalScrollBar();
        if (bar->value() > bar->minimum())
            bar->setValue(bar->value() - 1);
    });

    connect(m_uiAssets.forward, &QAction::triggered, m_textEdit, [&](){
        auto *bar = m_textEdit->verticalScrollBar();
        if (bar->value() < bar->maximum())
            bar->setValue(bar->value() + 1);
    });

    retranslate();
}
//! [init]

//! [open]
void TxtViewer::openFile()
{
    const QString type = tr("open");
    if (!m_file->open(QFile::ReadOnly | QFile::Text)) {
        statusMessage(tr("Cannot read file %1:\n%2.")
                      .arg(QDir::toNativeSeparators(m_file->fileName()),
                           m_file->errorString()), type);
        return;
    }

    QTextStream in(m_file.get());
#if QT_CONFIG(cursor)
    QGuiApplication::setOverrideCursor(Qt::WaitCursor);
#endif
    if (!m_textEdit->toPlainText().isEmpty()) {
        m_textEdit->clear();
        disablePrinting();
    }
    m_textEdit->setPlainText(in.readAll());
#if QT_CONFIG(cursor)
    QGuiApplication::restoreOverrideCursor();
#endif

    statusMessage(tr("File %1 loaded.")
                  .arg(QDir::toNativeSeparators(m_file->fileName())), type);
    maybeEnablePrinting();
}
//! [open]

//! [infoPrintAndSave]
bool TxtViewer::hasContent() const
{
    return (!m_textEdit->toPlainText().isEmpty());
}

#ifdef DOCUMENTVIEWER_PRINTSUPPORT
void TxtViewer::printDocument(QPrinter *printer) const
{
    if (!hasContent())
        return;

    m_textEdit->print(printer);
}
#endif // DOCUMENTVIEWER_PRINTSUPPORT

bool TxtViewer::saveFile(QFile *file)
{
    QString errorMessage;

    QGuiApplication::setOverrideCursor(Qt::WaitCursor);
    if (file->open(QFile::WriteOnly | QFile::Text)) {
        QTextStream out(file);
        out << m_textEdit->toPlainText();
    } else {
        errorMessage = tr("Cannot open file %1 for writing:\n%2.")
                       .arg(QDir::toNativeSeparators(file->fileName())),
                            file->errorString();
    }
    QGuiApplication::restoreOverrideCursor();

    if (!errorMessage.isEmpty()) {
        statusMessage(errorMessage);
        return false;
    }

    statusMessage(tr("File %1 saved")
                  .arg(QDir::toNativeSeparators(file->fileName())));
    return true;
}

bool TxtViewer::saveDocumentAs()
{
    QFileDialog dialog(mainWindow());
    dialog.setWindowModality(Qt::WindowModal);
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    if (dialog.exec() != QDialog::Accepted)
        return false;

    const QStringList &files = dialog.selectedFiles();
    if (files.isEmpty())
        return false;

    //newFile();
    m_file->setFileName(files.first());
    return saveDocument();
}

void TxtViewer::retranslate()
{
    if (toolBars().isEmpty())
        return;
    menus().at(0)->setTitle(tr("&Edit"));
    toolBars().at(0)->setWindowTitle(tr("Edit"));

    m_cutAct->setText(tr("Cu&t"));
    m_cutAct->setStatusTip(tr("Cut the current selection's contents to the clipboard"));
    m_copyAct->setText(tr("&Copy"));
    m_copyAct->setStatusTip(tr("Copy the current selection's contents to the clipboard"));
    m_pasteAct->setText(tr("&Paste"));
    m_pasteAct->setStatusTip(tr("Paste the clipboard's contents into the current selection"));
}
//! [infoPrintAndSave]
