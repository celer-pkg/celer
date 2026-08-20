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

#ifndef ANONYMOUS_TOKENS_CPP_PRIVACY_PASS_RSA_BSSA_PUBLIC_METADATA_CLIENT_H_
#define ANONYMOUS_TOKENS_CPP_PRIVACY_PASS_RSA_BSSA_PUBLIC_METADATA_CLIENT_H_

#include <memory>
#include <string>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/string_view.h"
#include "anonymous_tokens/cpp/crypto/rsa_blinder.h"
#include "anonymous_tokens/cpp/privacy_pass/token_encodings.h"
#include <openssl/base.h>

namespace anonymous_tokens {

class PrivacyPassRsaBssaPublicMetadataClient {
 public:
  #ifndef SWIG
  // PrivacyPassRsaBssaPublicMetadataClient is neither copyable nor copy
  // assignable.
  PrivacyPassRsaBssaPublicMetadataClient(
      const PrivacyPassRsaBssaPublicMetadataClient&) = delete;
  PrivacyPassRsaBssaPublicMetadataClient& operator=(
      const PrivacyPassRsaBssaPublicMetadataClient&) = delete;
  #endif
  // This method is to be used to create a client as its constructor is private.
  // It takes as input RSA public key.
  static absl::StatusOr<
      std::unique_ptr<PrivacyPassRsaBssaPublicMetadataClient> >
  Create(const RSA& rsa_public_key);

  // Method used to create the ExtendedTokenRequest. It takes in the input
  // "challenge" as an encoded string, "nonce" must a 32 byte random string,
  // "token_key_id" is the SHA256 digest of the DER encoding of RSA BSSA public
  // key containing the correct hash functions and salt size and "extensions" is
  // the structure carrying the public metadata / info.
  //
  // https://www.ietf.org/archive/id/draft-hendrickson-privacypass-public-metadata-01.html#name-client-to-issuer-request-2
  //
  // CreateTokenRequest must be called before FinalizeToken.
  absl::StatusOr<ExtendedTokenRequest> CreateTokenRequest(
      absl::string_view challenge, absl::string_view nonce,
      absl::string_view token_key_id, const Extensions& extensions);

  // Method that uses the client state and outputs the final token by unblinding
  // the "blinded_signature".
  //
  // https://www.ietf.org/archive/id/draft-hendrickson-privacypass-public-metadata-01.html#name-finalization-2
  //
  // CreateTokenRequest must be called before FinalizeToken.
  absl::StatusOr<Token> FinalizeToken(absl::string_view blinded_signature);

  // Method that takes in a token, extensions encoded as a string and the RSA
  // public key to run the token verification algorithm. It returns an ok status
  // on success and errs on verification failure.
  //
  // https://datatracker.ietf.org/doc/draft-hendrickson-privacypass-public-metadata/
  static absl::Status Verify(Token token_to_verify,
                             absl::string_view encoded_extensions,
                             RSA& rsa_public_key);

  static constexpr uint16_t kTokenType = 0xDA7A;

 private:
  PrivacyPassRsaBssaPublicMetadataClient(int salt_length,
                                         std::string rsa_modulus,
                                         std::string rsa_e,
                                         const EVP_MD* signature_hash_function,
                                         const EVP_MD* mgf1_hash_function);

  const int salt_length_;
  const std::string rsa_modulus_;
  const std::string rsa_e_;
  const EVP_MD* const signature_hash_function_;  // Owned by BoringSSL.
  const EVP_MD* const mgf1_hash_function_;       // Owned by BoringSSL.

  // RsaBlinder object to generate the token request and finalize the token.
  // Once CreateTokenRequest is called, this value is initialized and is no
  // longer a nullptr.
  std::unique_ptr<RsaBlinder> rsa_blinder_ = nullptr;
  // This Token object will be finalized and returned when FinalizeToken is
  // called.
  Token token_;
  // String used as input for (1) creating the token and (2) verifying the final
  // token against, under some fixed input extensions.
  std::string authenticator_input_;
};

}  // namespace anonymous_tokens

#endif  // ANONYMOUS_TOKENS_CPP_PRIVACY_PASS_RSA_BSSA_PUBLIC_METADATA_CLIENT_H_
