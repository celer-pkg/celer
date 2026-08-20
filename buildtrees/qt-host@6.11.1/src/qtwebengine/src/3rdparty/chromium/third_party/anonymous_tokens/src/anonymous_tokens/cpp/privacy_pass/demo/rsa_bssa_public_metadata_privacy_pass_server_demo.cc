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
// bazel run -c opt :rsa_bssa_public_metadata_privacy_pass_server_demo
// --cxxopt='-std=c++17'

#include <algorithm>
#include <iostream>
#include <ostream>
#include <string>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/escaping.h"
#include "anonymous_tokens/cpp/crypto/crypto_utils.h"
#include "anonymous_tokens/cpp/privacy_pass/token_encodings.h"
#include "anonymous_tokens/cpp/testing/utils.h"
#include <openssl/base.h>

absl::Status RunDemo() {
  // Construct RSA private key with a strong rsa modulus.
  auto [test_rsa_public_key, test_rsa_private_key] =
      anonymous_tokens::GetStrongTestRsaKeyPair2048();
  absl::StatusOr<bssl::UniquePtr<RSA>> rsa_private_key =
      anonymous_tokens::CreatePrivateKeyRSA(
          test_rsa_private_key.n, test_rsa_private_key.e,
          test_rsa_private_key.d, test_rsa_private_key.p,
          test_rsa_private_key.q, test_rsa_private_key.dp,
          test_rsa_private_key.dq, test_rsa_private_key.crt);
  if (!rsa_private_key.ok()) {
    return rsa_private_key.status();
  }

  // Get base64url encoded public key for client to use.
  absl::StatusOr<bssl::UniquePtr<RSA>> rsa_public_key =
      anonymous_tokens::CreatePublicKeyRSA(
          test_rsa_public_key.n, test_rsa_public_key.e);
  if (!rsa_public_key.ok()) {
    return rsa_public_key.status();
  }

  // Compute RSA BSSA Public Key in DER encoded format.
  absl::StatusOr<std::string> public_key_der =
      anonymous_tokens::RsaSsaPssPublicKeyToDerEncoding(
          rsa_public_key.value().get());
  if (!public_key_der.ok()) {
    return public_key_der.status();
  }
  std::string public_key_base64URL = absl::Base64Escape(public_key_der.value());

  // Replace '+' with '-' and '/' with '_'
  // Using std::replace instead of absl::WebSafeBase64Escape as latter does not
  // pad the string with '=', which is required by Privacy Pass spec.
  std::replace(public_key_base64URL.begin(), public_key_base64URL.end(), '+',
               '-');
  std::replace(public_key_base64URL.begin(), public_key_base64URL.end(), '/',
               '_');
  std::cout << "type: 0xDA7A\n"
            << "public_key: " << public_key_base64URL << std::endl;

  // Wait for token request.
  std::cout << "Waiting for Token Type DA7A, Extended Token Request (in "
               "hexadecimal string format):"
            << std::endl;

  std::string extended_token_request_hex_str;
  std::string extended_token_request_str;

  std::cin >> extended_token_request_hex_str;
  if (!absl::HexStringToBytes(extended_token_request_hex_str,
                              &extended_token_request_str)) {
    return absl::InvalidArgumentError("Invalid extended token request");
  }

  absl::StatusOr<anonymous_tokens::ExtendedTokenRequest>
      extended_token_request =
          anonymous_tokens::UnmarshalExtendedTokenRequest(
              extended_token_request_str);
  if (!extended_token_request.ok()) {
    return extended_token_request.status();
  }

  // Sign token request.
  absl::StatusOr<std::string> encoded_extensions =
      anonymous_tokens::EncodeExtensions(
          (*extended_token_request).extensions);
  if (!encoded_extensions.ok()) {
    return encoded_extensions.status();
  }

  absl::StatusOr<std::string> signature =
      anonymous_tokens::TestSignWithPublicMetadata(
          (*extended_token_request).request.blinded_token_request,
          /*public_metadata=*/*encoded_extensions, *rsa_private_key.value(),
          /*use_rsa_public_exponent=*/false);
  if (!signature.ok()) {
    return signature.status();
  }
  std::cout
      << "Token Type DA7A, Token Response (in hexadecimal string format):\n"
      << absl::BytesToHexString(signature.value()) << std::endl;

  return absl::OkStatus();
}

int main(int argc, char** argv) {
  absl::Status status = RunDemo();
  if (!status.ok()) {
    std::cout << status << std::endl;
    return -1;
  } else {
    std::cout << "Server demo ran successfully!" << std::endl;
  }
  return 0;
}
