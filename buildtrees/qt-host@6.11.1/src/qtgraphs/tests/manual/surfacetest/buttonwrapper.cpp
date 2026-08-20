// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
#include "buttonwrapper.h"
#include <QPushButton>

ButtonWrapper::ButtonWrapper(QPushButton *button)
{
    m_button = button;
}

void ButtonWrapper::setEnabled(int state)
{
    m_button->setEnabled(state);
}
