// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "takenumber.h"

TakeNumber::TakeNumber(QObject *parent)
    : QObject{parent}
{}

void TakeNumber::takeInt(int a)
{
    takenInt = a;
}

void TakeNumber::takeNegativeInt(int a)
{
    takenNegativeInt = a;
}

void TakeNumber::takeQSizeType(qsizetype a)
{
    takenQSizeType = a;
}

void TakeNumber::takeQLongLong(qlonglong a)
{
    takenQLongLong = a;
}

void TakeNumber::takeNumbers(const Numbers &a)
{
    takenNumbers = a;
}
