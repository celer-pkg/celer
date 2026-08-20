// Copyright 2012 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/common/url_schemes.h"

#include <string.h>

#include <iterator>
#include <utility>

#include "base/no_destructor.h"
#include "base/strings/string_util.h"
#include "build/build_config.h"
#include "content/public/common/content_client.h"
#include "content/public/common/content_features.h"
#include "content/public/common/url_constants.h"
#include "third_party/blink/public/common/scheme_registry.h"
#include "url/url_util.h"
#include "url/url_util_qt.h"

namespace content {
namespace {

bool g_registered_url_schemes = false;

const char* const kDefaultSavableSchemes[] = {
  url::kHttpScheme,
  url::kHttpsScheme,
  url::kFileScheme,
  url::kFileSystemScheme,
  kChromeDevToolsScheme,
  kChromeUIScheme,
  url::kDataScheme
};

// These lists are lazily initialized below and are leaked on shutdown to
// prevent any destructors from being called that will slow us down or cause
// problems.
std::vector<std::string>& GetMutableSavableSchemes() {
  static base::NoDestructor<std::vector<std::string>> schemes;
  return *schemes;
}

// This set contains serialized canonicalized origins as well as hostname
// patterns. The latter are canonicalized by component.
std::vector<std::string>& GetMutableServiceWorkerSchemes() {
  static base::NoDestructor<std::vector<std::string>> schemes;
  return *schemes;
}

}  // namespace

void RegisterContentSchemes(bool should_lock_registry) {
  // On Android and in tests, schemes may have been registered already.
  if (g_registered_url_schemes)
    return;
  g_registered_url_schemes = true;
  ContentClient::Schemes schemes;
  GetContentClient()->AddAdditionalSchemes(&schemes);

  url::AddStandardScheme(kChromeDevToolsScheme, url::SCHEME_WITH_HOST);
  url::AddStandardScheme(kChromeUIScheme, url::SCHEME_WITH_HOST);
  url::AddStandardScheme(kChromeUIUntrustedScheme, url::SCHEME_WITH_HOST);
  url::AddStandardScheme(kChromeErrorScheme, url::SCHEME_WITH_HOST);
  for (auto& scheme : schemes.standard_schemes)
    url::AddStandardScheme(scheme.c_str(), url::SCHEME_WITH_HOST);

  for (auto& scheme : schemes.referrer_schemes)
    url::AddReferrerScheme(scheme.c_str(), url::SCHEME_WITH_HOST);

  schemes.secure_schemes.push_back(kChromeDevToolsScheme);
  schemes.secure_schemes.push_back(kChromeUIScheme);
  schemes.secure_schemes.push_back(kChromeUIUntrustedScheme);
  schemes.secure_schemes.push_back(kChromeErrorScheme);
  for (auto& scheme : schemes.secure_schemes)
    url::AddSecureScheme(scheme.c_str());

  for (auto& scheme : schemes.local_schemes)
    url::AddLocalScheme(scheme.c_str());

  for (auto& scheme : schemes.extension_schemes)
    blink::CommonSchemeRegistry::RegisterURLSchemeAsExtension(scheme.c_str());

  for (auto& scheme : schemes.isolated_app_schemes) {
    blink::CommonSchemeRegistry::RegisterURLSchemeAsIsolatedApp(scheme.c_str());
  }

  schemes.no_access_schemes.push_back(kChromeErrorScheme);
  for (auto& scheme : schemes.no_access_schemes)
    url::AddNoAccessScheme(scheme.c_str());

  schemes.cors_enabled_schemes.push_back(kChromeUIScheme);
  schemes.cors_enabled_schemes.push_back(kChromeUIUntrustedScheme);
  for (auto& scheme : schemes.cors_enabled_schemes)
    url::AddCorsEnabledScheme(scheme.c_str());

  // TODO(mkwst): Investigate whether chrome-error should be included in
  // csp_bypassing_schemes.
  for (auto& scheme : schemes.csp_bypassing_schemes)
    url::AddCSPBypassingScheme(scheme.c_str());

  for (auto& scheme : schemes.empty_document_schemes)
    url::AddEmptyDocumentScheme(scheme.c_str());

#if BUILDFLAG(IS_ANDROID) || BUILDFLAG(IS_QTWEBENGINE)
  if (schemes.allow_non_standard_schemes_in_origins)
    url::EnableNonStandardSchemesForAndroidWebView();
#endif

  for (auto& [scheme, handler] : schemes.predefined_handler_schemes)
    url::AddPredefinedHandlerScheme(scheme.c_str(), handler.c_str());

  // This should only be registered if the
  // kEnableServiceWorkerForChrome or
  // kEnableServiceWorkerForChromeUntrusted feature is enabled but checking it
  // here causes a crash when --no-sandbox is enabled. See crbug.com/1313812
  // There are other render side checks and browser side checks that ensure
  // service workers don't work for chrome[-untrusted]:// when the flag is not
  // enabled.
  schemes.service_worker_schemes.push_back(kChromeUIScheme);
  schemes.service_worker_schemes.push_back(kChromeUIUntrustedScheme);

  // NOTE(juvaldma)(Chromium 67.0.3396.47)
  //
  // Since ContentClient::Schemes::standard_types doesn't have types
  // (url::SchemeType), we need to bypass AddAdditionalSchemes and add our
  // 'standard custom schemes' directly. Although the other scheme lists could
  // be filled also in AddAdditionalSchemes by QtWebEngineCore, to follow the
  // principle of the separation of concerns, we add them here instead. This
  // way, from the perspective of QtWebEngineCore, everything to do with custom
  // scheme parsing is fully encapsulated behind url::CustomScheme. The
  // complexity of QtWebEngineCore is reduced while the complexity of
  // url::CustomScheme is not significantly increased (since the functionality
  // is needed anyway).
  for (auto& cs : url::CustomScheme::GetSchemes()) {
    if (cs.type != url::SCHEME_WITHOUT_AUTHORITY) {
      url::AddStandardScheme(cs.name.c_str(), cs.type);
    }
    if (cs.flags & url::CustomScheme::Secure) {
      url::AddSecureScheme(cs.name.c_str());
    }
    if (cs.flags & url::CustomScheme::Local) {
      url::AddLocalScheme(cs.name.c_str());
    }
    if (cs.flags & url::CustomScheme::NoAccessAllowed) {
      url::AddNoAccessScheme(cs.name.c_str());
    }
    if (cs.flags & url::CustomScheme::ContentSecurityPolicyIgnored) {
      url::AddCSPBypassingScheme(cs.name.c_str());
    }
    if (cs.flags & url::CustomScheme::CorsEnabled) {
      url::AddCorsEnabledScheme(cs.name.c_str());
    }
  }

  // Prevent future modification of the scheme lists. This is to prevent
  // accidental creation of data races in the program. Add*Scheme aren't
  // threadsafe so must be called when GURL isn't used on any other thread. This
  // is really easy to mess up, so we say that all calls to Add*Scheme in Chrome
  // must be inside this function.
  if (should_lock_registry)
    url::LockSchemeRegistries();

  // Combine the default savable schemes with the additional ones given.
  GetMutableSavableSchemes().assign(std::begin(kDefaultSavableSchemes),
                                    std::end(kDefaultSavableSchemes));
  GetMutableSavableSchemes().insert(GetMutableSavableSchemes().end(),
                                    schemes.savable_schemes.begin(),
                                    schemes.savable_schemes.end());

  GetMutableServiceWorkerSchemes() = std::move(schemes.service_worker_schemes);

  // NOTE(juvaldma)(Chromium 67.0.3396.47)
  //
  // This list only applies to Chromium proper whereas Blink uses it's own
  // hardcoded list (see blink::URLSchemesRegistry).
  for (auto& cs : url::CustomScheme::GetSchemes()) {
    if (cs.flags & url::CustomScheme::ServiceWorkersAllowed) {
      GetMutableServiceWorkerSchemes().push_back(cs.name);
    }
  }
}

void ReRegisterContentSchemesForTests() {
  url::ClearSchemesForTests();
  g_registered_url_schemes = false;
  RegisterContentSchemes();
}

const std::vector<std::string>& GetSavableSchemes() {
  return GetMutableSavableSchemes();
}

const std::vector<std::string>& GetServiceWorkerSchemes() {
  return GetMutableServiceWorkerSchemes();
}

}  // namespace content
