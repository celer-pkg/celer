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

#ifndef ANONYMOUS_TOKENS_CPP_CRYPTO_CRYPTO_UTILS_H_
#define ANONYMOUS_TOKENS_CPP_CRYPTO_CRYPTO_UTILS_H_

#include <stddef.h>

#include <memory>
#include <optional>
#include <string>

#include "absl/status/statusor.h"
#include "absl/strings/string_view.h"
#include <openssl/base.h>
#include <openssl/bn.h>
#include <openssl/evp.h>
#include <openssl/rsa.h>

namespace anonymous_tokens {

// Internal functions only exposed for testing.
namespace internal {

// Outputs a public metadata `hash` using HKDF with the public metadata as
// input and the rsa modulus as salt. The expected output hash size is passed as
// out_len_bytes.
//
// Implementation follows the steps listed in
// https://datatracker.ietf.org/doc/draft-amjad-cfrg-partially-blind-rsa/
//
// This method internally calls HKDF with output size of more than
// out_len_bytes and later truncates the output to out_len_bytes. This is done
// so that the output is indifferentiable from truly random bytes.
// https://cfrg.github.io/draft-irtf-cfrg-hash-to-curve/draft-irtf-cfrg-hash-to-curve.html#name-hashing-to-a-finite-field
absl::StatusOr<bssl::UniquePtr<BIGNUM> > PublicMetadataHashWithHKDF(
    absl::string_view public_metadata, absl::string_view rsa_modulus_str,
    size_t out_len_bytes);

}  // namespace internal

// Deletes a BN_CTX.
class BnCtxDeleter {
 public:
  void operator()(BN_CTX* ctx) { BN_CTX_free(ctx); }
};
typedef std::unique_ptr<BN_CTX, BnCtxDeleter> BnCtxPtr;

// Deletes a BN_MONT_CTX.
class BnMontCtxDeleter {
 public:
  void operator()(BN_MONT_CTX* mont_ctx) { BN_MONT_CTX_free(mont_ctx); }
};
typedef std::unique_ptr<BN_MONT_CTX, BnMontCtxDeleter> BnMontCtxPtr;

// Deletes an EVP_MD_CTX.
class EvpMdCtxDeleter {
 public:
  void operator()(EVP_MD_CTX* ctx) { EVP_MD_CTX_destroy(ctx); }
};
typedef std::unique_ptr<EVP_MD_CTX, EvpMdCtxDeleter> EvpMdCtxPtr;

// Creates and starts a BIGNUM context.
absl::StatusOr<BnCtxPtr> GetAndStartBigNumCtx();

// Creates a new BIGNUM.
absl::StatusOr<bssl::UniquePtr<BIGNUM> > NewBigNum();

// Converts a BIGNUM to string.
absl::StatusOr<std::string> BignumToString(const BIGNUM& big_num,
                                           size_t output_len);

// Converts a string to BIGNUM.
absl::StatusOr<bssl::UniquePtr<BIGNUM> > StringToBignum(
    absl::string_view input_str);

// Retrieve error messages from OpenSSL.
std::string GetSslErrors();

// Mask message using protocol at
// https://datatracker.ietf.org/doc/draft-irtf-cfrg-rsa-blind-signatures/
std::string MaskMessageConcat(absl::string_view mask,
                              absl::string_view message);

// Encode Message and Public Metadata using steps in
// https://datatracker.ietf.org/doc/draft-amjad-cfrg-partially-blind-rsa/
//
// The length of public metadata must fit in 4 bytes.
std::string EncodeMessagePublicMetadata(absl::string_view message,
                                        absl::string_view public_metadata);

// Compute 2^(x - 1/2).
absl::StatusOr<bssl::UniquePtr<BIGNUM> > GetRsaSqrtTwo(int x);

// Compute compute 2^x.
absl::StatusOr<bssl::UniquePtr<BIGNUM> > ComputePowerOfTwo(int x);

// ComputeHash sub-routine used during blindness and verification of RSA blind
// signatures protocol with or without public metadata.
absl::StatusOr<std::string> ComputeHash(absl::string_view input,
                                        const EVP_MD& hasher);

// Computes the Carmichael LCM given phi(p) and phi(q) where N = p*q is a safe
// RSA modulus.
absl::StatusOr<bssl::UniquePtr<BIGNUM> > ComputeCarmichaelLcm(
    const BIGNUM& phi_p, const BIGNUM& phi_q, BN_CTX& bn_ctx);

// Create bssl::UniquePtr<RSA> representing a RSA private key.
//
// Note that this method should not be used to create a key with public exponent
// greater than 2^32.
absl::StatusOr<bssl::UniquePtr<RSA> > CreatePrivateKeyRSA(
    absl::string_view rsa_modulus, absl::string_view public_exponent,
    absl::string_view private_exponent, absl::string_view p,
    absl::string_view q, absl::string_view dp, absl::string_view dq,
    absl::string_view crt);

// Create bssl::UniquePtr<RSA> representing a RSA public key.
//
// Note that this method should not be used to create a key with public exponent
// greater than 2^32.
absl::StatusOr<bssl::UniquePtr<RSA> > CreatePublicKeyRSA(
    absl::string_view rsa_modulus, absl::string_view public_exponent);

// Create bssl::UniquePtr<RSA> representing a RSA public key derived using
// public metadata.
//
// If the boolean "use_rsa_public_exponent" is set to false, the public exponent
// is not used in any computations.
//
// Setting "use_rsa_public_exponent" to true is deprecated.
absl::StatusOr<bssl::UniquePtr<RSA> > CreatePublicKeyRSAWithPublicMetadata(
    const BIGNUM& rsa_modulus, const BIGNUM& public_exponent,
    absl::string_view public_metadata, bool use_rsa_public_exponent);

// Create bssl::UniquePtr<RSA> representing a RSA public key derived using
// public metadata.
//
// If the boolean "use_rsa_public_exponent" is set to false, the public exponent
// is not used in any computations.
//
// Setting "use_rsa_public_exponent" to true is deprecated.
absl::StatusOr<bssl::UniquePtr<RSA> > CreatePublicKeyRSAWithPublicMetadata(
    absl::string_view rsa_modulus, absl::string_view public_exponent,
    absl::string_view public_metadata, bool use_rsa_public_exponent);

// Compute exponent using only the public metadata and RSA modulus n. Assumes
// that n is a safe modulus i.e. it produces a strong RSA key pair. If not, the
// exponent may be invalid.
//
// Empty public metadata is considered to be a valid value for public_metadata
// and will output a valid exponent.
absl::StatusOr<bssl::UniquePtr<BIGNUM> > ComputeExponentWithPublicMetadata(
    const BIGNUM& n, absl::string_view public_metadata);

// Computes exponent by multiplying the public exponent e with the
// exponent derived from public metadata and RSA modulus n. Assumes that n is a
// safe modulus i.e. it produces a strong RSA key pair. If not, the exponent may
// be invalid.
//
// Empty public metadata is considered to be a valid value for public_metadata
// and will output an exponent different than `e` as well.
//
// This function is now deprecated.
absl::StatusOr<bssl::UniquePtr<BIGNUM> >
ComputeExponentWithPublicMetadataAndPublicExponent(
    const BIGNUM& n, const BIGNUM& e, absl::string_view public_metadata);

// Helper method that implements RSA PSS Blind Signatures verification protocol
// for both the standard scheme as well as the public metadata version.
//
// For the public metadata version,
//
// 1) `rsa_public_key' must contain a public exponent derived using the public
// metadata.
//
// 2) The `message' must be an encoding of an original input message
// and the public metadata e.g. by using EncodeMessagePublicMetadata method in
// this file. The caller should make sure that its original input message is a
// random message. In case it is not, it should be concatenated with a random
// string.
absl::Status RsaBlindSignatureVerify(int salt_length, const EVP_MD* sig_hash,
                                     const EVP_MD* mgf1_hash,
                                     absl::string_view signature,
                                     absl::string_view message,
                                     RSA* rsa_public_key);

// This method outputs a DER encoding of RSASSA-PSS (RSA Signature Scheme with
// Appendix - Probabilistic Signature Scheme) Public Key as described here
// https://datatracker.ietf.org/doc/html/rfc3447.html using the object
// identifier(s) here: https://oidref.com/1.2.840.113549.1.1.10  and using a
// fixed salt length of 48 bytes, SHA384 as the signature's hash function as
// well as the hash function that the signature's mask generating function is
// based on. A publicly availble equivalent function is available in Goa here:
// https://github.com/cloudflare/pat-go/blob/11579ba5b0b9b77d3e8e3d5247a98811227ac82e/x509util.go#L56
//
absl::StatusOr<std::string> RsaSsaPssPublicKeyToDerEncoding(const RSA* rsa);

// This method DER encodes inputted RSA public keys and hashes the encodings
// using SHA256. It returns true if the hashes collide on the least significant
// byte, otherwise it returns false.
//
// This method will be used to ensure that a new key's truncated key id does
// not collide with an existing key's truncated key id during key rotation as
// described here:
// https://www.ietf.org/archive/id/draft-ietf-privacypass-protocol-10.html#public-issuer-configuration
absl::StatusOr<bool> PrivacyPassTruncatedTokenKeyIdCollision(
    const RSA* public_key, const RSA* other_public_key);

}  // namespace anonymous_tokens

#endif  // ANONYMOUS_TOKENS_CPP_CRYPTO_CRYPTO_UTILS_H_
