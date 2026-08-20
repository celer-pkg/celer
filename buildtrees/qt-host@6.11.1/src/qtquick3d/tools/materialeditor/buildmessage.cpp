// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "buildmessage.h"

QT_BEGIN_NAMESPACE

ShaderBuildMessage::ShaderBuildMessage(const BuildMessage &data, const QString &filename, Stage stage)
    : m_message(data)
    , m_filename(filename)
    , m_stage(stage)
{

}

QT_END_NAMESPACE
