// Copyright 2023 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// To run this binary from this directory use:
// bazel run -c opt :rsa_bssa_public_metadata_privacy_pass_client_demo
// --cxxopt='-std=c++17'

#include <cstdint>
#include <iostream>
#include <memory>
#include <ostream>
#include <random>
#include <string>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/escaping.h"
#include "anonymous_tokens/cpp/crypto/crypto_utils.h"
#include "anonymous_tokens/cpp/privacy_pass/rsa_bssa_public_metadata_client.h"
#include "anonymous_tokens/cpp/privacy_pass/token_encodings.h"
#include "anonymous_tokens/cpp/testing/utils.h"
#include <openssl/base.h>
#include <openssl/digest.h>

// Generates a random string of size string_length.
std::string GetRandomString(int string_length) {
  std::mt19937_64 generator;
  std::uniform_int_distribution<int> distr_u8 =
      std::uniform_int_distribution<int>{0, 255};
  std::string rand(string_length, 0);
  for (int i = 0; i < string_length; ++i) {
    rand[i] = static_cast<uint8_t>((distr_u8)(generator));
  }
  return rand;
}

absl::Status RunDemo() {
  // Construct RSA public key with a strong rsa modulus.
  auto [test_rsa_public_key, _] =
      anonymous_tokens::GetStrongTestRsaKeyPair2048();
  absl::StatusOr<bssl::UniquePtr<RSA>> rsa_public_key =
      anonymous_tokens::CreatePublicKeyRSA(
          test_rsa_public_key.n, test_rsa_public_key.e);
  if (!rsa_public_key.ok()) {
    return rsa_public_key.status();
  }

  // Compute RSA BSSA Public Key Token ID.
  absl::StatusOr<std::string> public_key_der =
      anonymous_tokens::RsaSsaPssPublicKeyToDerEncoding(
          rsa_public_key.value().get());
  if (!public_key_der.ok()) {
    return public_key_der.status();
  }
  const EVP_MD* sha256 = EVP_sha256();
  absl::StatusOr<std::string> token_key_id =
      anonymous_tokens::ComputeHash(public_key_der.value(),
                                                        *sha256);
  if (!token_key_id.ok()) {
    return token_key_id.status();
  }

  // Create the privacy pass client.
  static absl::StatusOr<
      std::unique_ptr<anonymous_tokens::
                          PrivacyPassRsaBssaPublicMetadataClient>>
      pp_client = anonymous_tokens::
          PrivacyPassRsaBssaPublicMetadataClient::Create(
              *rsa_public_key.value());
  if (!pp_client.ok()) {
    return pp_client.status();
  }

  // Create token challenge.
  anonymous_tokens::TokenChallenge challenge;
  challenge.issuer_name = "issuer.dummy.com";
  absl::StatusOr<std::string> token_challenge =
      anonymous_tokens::MarshalTokenChallenge(challenge);
  if (!token_challenge.ok()) {
    return token_challenge.status();
  }

  // Nonce is a random string of 32 bytes.
  std::string nonce = GetRandomString(/*string_length=*/32);

  // Public metadata for the purposes of this demo.
  anonymous_tokens::Extensions extensions;
  anonymous_tokens::GeoHint geo_hint;
  geo_hint.country_code = "US";
  absl::StatusOr<anonymous_tokens::Extension>
      geo_hint_extension = geo_hint.AsExtension();
  if (!geo_hint_extension.ok()) {
    return geo_hint_extension.status();
  }
  extensions.extensions.push_back(*geo_hint_extension);
  anonymous_tokens::ServiceType service_type;
  service_type.service_type_id =
      anonymous_tokens::ServiceType::kChromeIpBlinding;
  absl::StatusOr<anonymous_tokens::Extension>
      service_type_extension = service_type.AsExtension();
  if (!service_type_extension.ok()) {
    return service_type_extension.status();
  }
  extensions.extensions.push_back(*service_type_extension);

  absl::StatusOr<std::string> encoded_extensions =
      anonymous_tokens::EncodeExtensions(extensions);
  if (!encoded_extensions.ok()) {
    return encoded_extensions.status();
  }

  // Create token request.
  absl::StatusOr<anonymous_tokens::ExtendedTokenRequest>
      extended_token_request = pp_client.value()->CreateTokenRequest(
          token_challenge.value(), nonce, token_key_id.value(), extensions);
  if (!extended_token_request.ok()) {
    return extended_token_request.status();
  }
  absl::StatusOr<std::string> marshaled_extended_token_request =
      anonymous_tokens::MarshalExtendedTokenRequest(
          extended_token_request.value());
  if (!marshaled_extended_token_request.ok()) {
    return marshaled_extended_token_request.status();
  }

  // Output token request and wait for response.
  std::cout << "Token Type DA7A, Extended Token Request (in hexadecimal string "
               "format):\n"
            << absl::BytesToHexString(marshaled_extended_token_request.value())
            << std::endl;

  std::cout << "Please enter the token response (in hexadecimal string format):"
            << std::endl;

  std::string token_response;
  std::string token_response_hex_str;

  std::cin >> token_response_hex_str;
  if (!absl::HexStringToBytes(token_response_hex_str, &token_response)) {
    return absl::InvalidArgumentError("Invalid token response.");
  }

  // Finalize the token.
  absl::StatusOr<anonymous_tokens::Token> final_token =
      pp_client.value()->FinalizeToken(token_response);
  if (!final_token.ok()) {
    return final_token.status();
  }

  // Run public key verification successfully.
  absl::Status is_verified = anonymous_tokens::
      PrivacyPassRsaBssaPublicMetadataClient::Verify(final_token.value(),
                                                     encoded_extensions.value(),
                                                     *rsa_public_key.value());
  if (!is_verified.ok()) {
    return is_verified;
  }
  std::cout << "Token successfully finalized and verified." << std::endl;

  return absl::OkStatus();
}

int main(int argc, char** argv) {
  absl::Status status = RunDemo();
  if (!status.ok()) {
    std::cout << status << std::endl;
    return -1;
  } else {
    std::cout << "Client demo ran successfully!" << std::endl;
  }
  return 0;
}
