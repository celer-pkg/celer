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

#include "anonymous_tokens/cpp/client/anonymous_tokens_redemption_client.h"

#include <cstdint>
#include <memory>
#include <random>
#include <string>
#include <utility>
#include <vector>

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include "anonymous_tokens/cpp/testing/utils.h"
#include "anonymous_tokens/proto/anonymous_tokens.pb.h"

namespace anonymous_tokens {
namespace {

using ::testing::HasSubstr;

// Generates a random string of size n.
std::string GetRandomString(int n, std::uniform_int_distribution<int>* distr_u8,
                            std::mt19937_64* generator) {
  std::string rand(n, 0);
  for (int i = 0; i < n; ++i) {
    rand[i] = static_cast<uint8_t>((*distr_u8)(*generator));
  }
  return rand;
}

// Saves redemption related public metadata and result for testing purposes for
// one token.
struct RedemptionInfoAndResult {
  std::string plaintext_message;
  std::string public_metadata;
  std::string message_mask;
  bool redeemed;
  bool double_spent;
};

// Takes as input AnonymousTokensRedemptionResponse and uses that to create a
// map of token to their respective RedemptionResult for testing purposes.
absl::flat_hash_map<std::string, RedemptionInfoAndResult>
CreateTokenToRedemptionResultMap(
    const AnonymousTokensRedemptionResponse& response) {
  absl::flat_hash_map<std::string, RedemptionInfoAndResult> response_map;
  for (const auto& result : response.anonymous_token_redemption_results()) {
    response_map[result.serialized_unblinded_token()] = {
        .plaintext_message = result.plaintext_message(),
        .public_metadata = result.public_metadata(),
        .message_mask = result.message_mask(),
        .redeemed = result.verified(),
        .double_spent = result.double_spent(),
    };
  }
  return response_map;
}

class AnonymousTokensRedemptionClientTest : public testing::Test {
 protected:
  void SetUp() override {
    ANON_TOKENS_ASSERT_OK_AND_ASSIGN(
        client_, AnonymousTokensRedemptionClient::Create(TEST_USE_CASE, 1));
    dummy_token_with_input_ = GetRandomDummyTokenWithInput();
    *(dummy_response_.add_anonymous_token_redemption_results()) =
        CreateRedemptionResultForTesting(dummy_token_with_input_);
    generator_.seed(GTEST_FLAG_GET(random_seed));
  }

  // Generates a dummy RSABlindSignatureTokenWithInput which is not
  // cryptographically valid as that is not needed for testing purposes.
  RSABlindSignatureTokenWithInput GetRandomDummyTokenWithInput() {
    PlaintextMessageWithPublicMetadata input;
    input.set_plaintext_message(GetRandomString(20, &distr_u8_, &generator_));
    input.set_public_metadata(GetRandomString(10, &distr_u8_, &generator_));
    RSABlindSignatureToken token;
    token.set_token(GetRandomString(512, &distr_u8_, &generator_));
    token.set_message_mask(GetRandomString(32, &distr_u8_, &generator_));
    RSABlindSignatureTokenWithInput token_with_input;
    *token_with_input.mutable_input() = input;
    *token_with_input.mutable_token() = token;
    return token_with_input;
  }

  // Creates a fake token redemption response for one
  // RSABlindSignatureTokenWithInput and outputs it as
  // AnonymousTokenRedemptionResult
  AnonymousTokensRedemptionResponse_AnonymousTokenRedemptionResult
  CreateRedemptionResultForTesting(
      RSABlindSignatureTokenWithInput token_with_input, bool verified = true,
      bool double_spent = false,
      AnonymousTokensUseCase use_case = TEST_USE_CASE,
      int64_t key_version = 1) {
    AnonymousTokensRedemptionResponse_AnonymousTokenRedemptionResult result;
    result.set_use_case(AnonymousTokensUseCase_Name(use_case));
    result.set_key_version(key_version);
    result.set_public_metadata(token_with_input.input().public_metadata());
    result.set_serialized_unblinded_token(token_with_input.token().token());
    result.set_plaintext_message(token_with_input.input().plaintext_message());
    result.set_message_mask(token_with_input.token().message_mask());
    result.set_verified(verified);
    result.set_double_spent(double_spent);
    return result;
  }

  std::mt19937_64 generator_;
  std::uniform_int_distribution<int> distr_u8_ =
      std::uniform_int_distribution<int>{0, 255};

  std::unique_ptr<AnonymousTokensRedemptionClient> client_;
  RSABlindSignatureTokenWithInput dummy_token_with_input_;
  AnonymousTokensRedemptionResponse dummy_response_;
};

TEST_F(AnonymousTokensRedemptionClientTest, UndefinedUseCase) {
  // Use case undefined.
  absl::StatusOr<std::unique_ptr<AnonymousTokensRedemptionClient>>
      redemption_client = AnonymousTokensRedemptionClient::Create(
          ANONYMOUS_TOKENS_USE_CASE_UNDEFINED, 1);
  EXPECT_EQ(redemption_client.status().code(),
            absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(redemption_client.status().message(),
              HasSubstr("must be defined"));
}

TEST_F(AnonymousTokensRedemptionClientTest, InvalidKeyVersions) {
  // Key version 0.
  absl::StatusOr<std::unique_ptr<AnonymousTokensRedemptionClient>>
      redemption_client_1 =
          AnonymousTokensRedemptionClient::Create(TEST_USE_CASE, 0);
  EXPECT_EQ(redemption_client_1.status().code(),
            absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(redemption_client_1.status().message(),
              HasSubstr("must be greater than 0"));
}

TEST_F(AnonymousTokensRedemptionClientTest, EmptyRequest) {
  absl::StatusOr<AnonymousTokensRedemptionRequest> redemption_request =
      client_->CreateAnonymousTokensRedemptionRequest({});
  EXPECT_EQ(redemption_request.status().code(),
            absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(redemption_request.status().message(),
              HasSubstr("empty request"));
}

TEST_F(AnonymousTokensRedemptionClientTest, CreatingRequestAgain) {
  ANON_TOKENS_ASSERT_OK_AND_ASSIGN(
      auto _, client_->CreateAnonymousTokensRedemptionRequest(
                  {dummy_token_with_input_}));
  // Creating same request again with same client.
  absl::StatusOr<AnonymousTokensRedemptionRequest> redemption_request_2 =
      client_->CreateAnonymousTokensRedemptionRequest(
          {dummy_token_with_input_});
  EXPECT_EQ(redemption_request_2.status().code(),
            absl::StatusCode::kFailedPrecondition);
  EXPECT_THAT(redemption_request_2.status().message(),
              HasSubstr("already created"));
  // Creating different request with the same client.
  absl::StatusOr<AnonymousTokensRedemptionRequest> redemption_request_3 =
      client_->CreateAnonymousTokensRedemptionRequest(
          {GetRandomDummyTokenWithInput()});
  EXPECT_EQ(redemption_request_3.status().code(),
            absl::StatusCode::kFailedPrecondition);
  EXPECT_THAT(redemption_request_3.status().message(),
              HasSubstr("already created"));
}

TEST_F(AnonymousTokensRedemptionClientTest, MissingTokenInRequest) {
  dummy_token_with_input_.mutable_token()->clear_token();
  absl::StatusOr<AnonymousTokensRedemptionRequest> redemption_request =
      client_->CreateAnonymousTokensRedemptionRequest(
          {dummy_token_with_input_});
  EXPECT_EQ(redemption_request.status().code(),
            absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(redemption_request.status().message(), HasSubstr("empty token"));
}

TEST_F(AnonymousTokensRedemptionClientTest, WrongMaskSize) {
  dummy_token_with_input_.mutable_token()->set_message_mask("wrongmasksize");
  absl::StatusOr<AnonymousTokensRedemptionRequest> redemption_request =
      client_->CreateAnonymousTokensRedemptionRequest(
          {dummy_token_with_input_});
  EXPECT_EQ(redemption_request.status().code(),
            absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(redemption_request.status().message(),
              HasSubstr("at least 32 bytes"));
}

TEST_F(AnonymousTokensRedemptionClientTest, RepeatedTokenInRequest) {
  absl::StatusOr<AnonymousTokensRedemptionRequest> redemption_request =
      client_->CreateAnonymousTokensRedemptionRequest(
          {dummy_token_with_input_, dummy_token_with_input_});
  EXPECT_EQ(redemption_request.status().code(),
            absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(redemption_request.status().message(),
              HasSubstr("should not be repeated"));
}

TEST_F(AnonymousTokensRedemptionClientTest, ProcessBeforeRequestCreation) {
  AnonymousTokensRedemptionResponse redemption_resp;
  absl::StatusOr<std::vector<RSABlindSignatureRedemptionResult>>
      redemption_result =
          client_->ProcessAnonymousTokensRedemptionResponse(redemption_resp);
  EXPECT_EQ(redemption_result.status().code(),
            absl::StatusCode::kFailedPrecondition);
  EXPECT_THAT(redemption_result.status().message(),
              HasSubstr("request was not created"));
}

TEST_F(AnonymousTokensRedemptionClientTest, EmptyResponseProcessing) {
  ANON_TOKENS_ASSERT_OK_AND_ASSIGN(
      auto _, client_->CreateAnonymousTokensRedemptionRequest(
                  {dummy_token_with_input_}));
  AnonymousTokensRedemptionResponse redemption_resp;
  absl::StatusOr<std::vector<RSABlindSignatureRedemptionResult>>
      redemption_result =
          client_->ProcessAnonymousTokensRedemptionResponse(redemption_resp);
  EXPECT_EQ(redemption_result.status().code(),
            absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(redemption_result.status().message(),
              HasSubstr("empty response"));
}

TEST_F(AnonymousTokensRedemptionClientTest, WrongSizeOfResponse) {
  ANON_TOKENS_ASSERT_OK_AND_ASSIGN(
      auto _, client_->CreateAnonymousTokensRedemptionRequest(
                  {dummy_token_with_input_, GetRandomDummyTokenWithInput()}));
  absl::StatusOr<std::vector<RSABlindSignatureRedemptionResult>>
      redemption_result =
          client_->ProcessAnonymousTokensRedemptionResponse(dummy_response_);
  EXPECT_EQ(redemption_result.status().code(),
            absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(redemption_result.status().message(),
              HasSubstr("missing some requested token redemptions"));
}

TEST_F(AnonymousTokensRedemptionClientTest, UseCaseMismatch) {
  ANON_TOKENS_ASSERT_OK_AND_ASSIGN(
      auto _, client_->CreateAnonymousTokensRedemptionRequest(
                  {dummy_token_with_input_}));
  dummy_response_.mutable_anonymous_token_redemption_results(0)->set_use_case(
      AnonymousTokensUseCase_Name(TEST_USE_CASE_2));
  absl::StatusOr<std::vector<RSABlindSignatureRedemptionResult>>
      redemption_result =
          client_->ProcessAnonymousTokensRedemptionResponse(dummy_response_);
  EXPECT_EQ(redemption_result.status().code(),
            absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(redemption_result.status().message(),
              HasSubstr("Use case does not match"));
}

TEST_F(AnonymousTokensRedemptionClientTest, KeyVersionMismatch) {
  ANON_TOKENS_ASSERT_OK_AND_ASSIGN(
      auto _, client_->CreateAnonymousTokensRedemptionRequest(
                  {dummy_token_with_input_}));
  dummy_response_.mutable_anonymous_token_redemption_results(0)
      ->set_key_version(2);
  absl::StatusOr<std::vector<RSABlindSignatureRedemptionResult>>
      redemption_result =
          client_->ProcessAnonymousTokensRedemptionResponse(dummy_response_);
  EXPECT_EQ(redemption_result.status().code(),
            absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(redemption_result.status().message(),
              HasSubstr("Key version does not match"));
}

TEST_F(AnonymousTokensRedemptionClientTest, EmptyTokenInResponse) {
  ANON_TOKENS_ASSERT_OK_AND_ASSIGN(
      auto _, client_->CreateAnonymousTokensRedemptionRequest(
                  {dummy_token_with_input_}));
  dummy_response_.mutable_anonymous_token_redemption_results(0)
      ->clear_serialized_unblinded_token();
  absl::StatusOr<std::vector<RSABlindSignatureRedemptionResult>>
      redemption_result =
          client_->ProcessAnonymousTokensRedemptionResponse(dummy_response_);
  EXPECT_EQ(redemption_result.status().code(),
            absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(redemption_result.status().message(),
              HasSubstr("Token cannot be empty"));
}

TEST_F(AnonymousTokensRedemptionClientTest, MissingMaskInResponse) {
  ANON_TOKENS_ASSERT_OK_AND_ASSIGN(
      auto _, client_->CreateAnonymousTokensRedemptionRequest(
                  {dummy_token_with_input_}));
  dummy_response_.mutable_anonymous_token_redemption_results(0)
      ->clear_message_mask();
  absl::StatusOr<std::vector<RSABlindSignatureRedemptionResult>>
      redemption_result =
          client_->ProcessAnonymousTokensRedemptionResponse(dummy_response_);
  EXPECT_EQ(redemption_result.status().code(),
            absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(
      redemption_result.status().message(),
      HasSubstr("Response message mask does not match input message mask"));
}

TEST_F(AnonymousTokensRedemptionClientTest, WrongMaskSizeInResponse) {
  ANON_TOKENS_ASSERT_OK_AND_ASSIGN(
      auto _, client_->CreateAnonymousTokensRedemptionRequest(
                  {dummy_token_with_input_}));
  dummy_response_.mutable_anonymous_token_redemption_results(0)
      ->set_message_mask(GetRandomString(31, &distr_u8_, &generator_));
  absl::StatusOr<std::vector<RSABlindSignatureRedemptionResult>>
      redemption_result =
          client_->ProcessAnonymousTokensRedemptionResponse(dummy_response_);
  EXPECT_EQ(redemption_result.status().code(),
            absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(redemption_result.status().message(),
              HasSubstr("at least 32 bytes"));
}

TEST_F(AnonymousTokensRedemptionClientTest, RepeatedTokenInResponse) {
  auto another_dummy_token_with_input = GetRandomDummyTokenWithInput();
  ANON_TOKENS_ASSERT_OK_AND_ASSIGN(
      auto _, client_->CreateAnonymousTokensRedemptionRequest(
                  {dummy_token_with_input_, another_dummy_token_with_input}));
  another_dummy_token_with_input.mutable_token()->set_token(
      dummy_token_with_input_.token().token());
  *(dummy_response_.add_anonymous_token_redemption_results()) =
      CreateRedemptionResultForTesting(another_dummy_token_with_input);
  absl::StatusOr<std::vector<RSABlindSignatureRedemptionResult>>
      redemption_result =
          client_->ProcessAnonymousTokensRedemptionResponse(dummy_response_);
  EXPECT_EQ(redemption_result.status().code(),
            absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(redemption_result.status().message(),
              HasSubstr("Token was repeated"));
}

TEST_F(AnonymousTokensRedemptionClientTest, NewTokenInResponse) {
  ANON_TOKENS_ASSERT_OK_AND_ASSIGN(
      auto _, client_->CreateAnonymousTokensRedemptionRequest(
                  {dummy_token_with_input_}));
  dummy_response_.mutable_anonymous_token_redemption_results(0)
      ->set_serialized_unblinded_token(
          GetRandomString(512, &distr_u8_, &generator_));
  absl::StatusOr<std::vector<RSABlindSignatureRedemptionResult>>
      redemption_result =
          client_->ProcessAnonymousTokensRedemptionResponse(dummy_response_);
  EXPECT_EQ(redemption_result.status().code(),
            absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(redemption_result.status().message(),
              HasSubstr("tokens whose redemptions were not requested"));
}

TEST_F(AnonymousTokensRedemptionClientTest, PublicMetadataMismatch) {
  ANON_TOKENS_ASSERT_OK_AND_ASSIGN(
      auto _, client_->CreateAnonymousTokensRedemptionRequest(
                  {dummy_token_with_input_}));
  dummy_response_.mutable_anonymous_token_redemption_results(0)
      ->set_public_metadata(GetRandomString(10, &distr_u8_, &generator_));
  absl::StatusOr<std::vector<RSABlindSignatureRedemptionResult>>
      redemption_result =
          client_->ProcessAnonymousTokensRedemptionResponse(dummy_response_);
  EXPECT_EQ(redemption_result.status().code(),
            absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(redemption_result.status().message(),
              HasSubstr("Response metadata does not match"));
}

TEST_F(AnonymousTokensRedemptionClientTest, PlaintextMessageMismatch) {
  ANON_TOKENS_ASSERT_OK_AND_ASSIGN(
      auto _, client_->CreateAnonymousTokensRedemptionRequest(
                  {dummy_token_with_input_}));
  dummy_response_.mutable_anonymous_token_redemption_results(0)
      ->set_plaintext_message(GetRandomString(20, &distr_u8_, &generator_));
  absl::StatusOr<std::vector<RSABlindSignatureRedemptionResult>>
      redemption_result =
          client_->ProcessAnonymousTokensRedemptionResponse(dummy_response_);
  EXPECT_EQ(redemption_result.status().code(),
            absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(redemption_result.status().message(),
              HasSubstr("Response plaintext message does not match"));
}

TEST_F(AnonymousTokensRedemptionClientTest, MessageMaskMismatch) {
  ANON_TOKENS_ASSERT_OK_AND_ASSIGN(
      auto _, client_->CreateAnonymousTokensRedemptionRequest(
                  {dummy_token_with_input_}));
  dummy_response_.mutable_anonymous_token_redemption_results(0)
      ->set_message_mask(GetRandomString(32, &distr_u8_, &generator_));
  absl::StatusOr<std::vector<RSABlindSignatureRedemptionResult>>
      redemption_result =
          client_->ProcessAnonymousTokensRedemptionResponse(dummy_response_);
  EXPECT_EQ(redemption_result.status().code(),
            absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(redemption_result.status().message(),
              HasSubstr("Response message mask does not match"));
}

TEST_F(AnonymousTokensRedemptionClientTest,
       SuccessfulResponseProcessingWithOneToken) {
  // Only one token in request
  ANON_TOKENS_ASSERT_OK_AND_ASSIGN(
      auto redemption_request, client_->CreateAnonymousTokensRedemptionRequest(
                                   {dummy_token_with_input_}));
  ANON_TOKENS_ASSERT_OK_AND_ASSIGN(
      auto rsa_blind_sig_redemption_results,
      client_->ProcessAnonymousTokensRedemptionResponse(dummy_response_));
  auto tokens_to_result_map = CreateTokenToRedemptionResultMap(dummy_response_);
  // Checks
  ASSERT_EQ(rsa_blind_sig_redemption_results.size(), 1u);
  std::string token =
      rsa_blind_sig_redemption_results[0].token_with_input().token().token();
  ASSERT_TRUE(tokens_to_result_map.contains(token));
  EXPECT_EQ(rsa_blind_sig_redemption_results[0]
                .token_with_input()
                .input()
                .plaintext_message(),
            tokens_to_result_map[token].plaintext_message);
  EXPECT_TRUE(!rsa_blind_sig_redemption_results[0]
                   .token_with_input()
                   .token()
                   .message_mask()
                   .empty());
  EXPECT_EQ(rsa_blind_sig_redemption_results[0]
                .token_with_input()
                .input()
                .public_metadata(),
            tokens_to_result_map[token].public_metadata);
  EXPECT_EQ(rsa_blind_sig_redemption_results[0].redeemed(),
            tokens_to_result_map[token].redeemed);
  EXPECT_EQ(rsa_blind_sig_redemption_results[0].double_spent(),
            tokens_to_result_map[token].double_spent);
}

TEST_F(AnonymousTokensRedemptionClientTest,
       SuccessfulResponseProcessingWithMultipleToken) {
  RSABlindSignatureTokenWithInput token_with_empty_message =
      GetRandomDummyTokenWithInput();
  token_with_empty_message.mutable_input()->clear_plaintext_message();
  RSABlindSignatureTokenWithInput token_with_empty_mask =
      GetRandomDummyTokenWithInput();
  token_with_empty_mask.mutable_token()->clear_message_mask();
  std::vector<RSABlindSignatureTokenWithInput> tokens_with_inputs = {
      dummy_token_with_input_, GetRandomDummyTokenWithInput(),
      GetRandomDummyTokenWithInput(), token_with_empty_message,
      token_with_empty_mask};
  ANON_TOKENS_ASSERT_OK_AND_ASSIGN(
      auto _,
      client_->CreateAnonymousTokensRedemptionRequest(tokens_with_inputs));
  *(dummy_response_.add_anonymous_token_redemption_results()) =
      CreateRedemptionResultForTesting(tokens_with_inputs[1], false, true,
                                       TEST_USE_CASE, 1);
  for (size_t i = 2; i < tokens_with_inputs.size(); ++i) {
    *(dummy_response_.add_anonymous_token_redemption_results()) =
        CreateRedemptionResultForTesting(tokens_with_inputs[i]);
  }
  ANON_TOKENS_ASSERT_OK_AND_ASSIGN(
      auto rsa_blind_sig_redemption_results,
      client_->ProcessAnonymousTokensRedemptionResponse(dummy_response_));
  auto tokens_to_result_map = CreateTokenToRedemptionResultMap(dummy_response_);
  // Checks
  ASSERT_EQ(tokens_with_inputs.size(), rsa_blind_sig_redemption_results.size());
  for (size_t i = 0; i < rsa_blind_sig_redemption_results.size(); ++i) {
    std::string token =
        rsa_blind_sig_redemption_results[i].token_with_input().token().token();
    ASSERT_TRUE(tokens_to_result_map.contains(token));
    EXPECT_EQ(rsa_blind_sig_redemption_results[i]
                  .token_with_input()
                  .input()
                  .plaintext_message(),
              tokens_to_result_map[token].plaintext_message);
    EXPECT_EQ(rsa_blind_sig_redemption_results[i]
                  .token_with_input()
                  .token()
                  .message_mask(),
              tokens_to_result_map[token].message_mask);
    EXPECT_EQ(rsa_blind_sig_redemption_results[i]
                  .token_with_input()
                  .input()
                  .public_metadata(),
              tokens_to_result_map[token].public_metadata);
    EXPECT_EQ(rsa_blind_sig_redemption_results[i].redeemed(),
              tokens_to_result_map[token].redeemed);
    EXPECT_EQ(rsa_blind_sig_redemption_results[i].double_spent(),
              tokens_to_result_map[token].double_spent);
  }
}

}  // namespace
}  // namespace anonymous_tokens
