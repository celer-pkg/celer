// Copyright 2019 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "services/network/cookie_access_delegate_impl.h"

#include <optional>
#include <set>

#include "base/containers/flat_map.h"
#include "base/containers/flat_set.h"
#include "base/functional/callback_forward.h"
#include "net/base/schemeful_site.h"
#include "net/cookies/cookie_constants.h"
#include "net/cookies/cookie_util.h"
#include "net/first_party_sets/first_party_set_metadata.h"
#include "net/first_party_sets/first_party_sets_cache_filter.h"
#include "services/network/cookie_manager.h"
#include "services/network/public/cpp/is_potentially_trustworthy.h"

namespace network {

CookieAccessDelegateImpl::CookieAccessDelegateImpl(
    mojom::CookieAccessDelegateType type,
    FirstPartySetsAccessDelegate* const first_party_sets_manager,
    const CookieSettings* cookie_settings,
    const CookieManager* cookie_manager)
    : type_(type),
      cookie_settings_(cookie_settings),
      first_party_sets_access_delegate_(first_party_sets_manager),
      cookie_manager_(cookie_manager) {
  if (type == mojom::CookieAccessDelegateType::USE_CONTENT_SETTINGS) {
    DCHECK(cookie_settings);
  }
}

CookieAccessDelegateImpl::~CookieAccessDelegateImpl() = default;

bool CookieAccessDelegateImpl::ShouldTreatUrlAsTrustworthy(
    const GURL& url) const {
  return IsUrlPotentiallyTrustworthy(url);
}

net::CookieAccessSemantics CookieAccessDelegateImpl::GetAccessSemantics(
    const net::CanonicalCookie& cookie) const {
  switch (type_) {
    case mojom::CookieAccessDelegateType::ALWAYS_LEGACY:
      return net::CookieAccessSemantics::LEGACY;
    case mojom::CookieAccessDelegateType::ALWAYS_NONLEGACY:
      return net::CookieAccessSemantics::NONLEGACY;
    case mojom::CookieAccessDelegateType::USE_CONTENT_SETTINGS:
      return cookie_settings_->GetCookieAccessSemanticsForDomain(
          cookie.Domain());
  }
}

net::CookieScopeSemantics CookieAccessDelegateImpl::GetScopeSemantics(
    const std::string_view domain) const {
  if (!cookie_settings_) {
    return net::CookieScopeSemantics::UNKNOWN;
  }
  // TODO(crbug.com/378827534)  finish propagating string_view thru cookie
  // settings.
  return cookie_settings_->GetCookieScopeSemanticsForDomain(
      std::string(domain));
}

bool CookieAccessDelegateImpl::ShouldIgnoreSameSiteRestrictions(
    const GURL& url,
    const net::SiteForCookies& site_for_cookies) const {
  if (cookie_settings_) {
    return cookie_settings_->ShouldIgnoreSameSiteRestrictions(url,
                                                              site_for_cookies);
  }
  return false;
}

void CookieAccessDelegateImpl::AllowedByFilter(
    const GURL& url,
    const net::SiteForCookies& site_for_cookies,
    base::OnceCallback<void(bool)> callback) const {
  if (cookie_manager_)
    cookie_manager_->AllowedByFilter(url, site_for_cookies, std::move(callback));
  else
    std::move(callback).Run(true);
}

std::optional<std::pair<net::FirstPartySetMetadata,
                        net::FirstPartySetsCacheFilter::MatchInfo>>
CookieAccessDelegateImpl::ComputeFirstPartySetMetadataMaybeAsync(
    const net::SchemefulSite& site,
    const net::SchemefulSite* top_frame_site,
    base::OnceCallback<void(net::FirstPartySetMetadata,
                            net::FirstPartySetsCacheFilter::MatchInfo)>
        callback) const {
  if (!first_party_sets_access_delegate_) {
    return std::make_pair(net::FirstPartySetMetadata(),
                          net::FirstPartySetsCacheFilter::MatchInfo());
  }
  return first_party_sets_access_delegate_->ComputeMetadata(
      site, top_frame_site, std::move(callback));
}

std::optional<FirstPartySetsAccessDelegate::EntriesResult>
CookieAccessDelegateImpl::FindFirstPartySetEntries(
    const base::flat_set<net::SchemefulSite>& sites,
    base::OnceCallback<void(FirstPartySetsAccessDelegate::EntriesResult)>
        callback) const {
  if (!first_party_sets_access_delegate_)
    return FirstPartySetsAccessDelegate::EntriesResult();
  return first_party_sets_access_delegate_->FindEntries(sites,
                                                        std::move(callback));
}

}  // namespace network
