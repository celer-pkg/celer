# Copyright (C) 2017 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

. "$PSScriptRoot\helpers.ps1"

# This script installs 7-Zip

$version = "24.09"
$nonDottedVersion = "2409"

$cpu_arch = Get-CpuArchitecture
switch ($cpu_arch) {
    arm64 {
        $arch = "-arm64"
        $sha1 = "2f5aaa22a4a591b01a1b06c17565233f0cd70429"
        Break
    }
    x64 {
        $arch = "-x64"
        $sha1 = "28b53835fe92c3fa6e0c422fc3b17c6bc1cb27e0"
        Break
    }
    x86 {
        $arch = ""
        $sha1 = "2135a90a9f6c3202c32a87b1c5cf805ce294a497"
        Break
    }
    default {
        throw "Unknown architecture $cpu_arch"
    }
}

$url_cache = "https://ci-files01-hki.ci.qt.io/input/windows/7z" + $nonDottedVersion + $arch + ".exe"
$url_official = "http://www.7-zip.org/a/7z" + $nonDottedVersion + $arch + ".exe"
$7zPackage = "C:\Windows\Temp\7zip-$nonDottedVersion.exe"
$7zTargetLocation = "C:\Utils\sevenzip\"

Download $url_official $url_cache $7zPackage
Verify-Checksum $7zPackage $sha1
Run-Executable $7zPackage "/S","/D=$7zTargetLocation"

Write-Host "Cleaning $7zPackage.."
Remove "$7zPackage"

Add-Path $7zTargetLocation

Write-Output "7-Zip = $version" >> ~\versions.txt
