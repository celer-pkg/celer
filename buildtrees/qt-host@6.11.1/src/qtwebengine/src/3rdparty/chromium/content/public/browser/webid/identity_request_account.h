// Copyright 2022 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_PUBLIC_BROWSER_WEBID_IDENTITY_REQUEST_ACCOUNT_H_
#define CONTENT_PUBLIC_BROWSER_WEBID_IDENTITY_REQUEST_ACCOUNT_H_

#include <optional>
#include <string>
#include <vector>

#include "base/memory/ref_counted.h"
#include "base/time/time.h"
#include "content/common/content_export.h"
#include "third_party/blink/public/common/webid/login_status_account.h"
#include "third_party/blink/public/mojom/webid/federated_auth_request.mojom-forward.h"
#include "third_party/skia/include/core/SkColor.h"
#include "ui/gfx/image/image.h"
#include "url/gurl.h"

namespace content {

// A Java counterpart will be generated for this enum.
// GENERATED_JAVA_ENUM_PACKAGE: org.chromium.content.webid
// GENERATED_JAVA_CLASS_NAME_OVERRIDE: IdentityRequestDialogDisclosureField
enum class IdentityRequestDialogDisclosureField : int32_t {
  kName,
  kEmail,
  kPicture,
  kPhoneNumber,
  kUsername
};

// The client metadata that will be used to display a FedCM dialog. This data is
// extracted from the client metadata endpoint from the FedCM API, where
// 'client' is essentially the relying party which invoked the API.
struct CONTENT_EXPORT ClientMetadata {
  ClientMetadata(const GURL& terms_of_service_url,
                 const GURL& privacy_policy_url,
                 const GURL& brand_icon_url,
                 const gfx::Image& brand_decoded_icon);
  ClientMetadata(const ClientMetadata& other);
  ~ClientMetadata();

  GURL terms_of_service_url;
  GURL privacy_policy_url;
  GURL brand_icon_url;
  // This will be an empty image if the fetching never happened or if it failed.
  gfx::Image brand_decoded_icon;
};

// The information about an error that will be used to display a FedCM dialog.
// This data is extracted from the error object returned by the identity
// provider when the user attempts to login via the FedCM API and an error
// occurs.
struct CONTENT_EXPORT IdentityCredentialTokenError {
  std::string code;
  GURL url;
};

// The metadata about the identity provider that will be used to display a FedCM
// dialog. This data is extracted from the config file which is fetched when the
// FedCM API is invoked.
struct CONTENT_EXPORT IdentityProviderMetadata {
  IdentityProviderMetadata();
  IdentityProviderMetadata(const IdentityProviderMetadata& other);
  ~IdentityProviderMetadata();

  std::optional<SkColor> brand_text_color;
  std::optional<SkColor> brand_background_color;
  GURL brand_icon_url;
  GURL idp_login_url;
  std::string requested_label;
  // For registered IdPs, the type is used to only show the accounts when the
  // RP is compatible.
  std::vector<std::string> types;
  // The token formats that are supported.
  std::vector<std::string> formats;
  // The URL of the configuration endpoint. This is stored in
  // IdentityProviderMetadata so that the UI code can pass it along when an
  // Account is selected by the user.
  GURL config_url;
  // Whether this IdP supports signing in to additional accounts.
  bool supports_add_account{false};
  // Whether this IdP has any filtered out account. This is reset to false each
  // time the accounts dialog is shown and recomputed then.
  bool has_filtered_out_account{false};
  // This will be an empty image if fetching failed.
  gfx::Image brand_decoded_icon;
};

// This class contains all of the data specific to an identity provider that is
// going to be used to display a FedCM dialog. This data is gathered from
// endpoints fetched when the FedCM API is invoked as well as from the
// parameters provided by the relying party when the API is invoked.
class CONTENT_EXPORT IdentityProviderData
    : public base::RefCounted<IdentityProviderData> {
 public:
  IdentityProviderData(const std::string& idp_for_display,
                       const IdentityProviderMetadata& idp_metadata,
                       const ClientMetadata& client_metadata,
                       blink::mojom::RpContext rp_context,
                       std::optional<blink::mojom::Format> format,
                       const std::vector<IdentityRequestDialogDisclosureField>&
                           disclosure_fields,
                       bool has_login_status_mismatch);

  std::string idp_for_display;
  IdentityProviderMetadata idp_metadata;
  ClientMetadata client_metadata;
  blink::mojom::RpContext rp_context;
  std::optional<blink::mojom::Format> format;
  // For which fields should the dialog request permission for (assuming
  // this is for signup).
  std::vector<IdentityRequestDialogDisclosureField> disclosure_fields;
  // Whether there was some login status API mismatch when fetching the IDP's
  // accounts.
  bool has_login_status_mismatch;

 private:
  friend class base::RefCounted<IdentityProviderData>;

  ~IdentityProviderData();
};

// Represents a federated user account which is used when displaying the FedCM
// account selector.
class CONTENT_EXPORT IdentityRequestAccount
    : public base::RefCounted<IdentityRequestAccount> {
 public:
  enum class LoginState {
    // This is a returning user signing in with RP/IDP in this browser.
    kSignIn,
    // This is a new user sign up for RP/IDP in *this browser*. Note that this
    // is the browser's notion of login state which may not match that of the
    // IDP. For example the user may actually be a returning user having
    // previously signed-up with this RP/IDP outside this browser. This is a
    // consequence of not relying the IDP's login state. This means that we
    // should be mindful to *NOT* rely on this value to mean definitely a new
    // user when using it to customize the UI.
    kSignUp,
  };

  enum class SignInMode {
    // This is the default sign in mode for returning users.
    kExplicit,
    // This represents the auto re-authn flow. Currently it's only available
    // when RP specifies |autoReauthn = true| AND there is only one signed in
    // account.
    kAuto,
  };

  IdentityRequestAccount(
      const std::string& id,
      const std::string& display_identifier,
      const std::string& display_name,
      const std::string& email,
      const std::string& name,
      const std::string& given_name,
      const GURL& picture,
      const std::string& phone,
      const std::string& username,
      std::vector<std::string> login_hints,
      std::vector<std::string> domain_hints,
      std::vector<std::string> labels,
      std::optional<LoginState> idp_claimed_login_state = std::nullopt,
      LoginState browser_trusted_login_state = LoginState::kSignUp,
      std::optional<base::Time> last_used_timestamp = std::nullopt);

  explicit IdentityRequestAccount(
      const blink::common::webid::LoginStatusAccount& account);

  // The identity provider to which the account belongs to. This is not set in
  // the constructor but instead set later.
  scoped_refptr<IdentityProviderData> identity_provider = nullptr;

  std::string id;
  // E.g. email or phone number
  std::string display_identifier;
  // E.g. the user's full name or username
  std::string display_name;
  std::string email;
  std::string name;
  std::string given_name;
  GURL picture;
  std::string phone;
  std::string username;
  // This will be an empty image if fetching failed.
  gfx::Image decoded_picture;

  std::vector<std::string> login_hints;
  std::vector<std::string> domain_hints;
  std::vector<std::string> labels;

  // The list of fields the UI should prompt the user for. This is based on the
  // fields that the RP requested and affected by the login state and the
  // actual available fields in the IDP accounts response.
  std::vector<IdentityRequestDialogDisclosureField> fields;

  // The account login state populated by the IDP through an approved clients
  // list.
  std::optional<LoginState> idp_claimed_login_state;

  // The account login state populated by the browser based on stored permission
  // grants.
  LoginState browser_trusted_login_state;

  // The last used timestamp, or nullopt if the account has not been used
  // before.
  std::optional<base::Time> last_used_timestamp;

  // Whether this account is filtered out or not. An account may be filtered out
  // due to login hint, domain hint, or account label.
  bool is_filtered_out = false;

  // Whether this account was retrieved from the Lightweight FedCM Accounts Push
  // storage. If this is true, the request for the account picture will only
  // check against cache, and will fail on cache miss.
  bool from_accounts_push = false;

 private:
  friend class base::RefCounted<IdentityRequestAccount>;

  ~IdentityRequestAccount();
};

}  // namespace content

#endif  // CONTENT_PUBLIC_BROWSER_WEBID_IDENTITY_REQUEST_ACCOUNT_H_
