// Copyright (C) 2025 Jarek Kobus
// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "imagescaling.h"
#include "downloaddialog.h"
#include <QThreadFunction>
#include <QNetworkReplyWrapper>
#include <QTaskTree>

using namespace QtTaskTree;

Images::Images(QWidget *parent)
    : QWidget(parent)
    , cancelButton(new QPushButton(tr("Cancel")))
    , imagesLayout(new QGridLayout)
    , statusBar(new QStatusBar)
{
    resize(800, 600);

    auto addUrlsButton = new QPushButton(tr("Add URLs"));
    connect(addUrlsButton, &QPushButton::clicked, this, &Images::process);

    cancelButton->setEnabled(false);
    connect(cancelButton, &QPushButton::clicked, this, [this] { taskTreeRunner.cancel(); });

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addWidget(addUrlsButton);
    buttonLayout->addWidget(cancelButton);
    buttonLayout->addStretch();

    auto mainLayout = new QVBoxLayout;
    mainLayout->addLayout(buttonLayout);
    mainLayout->addLayout(imagesLayout);
    mainLayout->addStretch();
    mainLayout->addWidget(statusBar);
    setLayout(mainLayout);
}

//! [6]
static void scale(QPromise<QImage> &promise, const QByteArray &data)
{
    const auto image = QImage::fromData(data);
    if (image.isNull())
        promise.future().cancel();
    else
        promise.addResult(image.scaled(100, 100, Qt::KeepAspectRatio));
}
//! [6]

void Images::process()
{
//! [0]
    DownloadDialog dialog(this);
    if (dialog.exec() != QDialog::Accepted)
        return;

    const QList<QUrl> urls = dialog.getUrls();
    initLayout(urls.size());
//! [0]

//! [1]
    const ListIterator iterator(urls);
    const Storage<QByteArray> storage;
//! [1]

//! [3]
    const auto onRootSetup = [this] {
        statusBar->showMessage(tr("Downloading and Scaling..."));
        cancelButton->setEnabled(true);
    };
    const auto onRootDone = [this](DoneWith result) {
        const QString message = result == DoneWith::Cancel ? tr("Canceled.") : tr("Finished.");
        statusBar->showMessage(message);
        cancelButton->setEnabled(false);
    };
//! [3]

//! [4]
    const auto onDownloadSetup = [this, iterator](QNetworkReplyWrapper &task) {
        task.setNetworkAccessManager(&qnam);
        task.setRequest(QNetworkRequest(*iterator));
    };
    const auto onDownloadDone = [this, storage, iterator](const QNetworkReplyWrapper &task,
                                                          DoneWith result) {
        const int it = iterator.iteration();
        if (result == DoneWith::Success)
            *storage = task.reply()->readAll();
        else if (result == DoneWith::Error)
            labels[it]->setText(tr("Download\nError.\nCode: %1.").arg(task.reply()->error()));
        else
            labels[it]->setText(tr("Canceled."));
    };
//! [4]

//! [5]
    const auto onScaleSetup = [storage](QThreadFunction<QImage> &task) {
        task.setThreadFunctionData(&scale, *storage);
    };
    const auto onScaleDone = [this, iterator](const QThreadFunction<QImage> &task,
                                              DoneWith result) {
        const int it = iterator.iteration();
        if (result == DoneWith::Success)
            labels[it]->setPixmap(QPixmap::fromImage(task.result()));
        else if (result == DoneWith::Error)
            labels[it]->setText(tr("Image\nData\nError."));
        else
            labels[it]->setText(tr("Canceled."));
    };
//! [5]

//! [2]
    const Group recipe = For (iterator) >> Do {
        finishAllAndSuccess,
        parallel,
        onGroupSetup(onRootSetup),
        Group {
            storage,
            QNetworkReplyWrapperTask(onDownloadSetup, onDownloadDone),
            QThreadFunctionTask<QImage>(onScaleSetup, onScaleDone)
        },
        onGroupDone(onRootDone)
    };
    taskTreeRunner.start(recipe);
//! [2]
}

void Images::initLayout(qsizetype count)
{
    // Clean old images
    QLayoutItem *child;
    while ((child = imagesLayout->takeAt(0)) != nullptr) {
        child->widget()->setParent(nullptr);
        delete child->widget();
        delete child;
    }
    labels.clear();

    // Init the images layout for the new images
    const auto dim = int(qSqrt(qreal(count))) + 1;
    for (int i = 0; i < dim; ++i) {
        for (int j = 0; j < dim; ++j) {
            QLabel *imageLabel = new QLabel;
            imageLabel->setFixedSize(100, 100);
            imageLabel->setAlignment(Qt::AlignCenter);
            imagesLayout->addWidget(imageLabel, i, j);
            labels.append(imageLabel);
        }
    }
}
