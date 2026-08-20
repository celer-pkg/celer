// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
#include "sliderwrapper.h"

SliderWrapper::SliderWrapper(QSlider *slider)
{
    m_slider = slider;
}

void SliderWrapper::setEnabled(int enabled)
{
    m_slider->setEnabled(enabled);
}
