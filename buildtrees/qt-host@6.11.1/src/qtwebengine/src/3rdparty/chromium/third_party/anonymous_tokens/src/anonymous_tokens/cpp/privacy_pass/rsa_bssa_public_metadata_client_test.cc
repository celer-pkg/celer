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

#include <sys/types.h>

#include <memory>

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include "absl/status/statusor.h"
#include "anonymous_tokens/cpp/crypto/crypto_utils.h"
#include "anonymous_tokens/cpp/privacy_pass/token_encodings.h"
#include "anonymous_tokens/cpp/testing/utils.h"
#include <openssl/digest.h>

namespace anonymous_tokens {
namespace {

class PrivacyPassRsaBssaClientTest : public testing::Test {
 protected:
  void SetUp() override {
    // Seed the random string generator.
    generator_.seed(GTEST_FLAG_GET(random_seed));

    // Create RSA public and private key structs.
    auto [test_rsa_public_key, test_rsa_private_key] =
        GetStrongTestRsaKeyPair2048();
    ANON_TOKENS_ASSERT_OK_AND_ASSIGN(
        rsa_public_key_,
        CreatePublicKeyRSA(test_rsa_public_key.n, test_rsa_public_key.e));
    ANON_TOKENS_ASSERT_OK_AND_ASSIGN(
        rsa_private_key_,
        CreatePrivateKeyRSA(test_rsa_private_key.n, test_rsa_private_key.e,
                            test_rsa_private_key.d, test_rsa_private_key.p,
                            test_rsa_private_key.q, test_rsa_private_key.dp,
                            test_rsa_private_key.dq, test_rsa_private_key.crt));
    // Compute RSA BSSA Public Key Token ID.
    ANON_TOKENS_ASSERT_OK_AND_ASSIGN(
        std::string public_key_der,
        RsaSsaPssPublicKeyToDerEncoding(rsa_public_key_.get()));
    const EVP_MD* sha256 = EVP_sha256();
    ANON_TOKENS_ASSERT_OK_AND_ASSIGN(token_key_id_,
                                     ComputeHash(public_key_der, *sha256));

    // Create a client using the rsa_public_key_.
    ANON_TOKENS_ASSERT_OK_AND_ASSIGN(
        client_,
        PrivacyPassRsaBssaPublicMetadataClient::Create(*rsa_public_key_.get()));

    // Challenge for the purposes of this test is a random 80 bytes string.
    challenge_encoding_ = GetRandomString(/*string_length=*/80);
    // Nonce is a random string of 32 bytes.
    nonce_ = GetRandomString(/*string_length=*/32);
    // Random extensions / public metadata for the purposes of this test.
    extensions_ = {{CreateTestExtension(1), CreateTestExtension(2)}};
  }

  // Generates a random string of size string_length.
  std::string GetRandomString(int string_length) {
    std::string rand(string_length, 0);
    for (int i = 0; i < string_length; ++i) {
      rand[i] = static_cast<uint8_t>((distr_u8_)(generator_));
    }
    return rand;
  }

  // Creates test extension by setting the type to extension_type and the value
  // to a random string of size 'extension_type'.
  Extension CreateTestExtension(uint16_t extension_type) {
    return {
        /*extension_type=*/extension_type,
        /*extension_value=*/GetRandomString(/*string_length=*/extension_type)};
  }

  bssl::UniquePtr<RSA> rsa_public_key_;
  bssl::UniquePtr<RSA> rsa_private_key_;
  std::string token_key_id_;

  std::unique_ptr<PrivacyPassRsaBssaPublicMetadataClient> client_;
  Extensions extensions_;
  std::string challenge_encoding_;
  std::string nonce_;

  std::mt19937_64 generator_;
  std::uniform_int_distribution<int> distr_u8_ =
      std::uniform_int_distribution<int>{0, 255};
};

TEST_F(PrivacyPassRsaBssaClientTest, WrongKeySize) {
  auto [test_rsa_public_key, _] = GetStrongTestRsaKeyPair3072();
  ANON_TOKENS_ASSERT_OK_AND_ASSIGN(
      bssl::UniquePtr<RSA> wrong_rsa_public_key,
      CreatePublicKeyRSA(test_rsa_public_key.n, test_rsa_public_key.e));

  // Passing wrong key.
  absl::StatusOr<std::unique_ptr<PrivacyPassRsaBssaPublicMetadataClient>>
      client = PrivacyPassRsaBssaPublicMetadataClient::Create(
          *wrong_rsa_public_key.get());

  EXPECT_FALSE(client.status().ok());
  EXPECT_EQ(client.status().code(), absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(client.status().message(),
              ::testing::HasSubstr("Token type DA7A must use RSA key with the "
                                   "modulus of size 256 bytes"));
}

TEST_F(PrivacyPassRsaBssaClientTest, WrongSizeOfTokenKeyID) {
  // Token key ID of invalid size 0.
  absl::StatusOr<ExtendedTokenRequest> token_req = client_->CreateTokenRequest(
      /*challenge=*/"", /*nonce=*/"", /*token_key_id=*/"", /*extensions=*/{});

  EXPECT_FALSE(token_req.status().ok());
  EXPECT_EQ(token_req.status().code(), absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(token_req.status().message(),
              ::testing::HasSubstr("token_key_id must be of size 32 bytes"));
}

TEST_F(PrivacyPassRsaBssaClientTest, CreateRequestTwice) {
  ANON_TOKENS_ASSERT_OK_AND_ASSIGN(
      ExtendedTokenRequest _,
      client_->CreateTokenRequest(/*challenge=*/"", /*nonce=*/"", token_key_id_,
                                  /*extensions=*/{}));
  // 2nd request.
  absl::StatusOr<ExtendedTokenRequest> token_req_2 =
      client_->CreateTokenRequest(
          /*challenge=*/"", /*nonce=*/"", token_key_id_, /*extensions=*/{});

  EXPECT_FALSE(token_req_2.status().ok());
  EXPECT_EQ(token_req_2.status().code(), absl::StatusCode::kFailedPrecondition);
  EXPECT_THAT(
      token_req_2.status().message(),
      ::testing::HasSubstr("CreateTokenRequest has already been called"));
}

TEST_F(PrivacyPassRsaBssaClientTest, FinalizeTokenWihtoutCreatingRequest) {
  const std::string dummy_signature = GetRandomString(/*string_length=*/256);
  absl::StatusOr<Token> token = client_->FinalizeToken(dummy_signature);

  EXPECT_FALSE(token.status().ok());
  EXPECT_EQ(token.status().code(), absl::StatusCode::kFailedPrecondition);
  EXPECT_THAT(token.status().message(),
              ::testing::HasSubstr(
                  "CreateRequest must be called before FinalizeToken"));
}

TEST_F(PrivacyPassRsaBssaClientTest, FinalizeTokenWithEmptySignature) {
  // Create token request.
  ANON_TOKENS_ASSERT_OK_AND_ASSIGN(
      ExtendedTokenRequest token_req,
      client_->CreateTokenRequest(challenge_encoding_, nonce_, token_key_id_,
                                  extensions_));
  // Finalize the token with empty signature.
  absl::StatusOr<Token> token = client_->FinalizeToken("");

  EXPECT_FALSE(token.status().ok());
  EXPECT_EQ(token.status().code(), absl::StatusCode::kInternal);
  EXPECT_THAT(token.status().message(),
              ::testing::HasSubstr("Expected blind signature size = 256"));
}

TEST_F(PrivacyPassRsaBssaClientTest, FinalizeWrongToken) {
  // Create token request.
  ANON_TOKENS_ASSERT_OK_AND_ASSIGN(
      ExtendedTokenRequest token_req,
      client_->CreateTokenRequest(challenge_encoding_, nonce_, token_key_id_,
                                  extensions_));
  const std::string dummy_signature = GetRandomString(256);
  // Finalize token with wrong signature.
  absl::StatusOr<Token> token = client_->FinalizeToken(dummy_signature);

  EXPECT_FALSE(token.status().ok());
  EXPECT_EQ(token.status().code(), absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(token.status().message(),
              ::testing::HasSubstr("PSS padding verification failed"));
}

TEST_F(PrivacyPassRsaBssaClientTest,
       FinalizeTokenWhereCreateRequestHasNoExtensions) {
  // Create token request without extensions.
  ANON_TOKENS_ASSERT_OK_AND_ASSIGN(
      ExtendedTokenRequest token_req,
      client_->CreateTokenRequest(challenge_encoding_, nonce_, token_key_id_,
                                  /*extensions=*/{}));
  // Compute token with populated extensions.
  ANON_TOKENS_ASSERT_OK_AND_ASSIGN(const std::string encoded_extensions,
                                   EncodeExtensions(extensions_));
  ANON_TOKENS_ASSERT_OK_AND_ASSIGN(
      const std::string signature,
      TestSignWithPublicMetadata(token_req.request.blinded_token_request,
                                 /*public_metadata=*/encoded_extensions,
                                 *rsa_private_key_.get(),
                                 /*use_rsa_public_exponent=*/true));
  // Finalize the token.
  absl::StatusOr<Token> token = client_->FinalizeToken(signature);

  EXPECT_FALSE(token.status().ok());
  EXPECT_EQ(token.status().code(), absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(token.status().message(),
              ::testing::HasSubstr("PSS padding verification failed"));
}

TEST_F(PrivacyPassRsaBssaClientTest, FinalizeTokenCreatedWithEmptyExtensions) {
  // Create token request.
  ANON_TOKENS_ASSERT_OK_AND_ASSIGN(
      ExtendedTokenRequest token_req,
      client_->CreateTokenRequest(challenge_encoding_, nonce_, token_key_id_,
                                  extensions_));
  // Compute token without encoded_extensions.
  ANON_TOKENS_ASSERT_OK_AND_ASSIGN(
      const std::string signature,
      TestSignWithPublicMetadata(token_req.request.blinded_token_request,
                                 /*public_metadata=*/"",
                                 *rsa_private_key_.get(),
                                 /*use_rsa_public_exponent=*/true));
  // Finalize the token.
  absl::StatusOr<Token> token = client_->FinalizeToken(signature);

  EXPECT_FALSE(token.status().ok());
  EXPECT_EQ(token.status().code(), absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(token.status().message(),
              ::testing::HasSubstr("PSS padding verification failed"));
}

TEST_F(PrivacyPassRsaBssaClientTest, FinalizeTokenTwice) {
  // Create token request.
  ANON_TOKENS_ASSERT_OK_AND_ASSIGN(
      ExtendedTokenRequest token_req,
      client_->CreateTokenRequest(challenge_encoding_, nonce_, token_key_id_,
                                  extensions_));
  // Compute correct token, given the ExtendedTokenRequest.
  ANON_TOKENS_ASSERT_OK_AND_ASSIGN(const std::string encoded_extensions,
                                   EncodeExtensions(token_req.extensions));
  ANON_TOKENS_ASSERT_OK_AND_ASSIGN(
      const std::string signature,
      TestSignWithPublicMetadata(token_req.request.blinded_token_request,
                                 /*public_metadata=*/encoded_extensions,
                                 *rsa_private_key_.get(),
                                 /*use_rsa_public_exponent=*/false));
  // Finalize the token.
  ANON_TOKENS_ASSERT_OK_AND_ASSIGN(const Token _,
                                   client_->FinalizeToken(signature));
  // Finalize the token again.
  absl::StatusOr<Token> token_2 = client_->FinalizeToken(signature);

  EXPECT_FALSE(token_2.status().ok());
  EXPECT_EQ(token_2.status().code(), absl::StatusCode::kFailedPrecondition);
  EXPECT_THAT(token_2.status().message(),
              ::testing::HasSubstr(
                  "RsaBlinder is in wrong state to unblind signature"));
}

TEST_F(PrivacyPassRsaBssaClientTest, FinalizeTokenWithWrongClient) {
  // Create token request.
  ANON_TOKENS_ASSERT_OK_AND_ASSIGN(
      ExtendedTokenRequest token_req_1,
      client_->CreateTokenRequest(challenge_encoding_, nonce_, token_key_id_,
                                  extensions_));
  // Create a request with client_2.
  ANON_TOKENS_ASSERT_OK_AND_ASSIGN(
      std::unique_ptr<PrivacyPassRsaBssaPublicMetadataClient> client_2,
      PrivacyPassRsaBssaPublicMetadataClient::Create(*rsa_public_key_.get()));
  ANON_TOKENS_ASSERT_OK_AND_ASSIGN(
      ExtendedTokenRequest _,
      client_2->CreateTokenRequest(challenge_encoding_, nonce_, token_key_id_,
                                   extensions_));
  // Compute correct signature for client_, given token_req_1.
  ANON_TOKENS_ASSERT_OK_AND_ASSIGN(const std::string encoded_extensions,
                                   EncodeExtensions(token_req_1.extensions));
  ANON_TOKENS_ASSERT_OK_AND_ASSIGN(
      const std::string signature,
      TestSignWithPublicMetadata(token_req_1.request.blinded_token_request,
                                 /*public_metadata=*/encoded_extensions,
                                 *rsa_private_key_.get(),
                                 /*use_rsa_public_exponent=*/false));
  // Finalize the token with wrong client_2.
  absl::StatusOr<Token> token = client_2->FinalizeToken(signature);

  EXPECT_FALSE(token.status().ok());
  EXPECT_EQ(token.status().code(), absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(token.status().message(),
              ::testing::HasSubstr("PSS padding verification failed"));
}

TEST_F(PrivacyPassRsaBssaClientTest, VerifyWithWrongExtensions) {
  // Create token request.
  ANON_TOKENS_ASSERT_OK_AND_ASSIGN(
      ExtendedTokenRequest token_req,
      client_->CreateTokenRequest(challenge_encoding_, nonce_, token_key_id_,
                                  extensions_));
  // Compute correct token, given the ExtendedTokenRequest.
  ANON_TOKENS_ASSERT_OK_AND_ASSIGN(const std::string encoded_extensions,
                                   EncodeExtensions(token_req.extensions));
  ANON_TOKENS_ASSERT_OK_AND_ASSIGN(
      const std::string signature,
      TestSignWithPublicMetadata(token_req.request.blinded_token_request,
                                 /*public_metadata=*/encoded_extensions,
                                 *rsa_private_key_.get(),
                                 /*use_rsa_public_exponent=*/false));
  // Finalize the token.
  ANON_TOKENS_ASSERT_OK_AND_ASSIGN(const Token token,
                                   client_->FinalizeToken(signature));
  // Run public key verification using wrong extensions.
  const Extensions wrong_extensions = {
      {CreateTestExtension(2), CreateTestExtension(3)}};
  ANON_TOKENS_ASSERT_OK_AND_ASSIGN(const std::string wrong_encoded_extensions,
                                   EncodeExtensions(wrong_extensions));
  absl::Status verification = PrivacyPassRsaBssaPublicMetadataClient::Verify(
      token, wrong_encoded_extensions, *rsa_public_key_.get());

  EXPECT_FALSE(verification.ok());
  EXPECT_EQ(verification.code(), absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(verification.message(),
              ::testing::HasSubstr("PSS padding verification failed"));
}

TEST_F(PrivacyPassRsaBssaClientTest, VerifyWithEmptyExtensions) {
  // Create token request.
  ANON_TOKENS_ASSERT_OK_AND_ASSIGN(
      ExtendedTokenRequest token_req,
      client_->CreateTokenRequest(challenge_encoding_, nonce_, token_key_id_,
                                  extensions_));
  // Compute correct token, given the ExtendedTokenRequest.
  ANON_TOKENS_ASSERT_OK_AND_ASSIGN(const std::string encoded_extensions,
                                   EncodeExtensions(token_req.extensions));
  ANON_TOKENS_ASSERT_OK_AND_ASSIGN(
      const std::string signature,
      TestSignWithPublicMetadata(token_req.request.blinded_token_request,
                                 /*public_metadata=*/encoded_extensions,
                                 *rsa_private_key_.get(),
                                 /*use_rsa_public_exponent=*/false));
  // Finalize the token.
  ANON_TOKENS_ASSERT_OK_AND_ASSIGN(const Token token,
                                   client_->FinalizeToken(signature));
  // Run public key verification using empty extensions.
  ANON_TOKENS_ASSERT_OK_AND_ASSIGN(const std::string empty_encoded_extensions,
                                   EncodeExtensions(/*extensions=*/{}));
  absl::Status verification = PrivacyPassRsaBssaPublicMetadataClient::Verify(
      token, empty_encoded_extensions, *rsa_public_key_.get());

  EXPECT_FALSE(verification.ok());
  EXPECT_EQ(verification.code(), absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(verification.message(),
              ::testing::HasSubstr("PSS padding verification failed"));
}

TEST_F(PrivacyPassRsaBssaClientTest, VerifyWithWrongPublicKey) {
  // Create token request.
  ANON_TOKENS_ASSERT_OK_AND_ASSIGN(
      ExtendedTokenRequest token_req,
      client_->CreateTokenRequest(challenge_encoding_, nonce_, token_key_id_,
                                  extensions_));
  // Compute correct token, given the ExtendedTokenRequest.
  ANON_TOKENS_ASSERT_OK_AND_ASSIGN(const std::string encoded_extensions,
                                   EncodeExtensions(token_req.extensions));
  ANON_TOKENS_ASSERT_OK_AND_ASSIGN(
      const std::string signature,
      TestSignWithPublicMetadata(token_req.request.blinded_token_request,
                                 /*public_metadata=*/encoded_extensions,
                                 *rsa_private_key_.get(),
                                 /*use_rsa_public_exponent=*/false));
  // Finalize the token.
  ANON_TOKENS_ASSERT_OK_AND_ASSIGN(const Token token,
                                   client_->FinalizeToken(signature));
  // Run public key verification using a new key.
  auto [test_rsa_public_key, _] = GetAnotherStrongTestRsaKeyPair2048();
  ANON_TOKENS_ASSERT_OK_AND_ASSIGN(
      bssl::UniquePtr<RSA> wrong_rsa_public_key,
      CreatePublicKeyRSA(test_rsa_public_key.n, test_rsa_public_key.e));
  absl::Status verification = PrivacyPassRsaBssaPublicMetadataClient::Verify(
      token, encoded_extensions, *wrong_rsa_public_key.get());

  EXPECT_FALSE(verification.ok());
  EXPECT_EQ(verification.code(), absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(verification.message(),
              ::testing::HasSubstr("PSS padding verification failed"));
}

TEST_F(PrivacyPassRsaBssaClientTest, TokenCreationAndVerificationSuccess) {
  // Create token request.
  ANON_TOKENS_ASSERT_OK_AND_ASSIGN(
      ExtendedTokenRequest token_req,
      client_->CreateTokenRequest(challenge_encoding_, nonce_, token_key_id_,
                                  extensions_));
  // Compute correct token, given the ExtendedTokenRequest.
  ANON_TOKENS_ASSERT_OK_AND_ASSIGN(const std::string encoded_extensions,
                                   EncodeExtensions(token_req.extensions));
  ANON_TOKENS_ASSERT_OK_AND_ASSIGN(
      const std::string signature,
      TestSignWithPublicMetadata(token_req.request.blinded_token_request,
                                 /*public_metadata=*/encoded_extensions,
                                 *rsa_private_key_.get(),
                                 /*use_rsa_public_exponent=*/false));
  // Finalize the token.
  ANON_TOKENS_ASSERT_OK_AND_ASSIGN(const Token token,
                                   client_->FinalizeToken(signature));
  // Run public key verification successfully.
  EXPECT_TRUE(PrivacyPassRsaBssaPublicMetadataClient::Verify(
                  token, encoded_extensions, *rsa_public_key_.get())
                  .ok());
}

}  // namespace
}  // namespace anonymous_tokens
