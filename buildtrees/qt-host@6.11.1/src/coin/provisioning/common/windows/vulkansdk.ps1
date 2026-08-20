# Copyright (C) 2021 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

. "$PSScriptRoot\helpers.ps1"

# This script will install Vulkan SDK
# Original Download page: https://vulkan.lunarg.com/sdk/home#windows

$cpu_arch = Get-CpuArchitecture
Write-Host "Installing $cpu_arch Vulkan SDK"
$version = "1.2.182.0"  # TODO: Update to newest 1.3.296.0
switch ($cpu_arch) {
    arm64 {
        $version = "1.3.296.0"
        $externalUrl = "https://sdk.lunarg.com/sdk/download/$version/warm/InstallVulkanARM64-$version.exe"
        $internalUrl = "\\ci-files01-hki.ci.qt.io\provisioning\windows\InstallVulkanARM64-$version.exe"
        $sha1 = "7d47d8dd10c09d363e6103925c4a032abf7b2c02"
        $installArgs = "--accept-licenses --default-answer --confirm-command install"
        Break
    }
    x64 {
        $externalUrl = "https://sdk.lunarg.com/sdk/download/$version/windows/VulkanSDK-$version-Installer.exe"
        $internalUrl = "\\ci-files01-hki.ci.qt.io\provisioning\windows\VulkanSDK-$version-Installer.exe"
        $sha1 = "1b662f338bfbfdd00fb9b0c09113eacb94f68a0e"
        $installArgs = "/S"
        Break
    }
    default {
        throw "Unknown architecture $cpu_arch"
    }
}

$vulkanPackage = "C:\Windows\Temp\vulkan-installer-$version.exe"

Download "$externalUrl" "$internalUrl" "$vulkanPackage"
Verify-Checksum "$vulkanPackage" "$sha1"

Run-Executable "$vulkanPackage" "$installArgs"

Write-Host "Cleaning $vulkanPackage.."
Remove "$vulkanPackage"

Write-Output "Vulkan SDK = $version" >> ~\versions.txt
