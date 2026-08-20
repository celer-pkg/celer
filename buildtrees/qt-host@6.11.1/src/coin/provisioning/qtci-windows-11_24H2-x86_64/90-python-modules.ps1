# Copyright (C) 2025 The Qt Company Ltd
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

. "$PSScriptRoot\..\common\windows\helpers.ps1"
# Needed by packaging scripts
$scriptsPath = [System.Environment]::GetEnvironmentVariable('PIP3_PATH', [System.EnvironmentVariableTarget]::Machine)
Run-Executable "$scriptsPath\pip3.exe" "install bs4"
