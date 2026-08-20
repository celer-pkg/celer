// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "mainwindow.h"
#include "encodingdialog.h"
#include "previewform.h"

#include <QAction>
#include <QApplication>
#include <QFileDialog>
#include <QMenuBar>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QRegularExpression>
#include <QScreen>
#include <QTextStream>

#include <QtCore5Compat/qtextcodec.h>

MainWindow::MainWindow()
{
    textEdit = new QPlainTextEdit;
    textEdit->setLineWrapMode(QPlainTextEdit::NoWrap);
    setCentralWidget(textEdit);

    findCodecs();

    previewForm = new PreviewForm(this);
    previewForm->setCodecList(codecs);

    createMenus();

    setWindowTitle(tr("Codecs"));

    const QRect screenGeometry = screen()->geometry();
    resize(screenGeometry.width() / 2, screenGeometry.height() * 2 / 3);
}

void MainWindow::open()
{
    const QString fileName = QFileDialog::getOpenFileName(this);
    if (fileName.isEmpty())
        return;
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly)) {
        QMessageBox::warning(this, tr("Codecs"),
                             tr("Cannot read file %1:\n%2")
                             .arg(QDir::toNativeSeparators(fileName),
                                  file.errorString()));
        return;
    }

    const QByteArray data = file.readAll();

    previewForm->setWindowTitle(tr("Choose Encoding for %1").arg(QFileInfo(fileName).fileName()));
    previewForm->setEncodedData(data);
    if (previewForm->exec())
        textEdit->setPlainText(previewForm->decodedString());
}

void MainWindow::save()
{
    const QAction *action = qobject_cast<const QAction *>(sender());
    const QByteArray codecName = action->data().toByteArray();
    const QString title = tr("Save As (%1)").arg(QLatin1String(codecName));

    QString fileName = QFileDialog::getSaveFileName(this, title);
    if (fileName.isEmpty())
        return;
    QFile file(fileName);
    if (!file.open(QFile::WriteOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("Codecs"),
                             tr("Cannot write file %1:\n%2")
                             .arg(QDir::toNativeSeparators(fileName),
                                  file.errorString()));
        return;
    }

    auto codec = QTextCodec::codecForName(codecName.constData());
    QByteArray text = codec->fromUnicode(textEdit->toPlainText());
    file.write(text);
}

void MainWindow::about()
{
    QMessageBox::about(this, tr("About Codecs"),
            tr("The <b>Codecs</b> example demonstrates how to read and write "
               "files using various encodings."));
}

void MainWindow::aboutToShowSaveAsMenu()
{
    const QString currentText = textEdit->toPlainText();
    for (QAction *action : std::as_const(saveAsActs)) {
        const QByteArray codecName = action->data().toByteArray();
        const auto codec = QTextCodec::codecForName(codecName);
        action->setVisible(codec && codec->canEncode(currentText));
    }
}

void MainWindow::findCodecs()
{
    QMap<QString, QTextCodec *> codecMap;
    QRegularExpression iso8859RegExp("^ISO[- ]8859-([0-9]+).*$");
    QRegularExpressionMatch match;

    const QList<int> mibs = QTextCodec::availableMibs();
    for (int mib : mibs) {
        auto codec = QTextCodec::codecForMib(mib);

        QString sortKey = codec->name().toUpper();
        char rank;

        if (sortKey.startsWith(QLatin1String("UTF-8"))) {
            rank = 1;
        } else if (sortKey.startsWith(QLatin1String("UTF-16"))) {
            rank = 2;
        } else if ((match = iso8859RegExp.match(sortKey)).hasMatch()) {
            if (match.capturedView(1).size() == 1)
                rank = 3;
            else
                rank = 4;
        } else {
            rank = 5;
        }
        sortKey.prepend(QLatin1Char('0' + rank));

        codecMap.insert(sortKey, codec);
    }
    for (const auto &codec : std::as_const(codecMap))
      codecs += codec;
}

void MainWindow::createMenus()
{
    QMenu *fileMenu = menuBar()->addMenu(tr("&File"));
    QAction *openAct =
        fileMenu->addAction(tr("&Open..."), this, &MainWindow::open);
    openAct->setShortcuts(QKeySequence::Open);

    QMenu *saveAsMenu = fileMenu->addMenu(tr("&Save As"));
    connect(saveAsMenu, &QMenu::aboutToShow,
            this, &MainWindow::aboutToShowSaveAsMenu);
    for (const auto codec : std::as_const(codecs)) {
        const QByteArray name = codec->name();
        QAction *action = saveAsMenu->addAction(tr("%1...").arg(QLatin1String(name)));
        action->setData(QVariant(name));
        connect(action, &QAction::triggered, this, &MainWindow::save);
        saveAsActs.append(action);
    }

    fileMenu->addSeparator();
    QAction *exitAct = fileMenu->addAction(tr("E&xit"), this, &QWidget::close);
    exitAct->setShortcuts(QKeySequence::Quit);

    auto toolMenu =  menuBar()->addMenu(tr("&Tools"));
    auto encodingAction = toolMenu->addAction(tr("Encodings"), this, &MainWindow::encodingDialog);
    encodingAction->setShortcut(Qt::CTRL + Qt::Key_E);
    encodingAction->setToolTip(tr("Shows a dialog allowing to convert to common encoding in programming languages."));


    menuBar()->addSeparator();

    QMenu *helpMenu = menuBar()->addMenu(tr("&Help"));
    helpMenu->addAction(tr("&About"), this, &MainWindow::about);
    helpMenu->addAction(tr("About &Qt"), qApp, &QApplication::aboutQt);
}

void MainWindow::encodingDialog()
{
    if (!m_encodingDialog) {
        m_encodingDialog = new EncodingDialog(this);
        const QRect screenGeometry = screen()->geometry();
        m_encodingDialog->setMinimumWidth(screenGeometry.width() / 4);
    }
    m_encodingDialog->show();
    m_encodingDialog->raise();

}
