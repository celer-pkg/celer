// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: BSD-3-Clause

#include <wrl.h>
#include <windows.devices.enumeration.h>
#include <windows.devices.bluetooth.h>

#if defined(_WIN32) && defined(__INTEL_COMPILER)
#error "Windows ICC fails to build the WinRT backend (QTBUG-68026)."
#endif

int main()
{
    Microsoft::WRL::ComPtr<ABI::Windows::Devices::Enumeration::IDeviceInformationStatics> deviceInformationStatics;
    ABI::Windows::Foundation::GetActivationFactory(Microsoft::WRL::Wrappers::HString::MakeReference
        (RuntimeClass_Windows_Devices_Enumeration_DeviceInformation).Get(), &deviceInformationStatics);

    (void)Microsoft::WRL::ComPtr<ABI::Windows::Devices::Bluetooth::GenericAttributeProfile::IGattDeviceService3>().Get();
    (void)Microsoft::WRL::ComPtr<ABI::Windows::Devices::Bluetooth::IBluetoothDevice>().Get();
    (void)Microsoft::WRL::ComPtr<ABI::Windows::Devices::Bluetooth::IBluetoothLEDevice>().Get();
    return 0;
}
