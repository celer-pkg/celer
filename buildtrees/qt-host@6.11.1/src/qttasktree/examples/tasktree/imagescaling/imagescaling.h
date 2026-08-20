// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef IMAGESCALING_H
#define IMAGESCALING_H

#include <QNetworkAccessManager>
#include <QSingleTaskTreeRunner>
#include <QtWidgets>

class Images : public QWidget
{
    Q_OBJECT

public:
    explicit Images(QWidget *parent = nullptr);

private:
    void process();
    void initLayout(qsizetype count);

    QPushButton *cancelButton;
    QGridLayout *imagesLayout;
    QStatusBar *statusBar;
    QList<QLabel *> labels;

    QNetworkAccessManager qnam;
    QtTaskTree::QSingleTaskTreeRunner taskTreeRunner;
};

#endif // IMAGESCALING_H
