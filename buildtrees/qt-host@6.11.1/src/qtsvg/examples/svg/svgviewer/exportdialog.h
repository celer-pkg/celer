// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef EXPORTDIALOG_H
#define EXPORTDIALOG_H

#include <QDialog>

QT_BEGIN_NAMESPACE
class QLineEdit;
class QSpinBox;
QT_END_NAMESPACE

class ExportDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ExportDialog(QWidget *parent = 0);

    QSize exportSize() const;
    void setExportSize(const QSize &);

    QString exportFileName() const;
    void setExportFileName(const QString &);

    void accept() override;

private slots:
    void browse();
    void resetExportSize();
    void exportWidthChanged(int width);
    void exportHeightChanged(int height);

private:
    void setExportWidthBlocked(int width);
    void setExportHeightBlocked(int height);

    QLineEdit *m_fileNameLineEdit;
    QSpinBox *m_widthSpinBox;
    QSpinBox *m_heightSpinBox;
    QSize m_defaultSize;
    qreal m_aspectRatio;
};

#endif // EXPORTDIALOG_H
