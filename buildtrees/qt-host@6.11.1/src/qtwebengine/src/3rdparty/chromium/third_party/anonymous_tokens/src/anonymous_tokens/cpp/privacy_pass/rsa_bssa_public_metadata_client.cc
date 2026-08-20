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

#include "anonymous_tokens/cpp/privacy_pass/rsa_bssa_public_metadata_client.h"

#include <memory>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/string_view.h"
#include "anonymous_tokens/cpp/crypto/constants.h"
#include "anonymous_tokens/cpp/crypto/crypto_utils.h"
#include "anonymous_tokens/cpp/crypto/rsa_blinder.h"
#include "anonymous_tokens/cpp/privacy_pass/token_encodings.h"
#include "anonymous_tokens/cpp/shared/status_utils.h"
#include <openssl/base.h>
#include <openssl/digest.h>
#include <openssl/rsa.h>

namespace anonymous_tokens {

namespace {

absl::Status CheckKeySize(const RSA& key) {
  if (RSA_size(&key) != kRsaModulusSizeInBytes256) {
    return absl::InvalidArgumentError(
        "Token type DA7A must use RSA key with the modulus of size 256 bytes.");
  }
  return absl::OkStatus();
}

}  // namespace

absl::StatusOr<std::unique_ptr<PrivacyPassRsaBssaPublicMetadataClient>>
PrivacyPassRsaBssaPublicMetadataClient::Create(const RSA& rsa_public_key) {
  ANON_TOKENS_RETURN_IF_ERROR(CheckKeySize(rsa_public_key));

  // Create modulus and public exponent strings.
  ANON_TOKENS_ASSIGN_OR_RETURN(
      const std::string rsa_modulus,
      BignumToString(*RSA_get0_n(&rsa_public_key), kRsaModulusSizeInBytes256));
  ANON_TOKENS_ASSIGN_OR_RETURN(
      const std::string rsa_e,
      BignumToString(*RSA_get0_e(&rsa_public_key), kRsaModulusSizeInBytes256));

  // Create hash digest methods.
  const EVP_MD* signature_hash_function = EVP_sha384();
  const EVP_MD* mgf1_hash_function = EVP_sha384();

  // Create and return client.
  return absl::WrapUnique(new PrivacyPassRsaBssaPublicMetadataClient(
      kSaltLengthInBytes48, rsa_modulus, rsa_e, signature_hash_function,
      mgf1_hash_function));
}

PrivacyPassRsaBssaPublicMetadataClient::PrivacyPassRsaBssaPublicMetadataClient(
    const int salt_length, const std::string rsa_modulus,
    const std::string rsa_e, const EVP_MD* signature_hash_function,
    const EVP_MD* mgf1_hash_function)
    : salt_length_(salt_length),
      rsa_modulus_(rsa_modulus),
      rsa_e_(rsa_e),
      signature_hash_function_(signature_hash_function),
      mgf1_hash_function_(mgf1_hash_function) {}

absl::StatusOr<ExtendedTokenRequest>
PrivacyPassRsaBssaPublicMetadataClient::CreateTokenRequest(
    const absl::string_view challenge, const absl::string_view nonce,
    const absl::string_view token_key_id, const Extensions& extensions) {
  // Basic validity checks.
  if (rsa_blinder_ != nullptr) {
    return absl::FailedPreconditionError(
        "CreateTokenRequest has already been called.");
  } else if (token_key_id.size() != 32) {
    return absl::InvalidArgumentError("token_key_id must be of size 32 bytes.");
  }

  // Compute context as sha256 of the challenge.
  const EVP_MD* sha256 = EVP_sha256();
  ANON_TOKENS_ASSIGN_OR_RETURN(const std::string context,
                               ComputeHash(challenge, *sha256));

  // Populate the token_ object except for the final signature i.e.
  // authenticator field.
  token_ = {/*token_type=*/kTokenType,
            /*token_key_id=*/std::string(token_key_id),
            /*nonce=*/std::string(nonce),
            /*context=*/context};

  // Encode extensions to string.
  ANON_TOKENS_ASSIGN_OR_RETURN(const std::string encoded_extensions,
                               EncodeExtensions(extensions));

  // Create RsaBlinder object.
  ANON_TOKENS_ASSIGN_OR_RETURN(
      rsa_blinder_,
      RsaBlinder::New(rsa_modulus_, rsa_e_, signature_hash_function_,
                      mgf1_hash_function_, salt_length_,
                      /*use_rsa_public_exponent=*/false,
                      /*public_metadata=*/encoded_extensions));

  // Call Blind on an encoding of the input message.
  ANON_TOKENS_ASSIGN_OR_RETURN(authenticator_input_,
                               AuthenticatorInput(token_));
  ANON_TOKENS_ASSIGN_OR_RETURN(const std::string blinded_message,
                               rsa_blinder_->Blind(authenticator_input_));

  // Create the token_request using the token_type, the last byte of the
  // token_key_id and the blinded_message.
  TokenRequest token_request = {
      /*token_type=*/kTokenType,
      /*truncated_token_key_id=*/
      static_cast<uint8_t>(token_key_id[token_key_id.size() - 1]),
      /*blinded_token_request=*/blinded_message};

  // ExtendedTokenRequest carries the public metadata / encoded extensions list.
  ExtendedTokenRequest extended_token_request = {/*request=*/token_request,
                                                 /*extensions=*/extensions};
  return extended_token_request;
}

absl::StatusOr<Token> PrivacyPassRsaBssaPublicMetadataClient::FinalizeToken(
    const absl::string_view blinded_signature) {
  if (rsa_blinder_ == nullptr) {
    return absl::FailedPreconditionError(
        "CreateRequest must be called before FinalizeToken.");
  }

  // Unblind the blinded signature to obtain the final signature and store it as
  // authenticator in token_.
  ANON_TOKENS_ASSIGN_OR_RETURN(token_.authenticator,
                               rsa_blinder_->Unblind(blinded_signature));

  // Verify the signature for correctness.
  ANON_TOKENS_RETURN_IF_ERROR(rsa_blinder_->Verify(
      /*signature=*/token_.authenticator, /*message=*/authenticator_input_));

  return token_;
}

absl::Status PrivacyPassRsaBssaPublicMetadataClient::Verify(
    Token token_to_verify, const absl::string_view encoded_extensions,
    RSA& rsa_public_key) {
  ANON_TOKENS_RETURN_IF_ERROR(CheckKeySize(rsa_public_key));
  ANON_TOKENS_ASSIGN_OR_RETURN(
      bssl::UniquePtr<RSA> derived_rsa_public_key,
      CreatePublicKeyRSAWithPublicMetadata(
          *RSA_get0_n(&rsa_public_key), *RSA_get0_e(&rsa_public_key),
          encoded_extensions, /*use_rsa_public_exponent=*/false));

  // Prepare input parameters for the verification function.
  const EVP_MD* signature_hash_function = EVP_sha384();
  const EVP_MD* mgf1_hash_function = EVP_sha384();

  ANON_TOKENS_ASSIGN_OR_RETURN(const std::string authenticator_input,
                               AuthenticatorInput(token_to_verify));
  std::string augmented_message =
      EncodeMessagePublicMetadata(authenticator_input, encoded_extensions);

  return RsaBlindSignatureVerify(
      kSaltLengthInBytes48, signature_hash_function, mgf1_hash_function,
      /*signature=*/token_to_verify.authenticator,
      /*message=*/augmented_message, derived_rsa_public_key.get());
}

}  // namespace anonymous_tokens
