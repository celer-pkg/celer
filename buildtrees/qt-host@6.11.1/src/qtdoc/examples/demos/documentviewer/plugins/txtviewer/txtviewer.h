// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef TXTVIEWER_H
#define TXTVIEWER_H

#include "viewerinterfaces.h"

QT_BEGIN_NAMESPACE
class QPlainTextEdit;
QT_END_NAMESPACE

//! [interfacing]
class TxtViewer : public ViewerInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.Examples.DocumentViewer.ViewerInterface" FILE "txtviewer.json")
    Q_INTERFACES(ViewerInterface)
//! [interfacing]
//! [classDefinition]
public:
    TxtViewer();
    ~TxtViewer() override;
    void init(QFile *file, QWidget *parent, QMainWindow *mainWindow) override;
    QString viewerName() const override { return QLatin1StringView(staticMetaObject.className()); };
    QStringList supportedMimeTypes() const override;
    bool saveDocument() override { return saveFile(m_file.get()); };
    bool saveDocumentAs() override;
    bool hasContent() const override;
    QByteArray saveState() const override { return {}; }
    bool restoreState(QByteArray &) override { return true; }
    bool supportsOverview() const override { return false; }

#ifdef DOCUMENTVIEWER_PRINTSUPPORT
protected:
    void printDocument(QPrinter *printer) const override;
#endif // DOCUMENTVIEWER_PRINTSUPPORT

private slots:
    void setupTxtUi();

private:
    void retranslate() override;
    void openFile();
    bool saveFile (QFile *file);

    QPlainTextEdit *m_textEdit;
    QAction *m_cutAct = nullptr;
    QAction *m_copyAct = nullptr;
    QAction *m_pasteAct = nullptr;
};
//! [classDefinition]

#endif //TXTVIEWER_H
