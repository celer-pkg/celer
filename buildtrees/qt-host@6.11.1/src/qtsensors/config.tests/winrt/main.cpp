// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: BSD-3-Clause

#include <functional>
#include <windows.system.h>

#include <windows.devices.sensors.h>
#include <windows.foundation.h>
#include <wrl.h>

using namespace Microsoft::WRL;
using namespace Microsoft::WRL::Wrappers;
using namespace ABI::Windows::Foundation;
using namespace ABI::Windows::Devices::Sensors;

typedef ITypedEventHandler<Accelerometer *, AccelerometerReadingChangedEventArgs *> AccelerometerReadingHandler;

int main(int, char**)
{
    HStringReference classId(RuntimeClass_Windows_Devices_Sensors_Accelerometer);
    ComPtr<IAccelerometer> sensor;
    ComPtr<IAccelerometerStatics> factory;
    HRESULT hr = RoGetActivationFactory(classId.Get(), IID_PPV_ARGS(&factory));
    hr = factory->GetDefault(&sensor);
    return 0;
}
