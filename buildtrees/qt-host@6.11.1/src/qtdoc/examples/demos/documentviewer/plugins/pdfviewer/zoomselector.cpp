// Copyright (C) 2017 Klaralvdalens Datakonsult AB (KDAB).
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "zoomselector.h"

#include <QEvent>
#include <QLineEdit>

#include <array>

static constexpr std::array factors= { 12, 25, 33, 50, 66, 75, 100, 125, 150, 200, 400 };

ZoomSelector::ZoomSelector(QWidget *parent)
    : QComboBox(parent)
{
    setSizeAdjustPolicy(QComboBox::AdjustToContents);
    setEditable(true);

    // ZoomMode::FitToWidth, ZoomMode::FitInView + factors
    addItems(QStringList(2 + factors.size(), QString{}));

    retranslate();

    connect(this, &QComboBox::currentTextChanged,
            this, &ZoomSelector::onCurrentTextChanged);

    connect(lineEdit(), &QLineEdit::editingFinished,
            this, [this](){onCurrentTextChanged(lineEdit()->text()); });
}

void ZoomSelector::retranslate()
{
    int i = 0;
    setItemText(i++, tr("Fit Width"));
    setItemText(i++, tr("Fit Page"));
    const QString &percent = QLocale().percent();
    for (auto factor : factors)
        setItemText(i++, QString::number(factor) + percent);
}

void ZoomSelector::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
        retranslate();
    QComboBox::changeEvent(event);
}

void ZoomSelector::setZoomFactor(qreal zoomFactor)
{
    setCurrentText(QString::number(qRound(zoomFactor * 100)) + QLocale().percent());
}

void ZoomSelector::reset()
{
    setCurrentIndex(8); // 100%
}

void ZoomSelector::onCurrentTextChanged(const QString &text)
{
    if (text == itemText(0)) {
        emit zoomModeChanged(QPdfView::ZoomMode::FitToWidth);
    } else if (text == itemText(1)) {
        emit zoomModeChanged(QPdfView::ZoomMode::FitInView);
    } else {
        qreal factor = 1.0;

        QString withoutPercent(text);
        withoutPercent.remove(QLocale().percent());

        bool ok = false;
        const int zoomLevel = withoutPercent.toInt(&ok);
        if (ok)
            factor = zoomLevel / 100.0;

        emit zoomModeChanged(QPdfView::ZoomMode::Custom);
        emit zoomFactorChanged(factor);
    }
}
