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

#include "anonymous_tokens/cpp/client/anonymous_tokens_public_key_client.h"

#include <cstddef>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include "absl/status/status.h"
#include "absl/strings/string_view.h"
#include "anonymous_tokens/cpp/shared/proto_utils.h"
#include "anonymous_tokens/cpp/shared/status_utils.h"
#include "anonymous_tokens/cpp/testing/utils.h"
#include "anonymous_tokens/proto/anonymous_tokens.pb.h"

namespace anonymous_tokens {
namespace {

constexpr int kKeyByteSize = 512;
constexpr int kSaltByteLength = 48;
constexpr int kMessageMaskByteLength = 32;
constexpr absl::Duration kEndTimeIncrement = absl::Minutes(100);
constexpr absl::Duration kStartTimeIncrement = absl::Minutes(35);

class AnonymousTokensPublicKeysGetClientTest : public ::testing::Test {
 protected:
  void SetUp() override {
    start_time_ = absl::Now();
    default_n_str_ = std::string(kKeyByteSize, 'a');
    default_e_str_ = std::string(kKeyByteSize, 'b');

    ANON_TOKENS_ASSERT_OK_AND_ASSIGN(
        client_, AnonymousTokensPublicKeysGetClient::Create());
  }

  RSAPublicKey GenerateFakeRsaKey(std::string n_str, std::string e_str) {
    RSAPublicKey public_key;
    public_key.set_n(n_str);
    public_key.set_e(e_str);
    return public_key;
  }

  absl::StatusOr<AnonymousTokensPublicKeysGetResponse> SimpleGetResponse() {
    RSAPublicKey public_key =
        GenerateFakeRsaKey(default_n_str_, default_e_str_);
    return PublicKeysGetResponse({public_key});
  }

  absl::StatusOr<AnonymousTokensPublicKeysGetResponse> PublicKeysGetResponse(
      const std::vector<RSAPublicKey>& public_keys) {
    AnonymousTokensPublicKeysGetResponse resp;
    for (size_t i = 0; i < public_keys.size(); ++i) {
      RSABlindSignaturePublicKey* key = resp.add_rsa_public_keys();
      key->set_use_case("TEST_USE_CASE");
      key->set_key_version(i + 1);
      key->set_serialized_public_key(public_keys[i].SerializeAsString());
      ANON_TOKENS_ASSIGN_OR_RETURN(
          *(key->mutable_key_validity_start_time()),
          TimeToProto(start_time_ - (kStartTimeIncrement * i)));
      ANON_TOKENS_ASSIGN_OR_RETURN(
          *(key->mutable_expiration_time()),
          TimeToProto(start_time_ + (kEndTimeIncrement * (i + 1))));
      key->set_mask_gen_function(AT_MGF_SHA384);
      key->set_sig_hash_type(AT_HASH_TYPE_SHA384);
      key->set_key_size(kKeyByteSize);
      key->set_salt_length(kSaltByteLength);
      key->set_message_mask_type(AT_MESSAGE_MASK_CONCAT);
      key->set_message_mask_size(kMessageMaskByteLength);
      key->set_public_metadata_support(true);
    }
    return resp;
  }

  absl::Time start_time_;
  std::string default_n_str_;
  std::string default_e_str_;
  std::unique_ptr<AnonymousTokensPublicKeysGetClient> client_;
};

TEST_F(AnonymousTokensPublicKeysGetClientTest,
       PublicKeyGetClientMoreThanOneRequest) {
  // Create the first request.
  ASSERT_TRUE(client_
                  ->CreateAnonymousTokensPublicKeysGetRequest(
                      TEST_USE_CASE, 1, absl::Now(), absl::nullopt)
                  .ok());
  // Second request will err.
  absl::StatusOr<AnonymousTokensPublicKeysGetRequest> request =
      client_->CreateAnonymousTokensPublicKeysGetRequest(
          TEST_USE_CASE, 1, absl::Now(), absl::nullopt);
  EXPECT_EQ(request.status().code(), absl::StatusCode::kFailedPrecondition);
  EXPECT_THAT(request.status().message(),
              testing::HasSubstr("Public Key request is already created."));
}

TEST_F(AnonymousTokensPublicKeysGetClientTest,
       PublicKeyGetClientInvalidUseCaseRequest) {
  absl::StatusOr<AnonymousTokensPublicKeysGetRequest> request =
      client_->CreateAnonymousTokensPublicKeysGetRequest(
          ANONYMOUS_TOKENS_USE_CASE_UNDEFINED, 0, absl::Now(), absl::nullopt);
  EXPECT_EQ(request.status().code(), absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(request.status().message(),
              testing::HasSubstr("Use case must be defined."));
}

TEST_F(AnonymousTokensPublicKeysGetClientTest,
       PublicKeyGetClientInvalidKeyVersionRequest) {
  absl::StatusOr<AnonymousTokensPublicKeysGetRequest> request =
      client_->CreateAnonymousTokensPublicKeysGetRequest(
          TEST_USE_CASE, -1, absl::Now(), absl::nullopt);
  EXPECT_EQ(request.status().code(), absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(
      request.status().message(),
      testing::HasSubstr("Key Version in an AnonymousTokensPublicKeysGetRequest"
                         " must be 0 or greater than 0."));
}

TEST_F(AnonymousTokensPublicKeysGetClientTest,
       PublicKeyGetClientExpiryBeforeValidityStartRequest) {
  absl::StatusOr<AnonymousTokensPublicKeysGetRequest> request =
      client_->CreateAnonymousTokensPublicKeysGetRequest(
          TEST_USE_CASE, 0, absl::Now(), absl::Now() - absl::Minutes(10));
  EXPECT_EQ(request.status().code(), absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(
      request.status().message(),
      testing::HasSubstr("Key validity start time can not be the same or after"
                         " key validity end time (if set)."));
}

TEST_F(AnonymousTokensPublicKeysGetClientTest,
       PublicKeyGetClientExpiryInThePastRequest) {
  absl::StatusOr<AnonymousTokensPublicKeysGetRequest> request =
      client_->CreateAnonymousTokensPublicKeysGetRequest(
          TEST_USE_CASE, 0, absl::Now() - absl::Minutes(30),
          absl::Now() - absl::Minutes(10));
  EXPECT_EQ(request.status().code(), absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(
      request.status().message(),
      testing::HasSubstr(
          "Requested Key expiry time (if set) must not be in the past."));
}

TEST_F(AnonymousTokensPublicKeysGetClientTest,
       CreateAnonymousTokensPublicKeysGetRequestWithExpiryTime) {
  absl::Time end_time = start_time_ + kEndTimeIncrement;
  ANON_TOKENS_ASSERT_OK_AND_ASSIGN(
      AnonymousTokensPublicKeysGetRequest request,
      client_->CreateAnonymousTokensPublicKeysGetRequest(
          TEST_USE_CASE, 0, start_time_, end_time));

  EXPECT_EQ(request.use_case(), AnonymousTokensUseCase_Name(TEST_USE_CASE));
  EXPECT_EQ(request.key_version(), 0u);
  ANON_TOKENS_ASSERT_OK_AND_ASSIGN(
      absl::Time request_start_time,
      TimeFromProto(request.key_validity_start_time()));
  EXPECT_EQ(request_start_time, start_time_);
  ANON_TOKENS_ASSERT_OK_AND_ASSIGN(
      absl::Time request_end_time,
      TimeFromProto(request.key_validity_end_time()));
  EXPECT_EQ(request_end_time, end_time);
}

TEST_F(AnonymousTokensPublicKeysGetClientTest,
       CreateAnonymousTokensPublicKeysGetRequestWithNoExpiryTime) {
  ANON_TOKENS_ASSERT_OK_AND_ASSIGN(
      auto request, client_->CreateAnonymousTokensPublicKeysGetRequest(
                        TEST_USE_CASE, 0, start_time_, absl::nullopt));

  EXPECT_EQ(request.use_case(), AnonymousTokensUseCase_Name(TEST_USE_CASE));
  EXPECT_EQ(request.key_version(), 0u);
  EXPECT_EQ(TimeFromProto(request.key_validity_start_time()).value(),
            start_time_);
  EXPECT_EQ(request.has_key_validity_end_time(), false);
}

TEST_F(AnonymousTokensPublicKeysGetClientTest,
       ProcessPublicKeyGetResponseWithoutCreatingRequest) {
  AnonymousTokensPublicKeysGetResponse response;
  absl::StatusOr<std::vector<RSABlindSignaturePublicKey>> public_keys =
      client_->ProcessAnonymousTokensRSAPublicKeysGetResponse(response);
  EXPECT_EQ(public_keys.status().code(), absl::StatusCode::kFailedPrecondition);
  EXPECT_THAT(public_keys.status().message(),
              testing::HasSubstr(
                  "CreateAnonymousTokensPublicKeysGetRequest has not been "
                  "called yet."));
}

TEST_F(AnonymousTokensPublicKeysGetClientTest,
       UndefinedUseCaseInPublicKeyGetResponse) {
  ASSERT_TRUE(
      client_
          ->CreateAnonymousTokensPublicKeysGetRequest(
              TEST_USE_CASE, 0, start_time_, start_time_ + absl::Minutes(100))
          .ok());
  ANON_TOKENS_ASSERT_OK_AND_ASSIGN(auto response, SimpleGetResponse());

  // Invalid use case.
  response.mutable_rsa_public_keys(0)->set_use_case("invalid");

  absl::StatusOr<std::vector<RSABlindSignaturePublicKey>> public_keys =
      client_->ProcessAnonymousTokensRSAPublicKeysGetResponse(response);
  EXPECT_EQ(public_keys.status().code(), absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(
      public_keys.status().message(),
      testing::HasSubstr("Invalid / undefined use case cannot be parsed."));
}

TEST_F(AnonymousTokensPublicKeysGetClientTest,
       UseCaseInPublicKeyGetResponseDifferentThanRequest) {
  ASSERT_TRUE(
      client_
          ->CreateAnonymousTokensPublicKeysGetRequest(
              TEST_USE_CASE, 0, start_time_, start_time_ + absl::Minutes(100))
          .ok());
  ANON_TOKENS_ASSERT_OK_AND_ASSIGN(auto response, SimpleGetResponse());

  // Receiving a different use case response is invalid.
  response.mutable_rsa_public_keys(0)->set_use_case("TEST_USE_CASE_2");

  absl::StatusOr<std::vector<RSABlindSignaturePublicKey>> public_keys =
      client_->ProcessAnonymousTokensRSAPublicKeysGetResponse(response);
  EXPECT_EQ(public_keys.status().code(), absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(
      public_keys.status().message(),
      testing::HasSubstr("Public key is not for the Use Case requested."));
}

TEST_F(AnonymousTokensPublicKeysGetClientTest,
       KeyVersionZeroInPublicKeyGetResponse) {
  ASSERT_TRUE(
      client_
          ->CreateAnonymousTokensPublicKeysGetRequest(
              TEST_USE_CASE, 0, start_time_, start_time_ + absl::Minutes(100))
          .ok());
  ANON_TOKENS_ASSERT_OK_AND_ASSIGN(auto response, SimpleGetResponse());

  // Key version cannot be 0.
  response.mutable_rsa_public_keys(0)->set_key_version(0);

  absl::StatusOr<std::vector<RSABlindSignaturePublicKey>> public_keys =
      client_->ProcessAnonymousTokensRSAPublicKeysGetResponse(response);
  EXPECT_EQ(public_keys.status().code(), absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(public_keys.status().message(),
              testing::HasSubstr("Key_version cannot be zero or negative."));
}

TEST_F(AnonymousTokensPublicKeysGetClientTest,
       KeyVersionDifferentThanRequestedInPublicKeyGetResponse) {
  ASSERT_TRUE(
      client_
          ->CreateAnonymousTokensPublicKeysGetRequest(
              TEST_USE_CASE, 2, start_time_, start_time_ + absl::Minutes(100))
          .ok());
  ANON_TOKENS_ASSERT_OK_AND_ASSIGN(auto response, SimpleGetResponse());

  absl::StatusOr<std::vector<RSABlindSignaturePublicKey>> public_keys =
      client_->ProcessAnonymousTokensRSAPublicKeysGetResponse(response);
  EXPECT_EQ(public_keys.status().code(), absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(
      public_keys.status().message(),
      testing::HasSubstr("Public key is not for the Key Version requested."));
}

TEST_F(AnonymousTokensPublicKeysGetClientTest,
       SaltLengthZeroInPublicKeyGetResponse) {
  ASSERT_TRUE(
      client_
          ->CreateAnonymousTokensPublicKeysGetRequest(
              TEST_USE_CASE, 0, start_time_, start_time_ + absl::Minutes(100))
          .ok());
  ANON_TOKENS_ASSERT_OK_AND_ASSIGN(auto response, SimpleGetResponse());

  // Do not allow deterministic signatures due to empty salts.
  response.mutable_rsa_public_keys(0)->set_salt_length(0);

  absl::StatusOr<std::vector<RSABlindSignaturePublicKey>> public_keys =
      client_->ProcessAnonymousTokensRSAPublicKeysGetResponse(response);
  EXPECT_EQ(public_keys.status().code(), absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(public_keys.status().message(),
              testing::HasSubstr("Salt length must not be zero or negative."));
}

TEST_F(AnonymousTokensPublicKeysGetClientTest,
       SaltLengthNegativeInPublicKeyGetResponse) {
  ASSERT_TRUE(
      client_
          ->CreateAnonymousTokensPublicKeysGetRequest(
              TEST_USE_CASE, 0, start_time_, start_time_ + absl::Minutes(100))
          .ok());
  ANON_TOKENS_ASSERT_OK_AND_ASSIGN(auto response, SimpleGetResponse());

  // Negative salt length is invalid.
  response.mutable_rsa_public_keys(0)->set_salt_length(-10);

  absl::StatusOr<std::vector<RSABlindSignaturePublicKey>> public_keys =
      client_->ProcessAnonymousTokensRSAPublicKeysGetResponse(response);
  EXPECT_EQ(public_keys.status().code(), absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(public_keys.status().message(),
              testing::HasSubstr("Salt length must not be zero or negative."));
}

TEST_F(AnonymousTokensPublicKeysGetClientTest,
       InvalidKeySizeInPublicKeyGetResponse) {
  ASSERT_TRUE(
      client_
          ->CreateAnonymousTokensPublicKeysGetRequest(
              TEST_USE_CASE, 0, start_time_, start_time_ + absl::Minutes(100))
          .ok());
  ANON_TOKENS_ASSERT_OK_AND_ASSIGN(auto response, SimpleGetResponse());

  // Too short keys are invalid.
  response.mutable_rsa_public_keys(0)->set_key_size(128);

  absl::StatusOr<std::vector<RSABlindSignaturePublicKey>> public_keys =
      client_->ProcessAnonymousTokensRSAPublicKeysGetResponse(response);
  EXPECT_EQ(public_keys.status().code(), absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(public_keys.status().message(),
              testing::HasSubstr("Key_size cannot be less than 256 bytes."));
}

TEST_F(AnonymousTokensPublicKeysGetClientTest,
       NegativeKeySizeInPublicKeyGetResponse) {
  ASSERT_TRUE(
      client_
          ->CreateAnonymousTokensPublicKeysGetRequest(
              TEST_USE_CASE, 0, start_time_, start_time_ + absl::Minutes(100))
          .ok());
  ANON_TOKENS_ASSERT_OK_AND_ASSIGN(auto response, SimpleGetResponse());

  // Negative key size is invalid.
  response.mutable_rsa_public_keys(0)->set_key_size(-10);

  absl::StatusOr<std::vector<RSABlindSignaturePublicKey>> public_keys =
      client_->ProcessAnonymousTokensRSAPublicKeysGetResponse(response);
  EXPECT_EQ(public_keys.status().code(), absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(public_keys.status().message(),
              testing::HasSubstr("Key_size cannot be less than 256 bytes."));
}

TEST_F(AnonymousTokensPublicKeysGetClientTest,
       UndefinedOrUnsupportedMessageMaskType) {
  ASSERT_TRUE(
      client_
          ->CreateAnonymousTokensPublicKeysGetRequest(
              TEST_USE_CASE, 0, start_time_, start_time_ + absl::Minutes(100))
          .ok());
  ANON_TOKENS_ASSERT_OK_AND_ASSIGN(auto response, SimpleGetResponse());

  // Undefined message mask type is invalid.
  response.mutable_rsa_public_keys(0)->set_message_mask_type(
      AT_MESSAGE_MASK_TYPE_UNDEFINED);

  absl::StatusOr<std::vector<RSABlindSignaturePublicKey>> public_keys =
      client_->ProcessAnonymousTokensRSAPublicKeysGetResponse(response);
  EXPECT_EQ(public_keys.status().code(), absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(
      public_keys.status().message(),
      testing::HasSubstr("Message mask type must be defined and supported."));

  // Unsupported message mask type is invalid.
  response.mutable_rsa_public_keys(0)->set_message_mask_type(
      AT_MESSAGE_MASK_XOR);

  public_keys =
      client_->ProcessAnonymousTokensRSAPublicKeysGetResponse(response);
  EXPECT_EQ(public_keys.status().code(), absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(
      public_keys.status().message(),
      testing::HasSubstr("Message mask type must be defined and supported."));
}

TEST_F(AnonymousTokensPublicKeysGetClientTest,
       MessageMaskConcatSizeLessThan32) {
  ASSERT_TRUE(
      client_
          ->CreateAnonymousTokensPublicKeysGetRequest(
              TEST_USE_CASE, 0, start_time_, start_time_ + absl::Minutes(100))
          .ok());
  ANON_TOKENS_ASSERT_OK_AND_ASSIGN(auto response, SimpleGetResponse());

  // Message mask must be at least 32 bytes.
  response.mutable_rsa_public_keys(0)->set_message_mask_size(10);

  absl::StatusOr<std::vector<RSABlindSignaturePublicKey>> public_keys =
      client_->ProcessAnonymousTokensRSAPublicKeysGetResponse(response);
  EXPECT_EQ(public_keys.status().code(), absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(
      public_keys.status().message(),
      testing::HasSubstr(
          "Message mask concat type must have a size of at least 32 bytes."));
}

TEST_F(AnonymousTokensPublicKeysGetClientTest, MessageMaskNoMaskSizeNotZero) {
  ASSERT_TRUE(
      client_
          ->CreateAnonymousTokensPublicKeysGetRequest(
              TEST_USE_CASE, 0, start_time_, start_time_ + absl::Minutes(100))
          .ok());
  ANON_TOKENS_ASSERT_OK_AND_ASSIGN(auto response, SimpleGetResponse());
  response.mutable_rsa_public_keys(0)->set_message_mask_type(
      AT_MESSAGE_MASK_NO_MASK);

  // If mask type is no mask, sizes other than 0 are invalid.
  response.mutable_rsa_public_keys(0)->set_message_mask_size(10);

  absl::StatusOr<std::vector<RSABlindSignaturePublicKey>> public_keys =
      client_->ProcessAnonymousTokensRSAPublicKeysGetResponse(response);
  EXPECT_EQ(public_keys.status().code(), absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(public_keys.status().message(),
              testing::HasSubstr(
                  "Message mask no mask type must be set to size 0 bytes."));
}

TEST_F(AnonymousTokensPublicKeysGetClientTest, KeySizeIsDifferentThanRealSize) {
  ASSERT_TRUE(
      client_
          ->CreateAnonymousTokensPublicKeysGetRequest(
              TEST_USE_CASE, 0, start_time_, start_time_ + absl::Minutes(100))
          .ok());

  // Make response with wrong key size.
  RSAPublicKey public_key = GenerateFakeRsaKey(
      std::string(kKeyByteSize - 1, 'a'), std::string(kKeyByteSize - 1, 'b'));
  ANON_TOKENS_ASSERT_OK_AND_ASSIGN(auto response,
                                   PublicKeysGetResponse({public_key}));

  absl::StatusOr<std::vector<RSABlindSignaturePublicKey>> public_keys =
      client_->ProcessAnonymousTokensRSAPublicKeysGetResponse(response);
  EXPECT_EQ(public_keys.status().code(), absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(
      public_keys.status().message(),
      testing::HasSubstr("Actual and given Public Key sizes are different."));
}

TEST_F(AnonymousTokensPublicKeysGetClientTest,
       NoPublicKeyInPublicKeyGetResponse) {
  ASSERT_TRUE(
      client_
          ->CreateAnonymousTokensPublicKeysGetRequest(
              TEST_USE_CASE, 0, start_time_, start_time_ + absl::Minutes(100))
          .ok());

  // Make response with no public key.
  ANON_TOKENS_ASSERT_OK_AND_ASSIGN(auto response, SimpleGetResponse());
  response.mutable_rsa_public_keys(0)->clear_serialized_public_key();

  absl::StatusOr<std::vector<RSABlindSignaturePublicKey>> public_keys =
      client_->ProcessAnonymousTokensRSAPublicKeysGetResponse(response);
  EXPECT_EQ(public_keys.status().code(), absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(
      public_keys.status().message(),
      testing::HasSubstr(
          "Public Key not set for a particular use case and key version."));
}

TEST_F(AnonymousTokensPublicKeysGetClientTest,
       NoValidityStartTimeInPublicKeyGetResponse) {
  ASSERT_TRUE(
      client_
          ->CreateAnonymousTokensPublicKeysGetRequest(
              TEST_USE_CASE, 0, start_time_, start_time_ + absl::Minutes(100))
          .ok());
  ANON_TOKENS_ASSERT_OK_AND_ASSIGN(auto response, SimpleGetResponse());
  response.mutable_rsa_public_keys(0)->clear_key_validity_start_time();
  absl::StatusOr<std::vector<RSABlindSignaturePublicKey>> public_keys =
      client_->ProcessAnonymousTokensRSAPublicKeysGetResponse(response);
  EXPECT_EQ(public_keys.status().code(), absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(public_keys.status().message(),
              testing::HasSubstr("Public Key has no set validity start time."));
}

TEST_F(AnonymousTokensPublicKeysGetClientTest,
       ValidityStartTimeAfterRequestedStartTime) {
  ASSERT_TRUE(client_
                  ->CreateAnonymousTokensPublicKeysGetRequest(
                      TEST_USE_CASE, 0, start_time_ - absl::Minutes(100),
                      start_time_ + absl::Minutes(100))
                  .ok());
  ANON_TOKENS_ASSERT_OK_AND_ASSIGN(auto response, SimpleGetResponse());

  absl::StatusOr<std::vector<RSABlindSignaturePublicKey>> public_keys =
      client_->ProcessAnonymousTokensRSAPublicKeysGetResponse(response);
  EXPECT_EQ(public_keys.status().code(), absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(
      public_keys.status().message(),
      testing::HasSubstr(
          "Public Key is not valid at the requested validity start time."));
}

TEST_F(AnonymousTokensPublicKeysGetClientTest,
       IndefinitelyValidKeyReturnedButNotRequested) {
  ASSERT_TRUE(
      client_
          ->CreateAnonymousTokensPublicKeysGetRequest(
              TEST_USE_CASE, 0, start_time_, start_time_ + absl::Minutes(100))
          .ok());
  ANON_TOKENS_ASSERT_OK_AND_ASSIGN(auto response, SimpleGetResponse());
  response.mutable_rsa_public_keys(0)->clear_expiration_time();

  absl::StatusOr<std::vector<RSABlindSignaturePublicKey>> public_keys =
      client_->ProcessAnonymousTokensRSAPublicKeysGetResponse(response);
  EXPECT_EQ(public_keys.status().code(), absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(public_keys.status().message(),
              testing::HasSubstr("Public Key does not expire."));
}

TEST_F(AnonymousTokensPublicKeysGetClientTest,
       KeyWithExpirationTimeReturnedButIndefinitelyValidKeyWasRequested) {
  ASSERT_TRUE(client_
                  ->CreateAnonymousTokensPublicKeysGetRequest(
                      TEST_USE_CASE, 0, start_time_, absl::nullopt)
                  .ok());
  ANON_TOKENS_ASSERT_OK_AND_ASSIGN(auto response, SimpleGetResponse());

  absl::StatusOr<std::vector<RSABlindSignaturePublicKey>> public_keys =
      client_->ProcessAnonymousTokensRSAPublicKeysGetResponse(response);
  EXPECT_EQ(public_keys.status().code(), absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(public_keys.status().message(),
              testing::HasSubstr("Public Key is not indefinitely valid"));
}

TEST_F(AnonymousTokensPublicKeysGetClientTest, WrongExpirationTime) {
  ASSERT_TRUE(
      client_
          ->CreateAnonymousTokensPublicKeysGetRequest(
              TEST_USE_CASE, 0, start_time_, start_time_ + absl::Minutes(90))
          .ok());
  ANON_TOKENS_ASSERT_OK_AND_ASSIGN(auto response, SimpleGetResponse());

  absl::StatusOr<std::vector<RSABlindSignaturePublicKey>> public_keys =
      client_->ProcessAnonymousTokensRSAPublicKeysGetResponse(response);
  EXPECT_EQ(public_keys.status().code(), absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(public_keys.status().message(),
              testing::HasSubstr(
                  "Public Key expires after the requested expiry time."));
}

TEST_F(AnonymousTokensPublicKeysGetClientTest, KeyExpiredBeforeValidityStart) {
  ASSERT_TRUE(
      client_
          ->CreateAnonymousTokensPublicKeysGetRequest(
              TEST_USE_CASE, 0, start_time_, start_time_ + absl::Minutes(100))
          .ok());
  ANON_TOKENS_ASSERT_OK_AND_ASSIGN(auto response, SimpleGetResponse());
  *(response.mutable_rsa_public_keys(0)->mutable_expiration_time()) =
      TimeToProto(start_time_ - absl::Minutes(10)).value();
  absl::StatusOr<std::vector<RSABlindSignaturePublicKey>> public_keys =
      client_->ProcessAnonymousTokensRSAPublicKeysGetResponse(response);
  EXPECT_EQ(public_keys.status().code(), absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(public_keys.status().message(),
              testing::HasSubstr(
                  "Public Key cannot be expired at or before its validity "
                  "start time."));
}

TEST_F(AnonymousTokensPublicKeysGetClientTest, AlreadyExpiredKey) {
  ASSERT_TRUE(
      client_
          ->CreateAnonymousTokensPublicKeysGetRequest(
              TEST_USE_CASE, 0, start_time_, start_time_ + absl::Minutes(110))
          .ok());
  ANON_TOKENS_ASSERT_OK_AND_ASSIGN(auto response, SimpleGetResponse());
  *(response.mutable_rsa_public_keys(0)->mutable_expiration_time()) =
      TimeToProto(start_time_ + (absl::Seconds(5))).value();
  absl::SleepFor(absl::Seconds(10));
  absl::StatusOr<std::vector<RSABlindSignaturePublicKey>> public_keys =
      client_->ProcessAnonymousTokensRSAPublicKeysGetResponse(response);
  EXPECT_EQ(public_keys.status().code(), absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(public_keys.status().message(),
              testing::HasSubstr("Expired Public Key was returned"));
}

TEST_F(AnonymousTokensPublicKeysGetClientTest,
       DuplicateResponsesInPublicKeyGetResponse) {
  ASSERT_TRUE(
      client_
          ->CreateAnonymousTokensPublicKeysGetRequest(
              TEST_USE_CASE, 0, start_time_, start_time_ + absl::Minutes(1000))
          .ok());
  int num_keys = 2;
  std::vector<RSAPublicKey> public_keys(2);
  for (int i = 0; i < num_keys; ++i) {
    public_keys[i] = GenerateFakeRsaKey(std::string(kKeyByteSize, 'a' + i),
                                        std::string(kKeyByteSize, 'b' + i));
  }
  ANON_TOKENS_ASSERT_OK_AND_ASSIGN(auto response,
                                   PublicKeysGetResponse(public_keys));
  response.mutable_rsa_public_keys(0)->set_key_version(1);
  response.mutable_rsa_public_keys(1)->set_key_version(1);

  absl::StatusOr<std::vector<RSABlindSignaturePublicKey>>
      processed_public_keys =
          client_->ProcessAnonymousTokensRSAPublicKeysGetResponse(response);
  EXPECT_EQ(processed_public_keys.status().code(),
            absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(
      processed_public_keys.status().message(),
      testing::HasSubstr("Use Case and Key Version combination must not be "
                         "repeated in the response."));
}

TEST_F(AnonymousTokensPublicKeysGetClientTest, ProcessPublicKeyGetResponse) {
  ASSERT_TRUE(
      client_
          ->CreateAnonymousTokensPublicKeysGetRequest(
              TEST_USE_CASE, 1, start_time_, start_time_ + absl::Minutes(100))
          .ok());
  ANON_TOKENS_ASSERT_OK_AND_ASSIGN(auto single_key_resp, SimpleGetResponse());
  ANON_TOKENS_ASSERT_OK_AND_ASSIGN(
      auto processed_single_key_resp,
      client_->ProcessAnonymousTokensRSAPublicKeysGetResponse(single_key_resp));

  ASSERT_EQ(processed_single_key_resp.size(), 1u);

  EXPECT_EQ(processed_single_key_resp[0].use_case(), "TEST_USE_CASE");
  EXPECT_EQ(processed_single_key_resp[0].key_version(), 1u);
  EXPECT_EQ(processed_single_key_resp[0].key_size(), kKeyByteSize);
  ANON_TOKENS_ASSERT_OK_AND_ASSIGN(
      absl::Time response_start_time,
      TimeFromProto(processed_single_key_resp[0].key_validity_start_time()));
  EXPECT_EQ(response_start_time, start_time_);
  ANON_TOKENS_ASSERT_OK_AND_ASSIGN(
      absl::Time response_end_time,
      TimeFromProto(processed_single_key_resp[0].expiration_time()));
  EXPECT_EQ(response_end_time, start_time_ + kEndTimeIncrement);
  EXPECT_EQ(processed_single_key_resp[0].mask_gen_function(), AT_MGF_SHA384);
  EXPECT_EQ(processed_single_key_resp[0].sig_hash_type(), AT_HASH_TYPE_SHA384);
  EXPECT_EQ(processed_single_key_resp[0].salt_length(), kSaltByteLength);
  EXPECT_EQ(processed_single_key_resp[0].message_mask_type(),
            AT_MESSAGE_MASK_CONCAT);
  EXPECT_EQ(processed_single_key_resp[0].message_mask_size(),
            kMessageMaskByteLength);
  EXPECT_EQ(processed_single_key_resp[0].public_metadata_support(), true);

  RSAPublicKey public_key;
  ASSERT_TRUE(public_key.ParseFromString(
      processed_single_key_resp[0].serialized_public_key()));
  EXPECT_EQ(public_key.n(), default_n_str_);
  EXPECT_EQ(public_key.e(), default_e_str_);
}

TEST_F(AnonymousTokensPublicKeysGetClientTest,
       ProcessPublicKeyGetResponseNoExpiry) {
  ASSERT_TRUE(client_
                  ->CreateAnonymousTokensPublicKeysGetRequest(
                      TEST_USE_CASE, 1, start_time_, absl::nullopt)
                  .ok());
  ANON_TOKENS_ASSERT_OK_AND_ASSIGN(auto single_key_resp, SimpleGetResponse());
  single_key_resp.mutable_rsa_public_keys(0)->clear_expiration_time();
  ANON_TOKENS_ASSERT_OK_AND_ASSIGN(
      auto processed_single_key_resp,
      client_->ProcessAnonymousTokensRSAPublicKeysGetResponse(single_key_resp));

  ASSERT_EQ(processed_single_key_resp.size(), 1u);

  EXPECT_EQ(processed_single_key_resp[0].use_case(), "TEST_USE_CASE");
  EXPECT_EQ(processed_single_key_resp[0].key_version(), 1u);
  EXPECT_EQ(processed_single_key_resp[0].key_size(), kKeyByteSize);
  ANON_TOKENS_ASSERT_OK_AND_ASSIGN(
      absl::Time response_start_time,
      TimeFromProto(processed_single_key_resp[0].key_validity_start_time()));
  EXPECT_EQ(response_start_time, start_time_);
  EXPECT_FALSE(processed_single_key_resp[0].has_expiration_time());
  EXPECT_EQ(processed_single_key_resp[0].mask_gen_function(), AT_MGF_SHA384);
  EXPECT_EQ(processed_single_key_resp[0].sig_hash_type(), AT_HASH_TYPE_SHA384);
  EXPECT_EQ(processed_single_key_resp[0].salt_length(), kSaltByteLength);
  EXPECT_EQ(processed_single_key_resp[0].message_mask_type(),
            AT_MESSAGE_MASK_CONCAT);
  EXPECT_EQ(processed_single_key_resp[0].message_mask_size(),
            kMessageMaskByteLength);
  EXPECT_EQ(processed_single_key_resp[0].public_metadata_support(), true);

  RSAPublicKey public_key;
  ASSERT_TRUE(public_key.ParseFromString(
      processed_single_key_resp[0].serialized_public_key()));
  EXPECT_EQ(public_key.n(), default_n_str_);
  EXPECT_EQ(public_key.e(), default_e_str_);
}

TEST_F(AnonymousTokensPublicKeysGetClientTest,
       ProcessResponseContainingMultiplePublicKeys) {
  size_t total_keys = 5;
  ASSERT_TRUE(client_
                  ->CreateAnonymousTokensPublicKeysGetRequest(
                      TEST_USE_CASE, 0, start_time_,
                      start_time_ + (kEndTimeIncrement * (total_keys + 1)))
                  .ok());
  std::vector<RSAPublicKey> public_keys(total_keys);
  for (size_t i = 0; i < total_keys; ++i) {
    public_keys[i] = GenerateFakeRsaKey(std::string(kKeyByteSize, 'a' + i),
                                        std::string(kKeyByteSize, 'b' + i));
  }
  ANON_TOKENS_ASSERT_OK_AND_ASSIGN(auto response,
                                   PublicKeysGetResponse(public_keys));
  ANON_TOKENS_ASSERT_OK_AND_ASSIGN(
      auto processed_multi_key_resp,
      client_->ProcessAnonymousTokensRSAPublicKeysGetResponse(response));

  ASSERT_EQ(processed_multi_key_resp.size(), total_keys);

  for (size_t i = 1; i < total_keys; ++i) {
    EXPECT_EQ(processed_multi_key_resp[i].use_case(), "TEST_USE_CASE");
    EXPECT_EQ(processed_multi_key_resp[i].key_version(), i + 1);
    EXPECT_EQ(processed_multi_key_resp[i].key_size(), kKeyByteSize);
    ANON_TOKENS_ASSERT_OK_AND_ASSIGN(
        absl::Time response_start_time,
        TimeFromProto(processed_multi_key_resp[i].key_validity_start_time()));
    EXPECT_EQ(response_start_time, start_time_ - (kStartTimeIncrement * i));
    ANON_TOKENS_ASSERT_OK_AND_ASSIGN(
        absl::Time response_end_time,
        TimeFromProto(processed_multi_key_resp[i].expiration_time()));
    EXPECT_EQ(response_end_time, start_time_ + (kEndTimeIncrement * (i + 1)));
    EXPECT_EQ(processed_multi_key_resp[i].mask_gen_function(), AT_MGF_SHA384);
    EXPECT_EQ(processed_multi_key_resp[i].sig_hash_type(), AT_HASH_TYPE_SHA384);
    EXPECT_EQ(processed_multi_key_resp[i].salt_length(), kSaltByteLength);
    EXPECT_EQ(processed_multi_key_resp[i].message_mask_type(),
              AT_MESSAGE_MASK_CONCAT);
    EXPECT_EQ(processed_multi_key_resp[i].message_mask_size(),
              kMessageMaskByteLength);
    EXPECT_EQ(processed_multi_key_resp[i].public_metadata_support(), true);

    RSAPublicKey public_key;
    ASSERT_TRUE(public_key.ParseFromString(
        processed_multi_key_resp[i].serialized_public_key()));
    EXPECT_EQ(public_key.n(), public_keys[i].n());
    EXPECT_EQ(public_key.e(), public_keys[i].e());
  }
}

}  // namespace
}  // namespace anonymous_tokens
