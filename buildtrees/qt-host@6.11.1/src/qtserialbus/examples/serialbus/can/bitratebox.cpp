// Copyright (C) 2017 Andre Hartmann <aha_1980@gmx.de>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "bitratebox.h"

#include <QLineEdit>

BitRateBox::BitRateBox(QWidget *parent) :
    QComboBox(parent),
    m_customSpeedValidator(new QIntValidator(0, 1000000, this))
{
    fillBitRates();

    connect(this, &QComboBox::currentIndexChanged,
            this, &BitRateBox::checkCustomSpeedPolicy);
}

BitRateBox::~BitRateBox()
{
    delete m_customSpeedValidator;
}

int BitRateBox::bitRate() const
{
    if (currentIndex() == (count() - 1))
        return currentText().toInt();

    return itemData(currentIndex()).toInt();
}

bool BitRateBox::isFlexibleDataRateEnabled() const
{
    return m_isFlexibleDataRateEnabled;
}

void BitRateBox::setFlexibleDateRateEnabled(bool enabled)
{
    m_isFlexibleDataRateEnabled = enabled;
    m_customSpeedValidator->setTop(enabled ? 10000000 : 1000000);
    fillBitRates();
}

void BitRateBox::checkCustomSpeedPolicy(int idx)
{
    const bool isCustomSpeed = !itemData(idx).isValid();
    setEditable(isCustomSpeed);
    if (isCustomSpeed) {
        clearEditText();
        lineEdit()->setValidator(m_customSpeedValidator);
    }
}

void BitRateBox::fillBitRates()
{
    const QList<int> rates = {
        10000, 20000, 50000, 100000, 125000, 250000, 500000, 800000, 1000000
    };
    const QList<int> dataRates = {
        2000000, 4000000, 8000000
    };

    clear();

    for (int rate : rates)
        addItem(QString::number(rate), rate);

    if (isFlexibleDataRateEnabled()) {
        for (int rate : dataRates)
            addItem(QString::number(rate), rate);
    }

    addItem(tr("Custom"));
    setCurrentIndex(6); // default is 500000 bits/sec
}
