// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "mainwindow.h"

#include <QUrl>

#include <QDesktopServices>
#include <QScreen>

#include <QLabel>
#include <QLayout>

MainWindow::MainWindow(QWidget *parent)
    : QWidget(parent),
      m_titleLabel(new QLabel(this)),
      m_infoLabel(new QLabel(tr("Enable NFC"), this)),
      m_uriLabel(new QLabel(this)),
      m_landscapeIconLabel(new QLabel(this)),
      m_portraitIconLabel(new QLabel(this))
{
    m_titleLabel->setAlignment(Qt::AlignCenter);
    m_infoLabel->setAlignment(Qt::AlignCenter);
    m_uriLabel->setAlignment(Qt::AlignCenter);
    m_landscapeIconLabel->setAlignment(Qt::AlignCenter);
    m_portraitIconLabel->setAlignment(Qt::AlignCenter);

    QFont f = m_titleLabel->font();
    f.setBold(true);
    m_titleLabel->setFont(f);

    QVBoxLayout *verticalLayout = new QVBoxLayout;
    verticalLayout->addWidget(m_titleLabel);
    verticalLayout->addWidget(m_infoLabel);
    verticalLayout->addWidget(m_portraitIconLabel);
    verticalLayout->addWidget(m_uriLabel);

    QHBoxLayout *horizontalLayout = new QHBoxLayout;
    horizontalLayout->addWidget(m_landscapeIconLabel);
    horizontalLayout->addLayout(verticalLayout);
    horizontalLayout->setSpacing(0);

    setLayout(horizontalLayout);

    handleScreenChange();
}

MainWindow::~MainWindow()
{
}

void MainWindow::displayAnnotatedUrl(const QUrl &url, const QString &title, const QPixmap &pixmap)
{
    m_infoLabel->setHidden(true);

    m_uriLabel->setText(url.toString());
    m_titleLabel->setText(title);

    m_pixmap = pixmap;
    updateWidgetLayout(m_screen->orientation());
}

void MainWindow::nfcStateChanged(bool enabled)
{
    const QString text = enabled ? "Touch a tag" : "Enable NFC";
    m_infoLabel->setText(text);
}

void MainWindow::showTagError(const QString &message)
{
    m_uriLabel->clear();
    m_titleLabel->clear();
    m_pixmap = QPixmap();
    m_landscapeIconLabel->setVisible(false);
    m_portraitIconLabel->setVisible(false);

    m_infoLabel->setHidden(false);
    m_infoLabel->setText(message);
}

void MainWindow::mouseReleaseEvent(QMouseEvent * /*event*/)
{
    QDesktopServices::openUrl(QUrl(m_uriLabel->text()));
}

void MainWindow::moveEvent(QMoveEvent * /*event*/)
{
    if (screen() != m_screen)
        handleScreenChange();
}

void MainWindow::updateWidgetLayout(Qt::ScreenOrientation orientation)
{
    m_landscapeIconLabel->setVisible(false);
    m_portraitIconLabel->setVisible(false);
    if (!m_pixmap.isNull()) {
        const bool landscapeMode = (orientation == Qt::LandscapeOrientation)
                || (orientation == Qt::InvertedLandscapeOrientation);
        QLabel *imageLabel = landscapeMode ? m_landscapeIconLabel : m_portraitIconLabel;
        imageLabel->setVisible(true);
        if (m_pixmap.width() > imageLabel->width() || m_pixmap.height() > imageLabel->height())
            imageLabel->setPixmap(m_pixmap.scaled(imageLabel->size(), Qt::KeepAspectRatio));
        else
            imageLabel->setPixmap(m_pixmap);
    }
}

void MainWindow::handleScreenChange()
{
    if (m_screen)
        disconnect(m_screen, &QScreen::orientationChanged, this, &MainWindow::updateWidgetLayout);

    m_screen = screen();
    connect(m_screen, &QScreen::orientationChanged, this, &MainWindow::updateWidgetLayout);

    updateWidgetLayout(m_screen->orientation());
}
