// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

// based on //chrome/browser/extensions/api/tabs/tabs_api.cc
// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "qtwebengine/browser/extensions/api/tabs/tabs_api.h"
#include "qtwebengine/common/extensions/api/tabs.h"

#include "base/containers/contains.h"
#include "base/metrics/histogram_macros.h"
#include "chrome/common/url_constants.h"
#include "components/guest_view/browser/guest_view_base.h"
#include "components/url_formatter/url_fixer.h"
#include "content/public/browser/navigation_controller.h"
#include "content/public/browser/navigation_entry.h"
#include "content/public/browser/render_frame_host.h"
#include "content/public/browser/render_widget_host_view.h"
#include "content/public/browser/web_contents.h"
#include "content/public/common/url_constants.h"
#include "extensions/common/error_utils.h"
#include "extensions/common/permissions/permissions_data.h"

#include <optional>
#include <string>
#include <utility>

using content::NavigationController;
using content::OpenURLParams;
using content::WebContents;

namespace extensions {

namespace tabs = api::tabs;

static GURL ResolvePossiblyRelativeURL(const std::string& url_string,
                                       const Extension* extension) {
  GURL url = GURL(url_string);
  if (!url.is_valid() && extension)
    url = extension->GetResourceURL(url_string);

  return url;
}

static bool IsKillURL(const GURL& url) {
#if DCHECK_IS_ON()
  // Caller should ensure that |url| is already "fixed up" by
  // url_formatter::FixupURL, which (among many other things) takes care
  // of rewriting about:kill into chrome://kill/.
  if (url.SchemeIs(url::kAboutScheme))
    DCHECK(url.IsAboutBlank() || url.IsAboutSrcdoc());
#endif

  static const char* const kill_hosts[] = {
      chrome::kChromeUICrashHost,         chrome::kChromeUIDelayedHangUIHost,
      chrome::kChromeUIHangUIHost,        chrome::kChromeUIKillHost,
      chrome::kChromeUIQuitHost,          chrome::kChromeUIRestartHost,
      content::kChromeUIBrowserCrashHost, content::kChromeUIMemoryExhaustHost,
  };

  if (!url.SchemeIs(content::kChromeUIScheme))
    return false;

  return base::Contains(kill_hosts, url.host_piece());
}

static bool PrepareURLForNavigation(const std::string& url_string,
                                    const Extension* extension,
                                    GURL* return_url,
                                    std::string* error) {
  GURL url = ResolvePossiblyRelativeURL(url_string, extension);

  // Ideally, the URL would only be "fixed" for user input (e.g. for URLs
  // entered into the Omnibox), but some extensions rely on the legacy behavior
  // where all navigations were subject to the "fixing".  See also
  // https://crbug.com/1145381.
  url = url_formatter::FixupURL(url.spec(), "" /* = desired_tld */);

  // Reject invalid URLs.
  if (!url.is_valid()) {
    *error = ErrorUtils::FormatErrorMessage("Invalid url: \"*\".", url_string);
    return false;
  }

  // Don't let the extension crash the browser or renderers.
  if (IsKillURL(url)) {
    *error = "I'm sorry. I'm afraid I can't do that.";
    return false;
  }

  if (url.SchemeIs(content::kChromeDevToolsScheme)) {
    *error = "Cannot navigate to a devtools:// page without either the devtools or "
             "debugger permission.";
    return false;
  }

  return_url->Swap(&url);
  return true;
}

TabsUpdateFunction::TabsUpdateFunction() : web_contents_(nullptr) {}

ExtensionFunction::ResponseAction TabsUpdateFunction::Run() {
  std::optional<tabs::Update::Params> params =
      tabs::Update::Params::Create(args());
  EXTENSION_FUNCTION_VALIDATE(params);

  int tab_id = -1;
  std::string error;

  web_contents_ = GetSenderWebContents();
  if (!web_contents_) {
    return RespondNow(Error("The specified target is not found."));
  } else {
    web_contents_ = guest_view::GuestViewBase::GetTopLevelWebContents(web_contents_);
  }

  // Navigate the tab to a new location if the url is different.
  if (params->update_properties.url) {
    std::string updated_url = *params->update_properties.url;
    if (!UpdateURL(updated_url, tab_id, &error))
      return RespondNow(Error(std::move(error)));
  }

  return RespondNow(GetResult());
}

bool TabsUpdateFunction::UpdateURL(const std::string& url_string,
                                   int tab_id,
                                   std::string* error) {
  GURL url;
  if (!PrepareURLForNavigation(url_string, extension(), &url,
                                                 error)) {
    return false;
  }

  const bool is_javascript_scheme = url.SchemeIs(url::kJavaScriptScheme);
  UMA_HISTOGRAM_BOOLEAN("Extensions.ApiTabUpdateJavascript",
                        is_javascript_scheme);
  // JavaScript URLs are forbidden in chrome.tabs.update().
  if (is_javascript_scheme) {
    *error = "JavaScript URLs are not allowed in chrome.tabs.update. Use "
             "chrome.tabs.executeScript instead.";
    return false;
  }

  NavigationController::LoadURLParams load_params(url);

  // Treat extension-initiated navigations as renderer-initiated so that the URL
  // does not show in the omnibox until it commits.  This avoids URL spoofs
  // since URLs can be opened on behalf of untrusted content.
  load_params.is_renderer_initiated = true;
  // All renderer-initiated navigations need to have an initiator origin.
  load_params.initiator_origin = extension()->origin();
  // |source_site_instance| needs to be set so that a renderer process
  // compatible with |initiator_origin| is picked by Site Isolation.
  load_params.source_site_instance = content::SiteInstance::CreateForURL(
      web_contents_->GetBrowserContext(),
      load_params.initiator_origin->GetURL());

  // Marking the navigation as initiated via an API means that the focus
  // will stay in the omnibox - see https://crbug.com/1085779.
  load_params.transition_type = ui::PAGE_TRANSITION_FROM_API;

  web_contents_->GetController().LoadURLWithParams(load_params);

  DCHECK_EQ(url,
            web_contents_->GetController().GetPendingEntry()->GetVirtualURL());

  return true;
}

ExtensionFunction::ResponseValue TabsUpdateFunction::GetResult() {
  return NoArguments();
}

void TabsUpdateFunction::OnExecuteCodeFinished(
    const std::string& error,
    const GURL& url,
    const base::Value::List& script_result) {
  if (!error.empty()) {
    Respond(Error(error));
    return;
  }

  return Respond(GetResult());
}

}  // namespace extensions
