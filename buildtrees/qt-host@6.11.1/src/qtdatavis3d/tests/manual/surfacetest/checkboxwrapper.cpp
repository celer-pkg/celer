// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
#include "checkboxwrapper.h"
#include <QCheckBox>

CheckBoxWrapper::CheckBoxWrapper(QCheckBox *cb)
{
    m_checkbox = cb;
}

void CheckBoxWrapper::setEnabled(int enabled)
{
    m_checkbox->setEnabled(enabled);
}
