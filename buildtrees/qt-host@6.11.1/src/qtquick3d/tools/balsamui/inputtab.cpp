// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "inputtab.h"

#include "inputlistview.h"
#include "settingstab.h"

#include <QFileDialog>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QtQuick3DAssetImport/private/qssgassetimportmanager_p.h>

InputTab::InputTab(SettingsTab *settings, QWidget *parent) : QWidget(parent), settingsTab(settings)
{
    QLabel *fileNameLabel = new QLabel(tr("Input files:"));
    lastInputPath = QDir::currentPath();
    inputFilesListBox = new InputListView;

    convertButton = new QPushButton("Convert", this);
    connect(convertButton, &QAbstractButton::clicked, this, [&]() { convert(); });

    statusText = new QPlainTextEdit;
    statusText->setReadOnly(true);

    QObject::connect(&converterThread, &ConverterThread::convertDone, this, &InputTab::convertDone);
    QObject::connect(&converterThread, &ConverterThread::convertStart, this, &InputTab::convertStart);
    QObject::connect(&converterThread, &ConverterThread::convertUpdate, this, &InputTab::convertUpdate);

    QPushButton *browseInputButton = new QPushButton(tr("Browse..."), this);
    connect(browseInputButton, &QAbstractButton::clicked, this, &InputTab::browseInput);

    QPushButton *removeSelectedButton = new QPushButton(tr("Remove selected"), this);
    connect(removeSelectedButton, &QAbstractButton::clicked, this, &InputTab::removeSelected);
    QPushButton *selectAllButton = new QPushButton(tr("Select all"), this);
    connect(selectAllButton, &QAbstractButton::clicked, this, &InputTab::selectAll);

    QLabel *pathLabel = new QLabel(tr("Output folder:"));
    outputPathEdit = new QLineEdit(QDir::currentPath());
    QPushButton *browseOutputButton = new QPushButton(tr("Browse..."), this);
    connect(browseOutputButton, &QAbstractButton::clicked, this, &InputTab::browseOutput);

    QLabel *emptyLabel = new QLabel();

    QGridLayout *mainLayout = new QGridLayout;

    constexpr int rowFileNameLabel = 0;
    constexpr int rowinputFiles = 1;
    constexpr int rowOutputPathLabel = 5;
    constexpr int rowOutputPathEdit = 6;
    constexpr int rowStatusLabel = 7;

    mainLayout->addWidget(fileNameLabel, rowFileNameLabel, 0, 1, 2);
    mainLayout->addWidget(inputFilesListBox, rowinputFiles, 0, 4, 1);
    mainLayout->addWidget(browseInputButton, rowinputFiles, 1);
    mainLayout->addWidget(removeSelectedButton, rowinputFiles + 1, 1);
    mainLayout->addWidget(selectAllButton, rowinputFiles + 2, 1);
    mainLayout->addWidget(emptyLabel, rowinputFiles + 3, 1);
    mainLayout->setRowStretch(rowinputFiles + 3, 1);

    mainLayout->addWidget(pathLabel, rowOutputPathLabel, 0, 1, 2);

    mainLayout->addWidget(outputPathEdit, rowOutputPathEdit, 0);
    mainLayout->addWidget(browseOutputButton, rowOutputPathEdit, 1);

    mainLayout->addWidget(new QLabel("Status:"), rowStatusLabel, 0, 1, 2);
    mainLayout->addWidget(statusText, rowStatusLabel + 1, 0, 1, 2);
    mainLayout->addWidget(convertButton, rowStatusLabel + 2, 0, 1, 2);

    setLayout(mainLayout);
}

void InputTab::convert()
{
    QStringList filenames = getInputFiles();
    QDir outputPath = getOutputPath();

    const auto options = settingsTab->getOptions();

    converterThread.convert(filenames, outputPath, options);
}

void InputTab::convertStart(const QString &text)
{
    statusText->setPlainText(text);
    convertButton->setText(QString("Converting..."));
    convertButton->setDisabled(true);
}

void InputTab::convertUpdate(const QString &text)
{
    statusText->appendPlainText(text);
}

void InputTab::convertDone(const QString &text)
{
    statusText->appendPlainText(text);
    convertButton->setText(QString("Convert"));
    convertButton->setDisabled(false);
}

QStringList InputTab::getInputFiles() const
{
    QStringList files;
    for (int i = 0; i < inputFilesListBox->count(); ++i)
        files.push_back(inputFilesListBox->item(i)->text());
    return files;
}

QString InputTab::getOutputPath() const
{
    return outputPathEdit->text();
}

void InputTab::browseInput()
{
    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::ExistingFiles);
    dialog.setDirectory(lastInputPath);

    const auto allDescs = QSSGAssetImportManager().getImporterPluginInfos();

    QStringList nameFilters;
    for (const auto &plugin : allDescs) {
        nameFilters.append(tr("%1 files (*.%2)").arg(plugin.typeDescription, plugin.inputExtensions.join(QStringLiteral(" *."))));
    }
    nameFilters.append(tr("Any files (*)"));
    dialog.setNameFilters(nameFilters);

    if (!dialog.exec())
        return;

    const auto selectedFiles = dialog.selectedFiles();
    for (const auto &file : selectedFiles)
        inputFilesListBox->tryAddItem(QDir::toNativeSeparators(file));

    if (!selectedFiles.empty())
        lastInputPath = QFileInfo(selectedFiles.first()).absoluteDir().absolutePath();
}

void InputTab::browseOutput()
{
    QString dir = QFileDialog::getExistingDirectory(this,
                                                    tr("Open Directory"),
                                                    outputPathEdit->text(),
                                                    QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (!dir.isEmpty()) {
        outputPathEdit->setText(dir);
    }
}

void InputTab::removeSelected()
{
    QList<QListWidgetItem *> selectedItems = inputFilesListBox->selectedItems();
    for (int i = 0; i < inputFilesListBox->count(); ++i) {
        auto item = inputFilesListBox->item(i);
        if (selectedItems.contains(item)) {
            inputFilesListBox->takeItem(i);
            selectedItems.removeAll(item);
            i--;
        }
    }
}

void InputTab::selectAll()
{
    for (int i = 0; i < inputFilesListBox->count(); ++i) {
        auto item = inputFilesListBox->item(i);
        item->setSelected(true);
    }
}
