// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "exportdialog.h"

#include <QApplication>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QScreen>
#include <QSpinBox>
#include <QToolButton>
#include <QVBoxLayout>

#include <QImageWriter>

#include <QDebug>

#include <QDir>
#include <QFileInfo>

enum { exportMinimumSize = 1, exportMaximumSize = 2000 };

ExportDialog::ExportDialog(QWidget *parent)
    : QDialog(parent)
    , m_fileNameLineEdit(new QLineEdit(this))
    , m_widthSpinBox(new QSpinBox(this))
    , m_heightSpinBox(new QSpinBox(this))
    , m_aspectRatio(1)
{
    typedef void (QSpinBox::*QSpinBoxIntSignal)(int);

    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setWindowTitle(tr("Export"));

    QFormLayout *formLayout = new QFormLayout(this);

    QHBoxLayout *fileLayout = new QHBoxLayout;
    fileLayout->addWidget(m_fileNameLineEdit);
    m_fileNameLineEdit->setMinimumWidth(this->screen()->availableGeometry() .width() / 6);
    QPushButton *browseButton = new QPushButton(tr("Browse..."), this);
    fileLayout->addWidget(browseButton);
    connect(browseButton, &QAbstractButton::clicked, this, &ExportDialog::browse);
    formLayout->addRow(tr("File:"), fileLayout);

    QHBoxLayout *sizeLayout = new QHBoxLayout;
    sizeLayout->addStretch();
    m_widthSpinBox->setMinimum(exportMinimumSize);
    m_widthSpinBox->setMaximum(exportMaximumSize);
    connect(m_widthSpinBox, static_cast<QSpinBoxIntSignal>(&QSpinBox::valueChanged),
            this, &ExportDialog::exportWidthChanged);
    sizeLayout->addWidget(m_widthSpinBox);
    //: Multiplication, as in 32x32
    sizeLayout->addWidget(new QLabel(tr("x")));
    m_heightSpinBox->setMinimum(exportMinimumSize);
    m_heightSpinBox->setMaximum(exportMaximumSize);
    connect(m_heightSpinBox, static_cast<QSpinBoxIntSignal>(&QSpinBox::valueChanged),
            this, &ExportDialog::exportHeightChanged);
    sizeLayout->addWidget(m_heightSpinBox);
    QToolButton *resetButton = new QToolButton(this);
    resetButton->setIcon(QIcon(":/qt-project.org/styles/commonstyle/images/refresh-32.png"));
    sizeLayout->addWidget(resetButton);
    connect(resetButton, &QAbstractButton::clicked, this, &ExportDialog::resetExportSize);
    formLayout->addRow(tr("Size:"), sizeLayout);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    formLayout->addRow(buttonBox);
}

void ExportDialog::accept()
{
    const QString fileName = exportFileName();
    if (fileName.isEmpty()) {
        QMessageBox::warning(this, windowTitle(), tr("Please enter a file name"));
        return;
    }
    QFileInfo fi(fileName);
    if (fi.exists()) {
        const QString question = tr("%1 already exists.\nWould you like to overwrite it?").arg(QDir::toNativeSeparators(fileName));
        if (QMessageBox::question(this, windowTitle(), question, QMessageBox::Yes | QMessageBox::No) !=  QMessageBox::Yes)
            return;
    }
    QDialog::accept();
}

QSize ExportDialog::exportSize() const
{
    return QSize(m_widthSpinBox->value(), m_heightSpinBox->value());
}

void ExportDialog::setExportSize(const QSize &size)
{
    m_defaultSize = size;
    QSizeF defaultSizeF(m_defaultSize);
    m_aspectRatio = defaultSizeF.width() / defaultSizeF.height();
    setExportWidthBlocked(size.width());
    setExportHeightBlocked(size.height());
}

void ExportDialog::resetExportSize()
{
    setExportWidthBlocked(m_defaultSize.width());
    setExportHeightBlocked(m_defaultSize.height());
}

void ExportDialog::setExportWidthBlocked(int width)
{
    if (m_widthSpinBox->value() != width) {
        const bool blockSignals = m_widthSpinBox->blockSignals(true);
        m_widthSpinBox->setValue(width);
        m_widthSpinBox->blockSignals(blockSignals);
    }
}

void ExportDialog::setExportHeightBlocked(int height)
{
    if (m_heightSpinBox->value() != height) {
        const bool blockSignals = m_heightSpinBox->blockSignals(true);
        m_heightSpinBox->setValue(height);
        m_heightSpinBox->blockSignals(blockSignals);
    }
}

void ExportDialog::exportWidthChanged(int width)
{
    const bool square = m_defaultSize.width() == m_defaultSize.height();
    setExportHeightBlocked(square ? width : qRound(qreal(width) / m_aspectRatio));
}

void ExportDialog::exportHeightChanged(int height)
{
    const bool square = m_defaultSize.width() == m_defaultSize.height();
    setExportWidthBlocked(square ? height : qRound(qreal(height) * m_aspectRatio));
}

QString ExportDialog::exportFileName() const
{
    return QDir::cleanPath(m_fileNameLineEdit->text().trimmed());
}

void ExportDialog::setExportFileName(const QString &f)
{
    m_fileNameLineEdit->setText(QDir::toNativeSeparators(f));
}

void ExportDialog::browse()
{
    QFileDialog fileDialog(this);
    fileDialog.setAcceptMode(QFileDialog::AcceptSave);
    const QString fileName = exportFileName();
    if (!fileName.isEmpty())
        fileDialog.setDirectory(QFileInfo(fileName).absolutePath());
    QStringList mimeTypes;
    const auto supportedMimeTypes = QImageWriter::supportedMimeTypes();
    for (const QByteArray &mimeType : supportedMimeTypes)
        mimeTypes.append(QLatin1String(mimeType));
    fileDialog.setMimeTypeFilters(mimeTypes);
    const int pngIndex = mimeTypes.indexOf("image/png");
    if (pngIndex >= 0) {
        fileDialog.selectMimeTypeFilter(mimeTypes.at(pngIndex));
        fileDialog.setDefaultSuffix("png");
    }
    if (fileDialog.exec() == QDialog::Accepted)
        setExportFileName(fileDialog.selectedFiles().constFirst());
}
