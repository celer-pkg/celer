// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR
// GPL-2.0-only OR GPL-3.0-only

#include "build/build_config.h"
#include "gpu/vulkan/buildflags.h"
#include "ui/base/ozone_buildflags.h"

#include "ui/base/dragdrop/os_exchange_data_provider_factory.h"
#include "ui/base/pointer/pointer_device.h"
#include "ui/base/resource/resource_bundle.h"

#if BUILDFLAG(ENABLE_VULKAN)
#include "gpu/vulkan/init/vulkan_factory.h"
#endif

#if BUILDFLAG(IS_LINUX) && BUILDFLAG(IS_OZONE_X11)
void* GetQtXDisplay() {
  return nullptr;
}
#endif  // if BUILDFLAG(IS_LINUX) && BUILDFLAG(IS_OZONE_X11)

namespace ui {
class OzonePlatform;
OzonePlatform* CreateOzonePlatformQt() {
  return nullptr;
}
std::unique_ptr<OSExchangeDataProvider>
OSExchangeDataProviderFactory::CreateProvider() {
  return nullptr;
}
bool ResourceBundle::LocaleDataPakExists(std::string_view locale, Gender) {
  return false;
}
std::string ResourceBundle::LoadLocaleResources(const std::string& pref_locale,
                                                bool crash_on_failure) {
  return std::string();
}
void ResourceBundle::LoadCommonResources() {}
int GetAvailablePointerTypes() {
  return POINTER_TYPE_NONE;
}
int GetAvailableHoverTypes() {
  return HOVER_TYPE_NONE;
}
gfx::Image& ResourceBundle::GetNativeImageNamed(int resource_id) {
  return GetImageNamed(resource_id);
}

namespace gfx {
class ClientNativePixmapFactory;
}
gfx::ClientNativePixmapFactory* CreateClientNativePixmapFactoryQt() {
  return nullptr;
}
}  // namespace ui

#if BUILDFLAG(ENABLE_VULKAN)
namespace gpu {
std::unique_ptr<VulkanImplementation> CreateVulkanImplementation(
    bool use_swiftshader,
    bool allow_protected_memory) {
  return nullptr;
}
}
#endif  // BUILDFLAG(ENABLE_VULKAN)
