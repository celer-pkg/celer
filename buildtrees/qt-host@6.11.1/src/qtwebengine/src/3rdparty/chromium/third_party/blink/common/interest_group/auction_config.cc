// Copyright 2022 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "third_party/blink/public/common/interest_group/auction_config.h"

#include <cmath>
#include <string_view>
#include <tuple>

#include "base/strings/to_string.h"
#include "third_party/blink/public/common/features.h"
#include "third_party/blink/public/common/interest_group/ad_display_size_utils.h"
#include "third_party/blink/public/mojom/interest_group/ad_auction_service.mojom.h"

namespace blink {

DirectFromSellerSignalsSubresource::DirectFromSellerSignalsSubresource() =
    default;
DirectFromSellerSignalsSubresource::DirectFromSellerSignalsSubresource(
    const DirectFromSellerSignalsSubresource&) = default;
DirectFromSellerSignalsSubresource::DirectFromSellerSignalsSubresource(
    DirectFromSellerSignalsSubresource&&) = default;
DirectFromSellerSignalsSubresource::~DirectFromSellerSignalsSubresource() =
    default;

DirectFromSellerSignalsSubresource&
DirectFromSellerSignalsSubresource::operator=(
    const DirectFromSellerSignalsSubresource&) = default;
DirectFromSellerSignalsSubresource&
DirectFromSellerSignalsSubresource::operator=(
    DirectFromSellerSignalsSubresource&&) = default;
bool operator==(const DirectFromSellerSignalsSubresource& a,
                const DirectFromSellerSignalsSubresource& b) {
  return std::tie(a.bundle_url, a.token) == std::tie(b.bundle_url, b.token);
}


DirectFromSellerSignals::DirectFromSellerSignals() = default;
DirectFromSellerSignals::DirectFromSellerSignals(
    const DirectFromSellerSignals&) = default;
DirectFromSellerSignals::DirectFromSellerSignals(DirectFromSellerSignals&&) =
    default;
DirectFromSellerSignals::~DirectFromSellerSignals() = default;

DirectFromSellerSignals& DirectFromSellerSignals::operator=(
    const DirectFromSellerSignals&) = default;
DirectFromSellerSignals& DirectFromSellerSignals::operator=(
    DirectFromSellerSignals&&) = default;
bool operator==(const DirectFromSellerSignals& a,
                const DirectFromSellerSignals& b) {
  return std::tie(a.prefix, a.per_buyer_signals, a.seller_signals,
                  a.auction_signals) == std::tie(b.prefix, b.per_buyer_signals,
                                                 b.seller_signals,
                                                 b.auction_signals);
}

bool operator==(const AuctionConfig::BuyerTimeouts& a,
                const AuctionConfig::BuyerTimeouts& b) {
  return std::tie(a.all_buyers_timeout, a.per_buyer_timeouts) ==
         std::tie(b.all_buyers_timeout, b.per_buyer_timeouts);
}

bool operator==(const AuctionConfig::BuyerCurrencies& a,
                const AuctionConfig::BuyerCurrencies& b) {
  return std::tie(a.all_buyers_currency, a.per_buyer_currencies) ==
         std::tie(b.all_buyers_currency, b.per_buyer_currencies);
}

AuctionConfig::NonSharedParams::NonSharedParams() = default;
AuctionConfig::NonSharedParams::NonSharedParams(const NonSharedParams&) =
    default;
AuctionConfig::NonSharedParams::NonSharedParams(NonSharedParams&&) = default;
AuctionConfig::NonSharedParams::~NonSharedParams() = default;

AuctionConfig::NonSharedParams& AuctionConfig::NonSharedParams::operator=(
    const NonSharedParams&) = default;
AuctionConfig::NonSharedParams& AuctionConfig::NonSharedParams::operator=(
    NonSharedParams&&) = default;
bool operator==(const AuctionConfig::NonSharedParams& a,
                const AuctionConfig::NonSharedParams& b) {
  return std::tie(a.interest_group_buyers, a.auction_signals, a.seller_signals,
                  a.seller_timeout, a.per_buyer_signals, a.buyer_timeouts,
                  a.buyer_cumulative_timeouts, a.seller_currency,
                  a.buyer_currencies, a.per_buyer_group_limits,
                  a.all_buyers_group_limit, a.per_buyer_priority_signals,
                  a.all_buyers_priority_signals, a.auction_report_buyer_keys,
                  a.auction_report_buyers,
                  a.auction_report_buyer_debug_mode_config, a.requested_size,
                  a.all_slots_requested_sizes, a.required_seller_capabilities,
                  a.auction_nonce, a.component_auctions,
                  a.deprecated_render_url_replacements,
                  a.max_trusted_scoring_signals_url_length) ==
         std::tie(b.interest_group_buyers, b.auction_signals, b.seller_signals,
                  b.seller_timeout, b.per_buyer_signals, b.buyer_timeouts,
                  b.buyer_cumulative_timeouts, b.seller_currency,
                  b.buyer_currencies, b.per_buyer_group_limits,
                  b.all_buyers_group_limit, b.per_buyer_priority_signals,
                  b.all_buyers_priority_signals, b.auction_report_buyer_keys,
                  b.auction_report_buyers,
                  b.auction_report_buyer_debug_mode_config, b.requested_size,
                  b.all_slots_requested_sizes, b.required_seller_capabilities,
                  b.auction_nonce, b.component_auctions,
                  b.deprecated_render_url_replacements,
                  b.max_trusted_scoring_signals_url_length);
}

bool operator==(
    const AuctionConfig::NonSharedParams::AuctionReportBuyersConfig& a,
    const AuctionConfig::NonSharedParams::AuctionReportBuyersConfig& b) {
  return std::tie(a.bucket, a.scale) == std::tie(b.bucket, b.scale);
}

bool operator==(
    const AuctionConfig::NonSharedParams::AuctionReportBuyerDebugModeConfig& a,
    const AuctionConfig::NonSharedParams::AuctionReportBuyerDebugModeConfig&
        b) {
  return std::tie(a.is_enabled, a.debug_key) ==
         std::tie(b.is_enabled, b.debug_key);
}

AuctionConfig::ServerResponseConfig::ServerResponseConfig() = default;
AuctionConfig::ServerResponseConfig::ServerResponseConfig(
    const ServerResponseConfig& other) = default;
AuctionConfig::ServerResponseConfig::ServerResponseConfig(
    ServerResponseConfig&&) = default;
AuctionConfig::ServerResponseConfig::~ServerResponseConfig() = default;

AuctionConfig::ServerResponseConfig&
AuctionConfig::ServerResponseConfig::operator=(
    const ServerResponseConfig& other) = default;

AuctionConfig::ServerResponseConfig&
AuctionConfig::ServerResponseConfig::operator=(ServerResponseConfig&&) =
    default;

bool operator==(const AuctionConfig::ServerResponseConfig& a,
                const AuctionConfig::ServerResponseConfig& b) {
  return a.request_id == b.request_id
         && a.got_response == b.got_response;
}

AuctionConfig::AuctionConfig() = default;
AuctionConfig::AuctionConfig(const AuctionConfig&) = default;
AuctionConfig::AuctionConfig(AuctionConfig&&) = default;
AuctionConfig::~AuctionConfig() = default;

AuctionConfig& AuctionConfig::operator=(const AuctionConfig&) = default;
AuctionConfig& AuctionConfig::operator=(AuctionConfig&&) = default;

bool operator==(const AuctionConfig& a, const AuctionConfig& b) {
  return std::tie(a.seller, a.decision_logic_url, a.trusted_scoring_signals_url,
                  a.non_shared_params,
                  a.direct_from_seller_signals,
                  a.expects_direct_from_seller_signals_header_ad_slot,
                  a.seller_experiment_group_id, a.all_buyer_experiment_group_id,
                  a.per_buyer_experiment_group_ids,
                  a.expects_additional_bids) ==
         std::tie(b.seller, b.decision_logic_url, b.trusted_scoring_signals_url,
                  b.non_shared_params,
                  b.direct_from_seller_signals,
                  b.expects_direct_from_seller_signals_header_ad_slot,
                  b.seller_experiment_group_id, b.all_buyer_experiment_group_id,
                  b.per_buyer_experiment_group_ids, b.expects_additional_bids);
}

int AuctionConfig::NumPromises() const {
  int total = 0;
  if (non_shared_params.auction_signals.is_promise()) {
    ++total;
  }
  if (non_shared_params.seller_signals.is_promise()) {
    ++total;
  }
  if (non_shared_params.seller_tkv_signals.is_promise()) {
    ++total;
  }
  if (non_shared_params.per_buyer_signals.is_promise()) {
    ++total;
  }
  if (non_shared_params.buyer_timeouts.is_promise()) {
    ++total;
  }
  if (non_shared_params.buyer_currencies.is_promise()) {
    ++total;
  }
  if (non_shared_params.buyer_cumulative_timeouts.is_promise()) {
    ++total;
  }
  if (non_shared_params.deprecated_render_url_replacements.is_promise()) {
    ++total;
  }
  for (const auto& buyer_tkv_signals :
       non_shared_params.per_buyer_tkv_signals) {
    if (buyer_tkv_signals.second.is_promise()) {
      ++total;
    }
  }

  if (direct_from_seller_signals.is_promise()) {
    ++total;
  }
  if (expects_direct_from_seller_signals_header_ad_slot) {
    ++total;
  }
  if (expects_additional_bids) {
    ++total;
  }

  for (const blink::AuctionConfig& sub_auction :
       non_shared_params.component_auctions) {
    total += sub_auction.NumPromises();
  }
  return total;
}

bool AuctionConfig::IsHttpsAndMatchesSellerOrigin(const GURL& url) const {
  return url.scheme() == url::kHttpsScheme &&
         url::Origin::Create(url) == seller;
}

bool AuctionConfig::IsValidTrustedScoringSignalsURL(const GURL& url) const {
  if (url.has_query() || url.has_ref() || url.has_username() ||
      url.has_password()) {
    return false;
  }

  return url.scheme() == url::kHttpsScheme;
}

bool AuctionConfig::IsDirectFromSellerSignalsValid(
    const std::optional<blink::DirectFromSellerSignals>&
        candidate_direct_from_seller_signals) const {
  if (!candidate_direct_from_seller_signals.has_value()) {
    return true;
  }

  const GURL& prefix = candidate_direct_from_seller_signals->prefix;
  // The prefix can't have a query because the browser process appends its own
  // query suffix.
  if (prefix.has_query()) {
    return false;
  }
  // NOTE: uuid-in-package isn't supported, since it doesn't support CORS.
  if (!IsHttpsAndMatchesSellerOrigin(prefix)) {
    return false;
  }

  base::flat_set<url::Origin> interest_group_buyers(
      non_shared_params.interest_group_buyers
          ? *non_shared_params.interest_group_buyers
          : std::vector<url::Origin>());
  for (const auto& [buyer_origin, bundle_url] :
       candidate_direct_from_seller_signals->per_buyer_signals) {
    // The renderer shouldn't provide bundles for origins that aren't buyers
    // in this auction -- there would be no worklet to receive them.
    if (interest_group_buyers.count(buyer_origin) < 1) {
      return false;
    }
    // All DirectFromSellerSignals must come from the seller.
    if (!IsHttpsAndMatchesSellerOrigin(bundle_url.bundle_url)) {
      return false;
    }
  }
  if (candidate_direct_from_seller_signals->seller_signals &&
      !IsHttpsAndMatchesSellerOrigin(
          candidate_direct_from_seller_signals->seller_signals->bundle_url)) {
    // All DirectFromSellerSignals must come from the seller.
    return false;
  }
  if (candidate_direct_from_seller_signals->auction_signals &&
      !IsHttpsAndMatchesSellerOrigin(
          candidate_direct_from_seller_signals->auction_signals->bundle_url)) {
    // All DirectFromSellerSignals must come from the seller.
    return false;
  }
  return true;
}

}  // namespace blink
