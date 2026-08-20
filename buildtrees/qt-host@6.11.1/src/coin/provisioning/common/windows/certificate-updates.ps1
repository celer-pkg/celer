# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

# This script updates Windows Root Certifications which are usually updated weekly by Windows update

. "$PSScriptRoot\helpers.ps1"

$sstCerts = "C:\Windows\Temp\certificates.sst"
Run-Executable "certutil.exe" "-generateSSTFromWU $sstCerts"
$sstCertsPath = (Get-ChildItem -Path $sstCerts)
$sstCertsPath | Import-Certificate -CertStoreLocation "Cert:\LocalMachine\Root" | Out-String | Measure-Object -Line
Remove-Item -Path $sstCerts
