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

#include "anonymous_tokens/cpp/privacy_pass/token_encodings.h"

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/escaping.h"
#include "absl/time/clock.h"
#include "absl/time/time.h"
#include "absl/types/span.h"
#include "anonymous_tokens/cpp/testing/utils.h"

namespace anonymous_tokens {
namespace {

TEST(AnonymousTokensPrivacyPassTokenEncodingsTest,
     EmptyAuthenticatorInputTest) {
  Token token;
  ANON_TOKENS_ASSERT_OK_AND_ASSIGN(std::string authenticator_input,
                                   AuthenticatorInput(token));

  std::string expected_authenticator_input_encoding;
  ASSERT_TRUE(
      absl::HexStringToBytes("DA7A", &expected_authenticator_input_encoding));

  EXPECT_EQ(authenticator_input, expected_authenticator_input_encoding);
}

TEST(AnonymousTokensPrivacyPassTokenEncodingsTest, AuthenticatorInputTest) {
  std::string token_key_id;
  ASSERT_TRUE(absl::HexStringToBytes(
      "ca572f8982a9ca248a3056186322d93ca147266121ddeb5632c07f1f71cd2708",
      &token_key_id));
  std::string nonce;
  ASSERT_TRUE(absl::HexStringToBytes(
      "5f5e46604255ac6a8ae0820f5b20c236118d97d917509ccbc96b5a82ae40ebeb",
      &nonce));
  std::string context;
  ASSERT_TRUE(absl::HexStringToBytes(
      "11e15c91a7c2ad02abd66645802373db1d823bea80f08d452541fb2b62b5898b",
      &context));
  Token token = {/*token_type=*/0XDA7A, std::move(token_key_id),
                 std::move(nonce), std::move(context)};

  ANON_TOKENS_ASSERT_OK_AND_ASSIGN(std::string authenticator_input,
                                   AuthenticatorInput(token));

  std::string expected_authenticator_input_encoding;
  ASSERT_TRUE(absl::HexStringToBytes(
      "DA7A5f5e46604255ac6a8ae0820f5b20c236118d97d917509ccbc96b5a82ae40ebeb11e1"
      "5c91a7c2ad02abd66645802373db1d823bea80f08d452541fb2b62b5898bca572f8982a9"
      "ca248a3056186322d93ca147266121ddeb5632c07f1f71cd2708",
      &expected_authenticator_input_encoding));
  EXPECT_EQ(authenticator_input, expected_authenticator_input_encoding);
}

TEST(AnonymousTokensPrivacyPassTokenEncodingsTest, EmptyMarshalTokenTest) {
  Token token;
  ANON_TOKENS_ASSERT_OK_AND_ASSIGN(std::string encoded_token,
                                   MarshalToken(token));
  std::string expected_token_encoding;
  ASSERT_TRUE(absl::HexStringToBytes("DA7A", &expected_token_encoding));

  EXPECT_EQ(encoded_token, expected_token_encoding);
}

TEST(AnonymousTokensPrivacyPassTokenEncodingsTest, MarshalTokenTest) {
  std::string token_key_id;
  ASSERT_TRUE(absl::HexStringToBytes(
      "ca572f8982a9ca248a3056186322d93ca147266121ddeb5632c07f1f71cd2708",
      &token_key_id));
  std::string nonce;
  ASSERT_TRUE(absl::HexStringToBytes(
      "5f5e46604255ac6a8ae0820f5b20c236118d97d917509ccbc96b5a82ae40ebeb",
      &nonce));
  std::string context;
  ASSERT_TRUE(absl::HexStringToBytes(
      "11e15c91a7c2ad02abd66645802373db1d823bea80f08d452541fb2b62b5898b",
      &context));
  std::string authenticator;
  ASSERT_TRUE(absl::HexStringToBytes(
      "4ed3f2a25ec528543d9a83c850d12b3036b518fafec080df3efcd9693b944d0560568620"
      "0d6500f249475737ea9246a70c3c2a1ff280663e46c792a8ae0d9a6877d1b427bbae7129"
      "b88c92ad61c08a9fe41629a642263e4857e428a706ba87659361fed38087c0e881f5e156"
      "68e0701d7edd63be98fcc7415819d466c61341de03d7e2a24181d7b9321b0826d59402a8"
      "7e08514f36cc45b0f7aac0e9a6578ddb0534c8ebe528c693b6efb54e76a5a8056f5c27d0"
      "1ad42119953c5987b05c9ae2ca04b12838e641b4b1aac21e36f18573f603735fac1f8f61"
      "1029e4cb76c8a5cc6f2c4143474e458c8d2ca8e9a71f01d90e0d2d784874ff000ae10548"
      "3941652e",
      &authenticator));
  Token token = {/*token_type=*/0XDA7A, std::move(token_key_id),
                 std::move(nonce), std::move(context),
                 std::move(authenticator)};

  ANON_TOKENS_ASSERT_OK_AND_ASSIGN(std::string encoded_token,
                                   MarshalToken(token));

  std::string expected_token_encoding;
  ASSERT_TRUE(absl::HexStringToBytes(
      "DA7A5f5e46604255ac6a8ae0820f5b20c236118d97d917509ccbc96b5a82ae40ebeb11e1"
      "5c91a7c2ad02abd66645802373db1d823bea80f08d452541fb2b62b5898bca572f8982a9"
      "ca248a3056186322d93ca147266121ddeb5632c07f1f71cd27084ed3f2a25ec528543d9a"
      "83c850d12b3036b518fafec080df3efcd9693b944d05605686200d6500f249475737ea92"
      "46a70c3c2a1ff280663e46c792a8ae0d9a6877d1b427bbae7129b88c92ad61c08a9fe416"
      "29a642263e4857e428a706ba87659361fed38087c0e881f5e15668e0701d7edd63be98fc"
      "c7415819d466c61341de03d7e2a24181d7b9321b0826d59402a87e08514f36cc45b0f7aa"
      "c0e9a6578ddb0534c8ebe528c693b6efb54e76a5a8056f5c27d01ad42119953c5987b05c"
      "9ae2ca04b12838e641b4b1aac21e36f18573f603735fac1f8f611029e4cb76c8a5cc6f2c"
      "4143474e458c8d2ca8e9a71f01d90e0d2d784874ff000ae105483941652e",
      &expected_token_encoding));
  EXPECT_EQ(encoded_token, expected_token_encoding);

  ANON_TOKENS_ASSERT_OK_AND_ASSIGN(const Token token2,
                                   UnmarshalToken(encoded_token));
  EXPECT_EQ(token.token_type, token2.token_type);
  EXPECT_EQ(token.token_key_id, token2.token_key_id);
  EXPECT_EQ(token.context, token2.context);
  EXPECT_EQ(token.nonce, token2.nonce);
  EXPECT_EQ(token.authenticator, token2.authenticator);
}

TEST(AnonymousTokensPrivacyPassTokenEncodingsTest, UnmarshalTooShort) {
  std::string short_token;
  ASSERT_TRUE(absl::HexStringToBytes("DA7A5f5e466042", &short_token));
  EXPECT_FALSE(UnmarshalToken(short_token).ok());
}

TEST(AnonymousTokensPrivacyPassTokenEncodingsTest, UnmarshalTooLong) {
  std::string long_token;
  ASSERT_TRUE(absl::HexStringToBytes(
      "DA7A5f5e46604255ac6a8ae0820f5b20c236118d97d917509ccbc96b5a82ae40ebeb11e1"
      "5c91a7c2ad02abd66645802373db1d823bea80f08d452541fb2b62b5898bca572f8982a9"
      "ca248a3056186322d93ca147266121ddeb5632c07f1f71cd27084ed3f2a25ec528543d9a"
      "83c850d12b3036b518fafec080df3efcd9693b944d05605686200d6500f249475737ea92"
      "46a70c3c2a1ff280663e46c792a8ae0d9a6877d1b427bbae7129b88c92ad61c08a9fe416"
      "29a642263e4857e428a706ba87659361fed38087c0e881f5e15668e0701d7edd63be98fc"
      "c7415819d466c61341de03d7e2a24181d7b9321b0826d59402a87e08514f36cc45b0f7aa"
      "c0e9a6578ddb0534c8ebe528c693b6efb54e76a5a8056f5c27d01ad42119953c5987b05c"
      "9ae2ca04b12838e641b4b1aac21e36f18573f603735fac1f8f611029e4cb76c8a5cc6f2c"
      "4143474e458c8d2ca8e9a71f01d90e0d2d784874ff000ae105483941652e9ae2ca04",
      &long_token));
  EXPECT_FALSE(UnmarshalToken(long_token).ok());
}

TEST(AnonymousTokensPrivacyPassTokenEncodingsTest, UnmarshalWrongType) {
  std::string token;
  ASSERT_TRUE(absl::HexStringToBytes(
      "DA7B5f5e46604255ac6a8ae0820f5b20c236118d97d917509ccbc96b5a82ae40ebeb11e1"
      "5c91a7c2ad02abd66645802373db1d823bea80f08d452541fb2b62b5898bca572f8982a9"
      "ca248a3056186322d93ca147266121ddeb5632c07f1f71cd27084ed3f2a25ec528543d9a"
      "83c850d12b3036b518fafec080df3efcd9693b944d05605686200d6500f249475737ea92"
      "46a70c3c2a1ff280663e46c792a8ae0d9a6877d1b427bbae7129b88c92ad61c08a9fe416"
      "29a642263e4857e428a706ba87659361fed38087c0e881f5e15668e0701d7edd63be98fc"
      "c7415819d466c61341de03d7e2a24181d7b9321b0826d59402a87e08514f36cc45b0f7aa"
      "c0e9a6578ddb0534c8ebe528c693b6efb54e76a5a8056f5c27d01ad42119953c5987b05c"
      "9ae2ca04b12838e641b4b1aac21e36f18573f603735fac1f8f611029e4cb76c8a5cc6f2c"
      "4143474e458c8d2ca8e9a71f01d90e0d2d784874ff000ae105483941652e",
      &token));
  EXPECT_FALSE(UnmarshalToken(token).ok());
}

TEST(AnonymousTokensPrivacyPassTokenEncodingsTest, EmptyExtensionTest) {
  Extension extension;
  ANON_TOKENS_ASSERT_OK_AND_ASSIGN(std::string encoded_extension,
                                   EncodeExtension(extension));

  std::string expected_extension_encoding;
  ASSERT_TRUE(absl::HexStringToBytes("00010000", &expected_extension_encoding));
  EXPECT_EQ(encoded_extension, expected_extension_encoding);
}

TEST(AnonymousTokensPrivacyPassTokenEncodingsTest,
     ExtensionValueSizeOfMoreThanTwoBytes) {
  // Input string of size more than 2 bytes.
  std::string large_test_value = std::string(65536, 'a');
  // Random hex number to populate the uint16_t extension_type.
  Extension extension = {/*extension_type=*/0x5E6D,
                         /*extension_value=*/large_test_value};
  absl::StatusOr<std::string> encoded_extension = EncodeExtension(extension);

  EXPECT_EQ(encoded_extension.status().code(),
            absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(encoded_extension.status().message(),
              ::testing::HasSubstr("Failed to generate extension encoding"));
}

TEST(AnonymousTokensPrivacyPassTokenEncodingsTest, ExtensionEncodingSuccess) {
  std::string extension_value;
  ASSERT_TRUE(absl::HexStringToBytes(
      "46a70c3c2a1ff280663e46c792a8ae0d9a6877d1b427bbae7129b88c92ad61c08a9fe416"
      "29a642263e4857e428a706ba87659361fed38087c0e881f5e15668e0701d7edd63be98fc"
      "c7415819d466c61341de03d7e2a24181d7b9321b0826d59402a87e08514f36cc45b0f7a"
      "a",
      &extension_value));
  Extension extension = {
      // Random hex number to populate the uint16_t extension_type.
      /*extension_type=*/0x5E6D,
      // Random hex string to populate extension_value.
      std::move(extension_value)};
  ANON_TOKENS_ASSERT_OK_AND_ASSIGN(std::string encoded_extension,
                                   EncodeExtension(extension));
  // The first 2 bytes of the expected_extension_encoding store the
  // extension_type. The next two bytes store the number of bytes needed for the
  // extension_value. These 4 bytes are then prefixed to the extension_value.
  std::string expected_extension_encoding;
  ASSERT_TRUE(absl::HexStringToBytes(
      "5e6d006c46a70c3c2a1ff280663e46c792a8ae0d9a6877d1b427bbae7129b88c92ad61c0"
      "8a9fe41629a642263e4857e428a706ba87659361fed38087c0e881f5e15668e0701d7edd"
      "63be98fcc7415819d466c61341de03d7e2a24181d7b9321b0826d59402a87e08514f36cc"
      "45b0f7aa",
      &expected_extension_encoding));
  EXPECT_EQ(encoded_extension, expected_extension_encoding);
}

TEST(AnonymousTokensPrivacyPassTokenEncodingsTest, EmptyExtensionsTest) {
  Extensions extensions;
  ANON_TOKENS_ASSERT_OK_AND_ASSIGN(std::string encoded_extensions,
                                   EncodeExtensions(extensions));

  std::string expected_extensions_encoding;
  ASSERT_TRUE(absl::HexStringToBytes("0000", &expected_extensions_encoding));
  EXPECT_EQ(encoded_extensions, expected_extensions_encoding);
}

TEST(AnonymousTokensPrivacyPassTokenEncodingsTest,
     ExtensionsListSizeOfMoreThanTwoBytes) {
  std::string large_test_value = std::string(65532, 'a');
  Extensions extensions;
  extensions.extensions.push_back(
      {// Random hex number to populate the uint16_t extension_type.
       /*extension_type=*/0x5E6D,
       /*extension_value=*/large_test_value});
  absl::StatusOr<std::string> encoded_extensions = EncodeExtensions(extensions);
  // The string encoding of this extensions struct will take at least 65536
  // bytes. This length is not be storable in 2 bytes.
  EXPECT_EQ(encoded_extensions.status().code(),
            absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(
      encoded_extensions.status().message(),
      ::testing::HasSubstr("Failed to generate encoded extensions list"));
}

TEST(AnonymousTokensPrivacyPassTokenEncodingsTest,
     SingleExtensionInExtensionsSuccess) {
  std::string extension_value;
  ASSERT_TRUE(absl::HexStringToBytes(
      "46a70c3c2a1ff280663e46c792a8ae0d9a6877d1b427bbae7129b88c92ad61c08a9fe416"
      "29a642263e4857e428a706ba87659361fed38087c0e881f5e15668e0701d7edd63be98fc"
      "c7415819d466c61341de03d7e2a24181d7b9321b0826d59402a87e08514f36cc45b0f7a"
      "a",
      &extension_value));
  Extensions extensions;
  extensions.extensions.push_back(Extension{
      // Random hex number to populate the uint16_t extension_type.
      /*extension_type=*/0x5E6D,
      // Random hex string to populate extension_value.
      std::move(extension_value)});
  ANON_TOKENS_ASSERT_OK_AND_ASSIGN(std::string encoded_extensions,
                                   EncodeExtensions(extensions));
  // The first 2 bytes of the expected_extensions_encoding store the length of
  // the rest of the string. The rest of the string is the concatenation
  // of individually encoded extensions.
  std::string expected_extensions_encoding;
  ASSERT_TRUE(absl::HexStringToBytes(
      "00705e6d006c46a70c3c2a1ff280663e46c792a8ae0d9a6877d1b427bbae7129b88c92ad"
      "61c08a9fe41629a642263e4857e428a706ba87659361fed38087c0e881f5e15668e0701d"
      "7edd63be98fcc7415819d466c61341de03d7e2a24181d7b9321b0826d59402a87e08514f"
      "36cc45b0f7aa",
      &expected_extensions_encoding));
  EXPECT_EQ(encoded_extensions, expected_extensions_encoding);
}

TEST(AnonymousTokensPrivacyPassTokenEncodingsTest,
     MultipleExtensionsEncodingSuccess) {
  std::string extension_value_1;
  ASSERT_TRUE(absl::HexStringToBytes("01", &extension_value_1));
  std::string extension_value_2;
  ASSERT_TRUE(absl::HexStringToBytes("0202", &extension_value_2));
  Extensions extensions;
  extensions.extensions.push_back(
      Extension{/*extension_type=*/0x0001, std::move(extension_value_1)});
  extensions.extensions.push_back(
      Extension{/*extension_type=*/0x0002, std::move(extension_value_2)});
  ANON_TOKENS_ASSERT_OK_AND_ASSIGN(const std::string encoded_extensions,
                                   EncodeExtensions(extensions));
  // The first 2 bytes of the expected_extensions_encoding store the length of
  // the rest of the string. The rest of the string is the concatenation
  // of individually encoded extensions.
  std::string expected_extensions_encoding;
  ASSERT_TRUE(absl::HexStringToBytes("000b0001000101000200020202",
                                     &expected_extensions_encoding));
  EXPECT_EQ(encoded_extensions, expected_extensions_encoding);
  ANON_TOKENS_ASSERT_OK_AND_ASSIGN(const Extensions extensions2,
                                   DecodeExtensions(encoded_extensions));
  EXPECT_EQ(extensions2.extensions.size(), extensions.extensions.size());
  for (size_t i = 0; i < extensions.extensions.size(); ++i) {
    EXPECT_EQ(extensions2.extensions[i].extension_type,
              extensions.extensions[i].extension_type);
    EXPECT_EQ(extensions2.extensions[i].extension_value,
              extensions.extensions[i].extension_value);
  }
}

TEST(AnonymousTokensPrivacyPassTokenEncodingsTest,
     ExpirationTimestampRoundTrip) {
  ExpirationTimestamp et{.timestamp_precision = 3600, .timestamp = 1688583600};
  ANON_TOKENS_ASSERT_OK_AND_ASSIGN(const Extension ext, et.AsExtension());
  EXPECT_EQ(ext.extension_type, 0x0001);
  ANON_TOKENS_ASSERT_OK_AND_ASSIGN(const ExpirationTimestamp et2,
                                   ExpirationTimestamp::FromExtension(ext));
  EXPECT_EQ(et.timestamp_precision, et2.timestamp_precision);
  EXPECT_EQ(et.timestamp, et2.timestamp);
}

TEST(AnonymousTokensPrivacyPassTokenEncodingsTest,
     ExpirationTimestampWrongType) {
  const Extension ext = {.extension_type = 0x0002, .extension_value = ""};
  EXPECT_FALSE(ExpirationTimestamp::FromExtension(ext).ok());
}

TEST(AnonymousTokensPrivacyPassTokenEncodingsTest, DecodeExtensions) {
  std::string encoded_extensions;
  ASSERT_TRUE(absl::HexStringToBytes(
      "0014000100100000000000000E100000000064A5BDB0", &encoded_extensions));
  ANON_TOKENS_ASSERT_OK_AND_ASSIGN(const Extensions extensions,
                                   DecodeExtensions(encoded_extensions));
  EXPECT_EQ(extensions.extensions.size(), 1u);
  ANON_TOKENS_ASSERT_OK_AND_ASSIGN(
      const ExpirationTimestamp et,
      ExpirationTimestamp::FromExtension(extensions.extensions[0]));
  EXPECT_EQ(et.timestamp_precision, 3600u);
  EXPECT_EQ(et.timestamp, 1688583600u);
  Extensions extensions2;
  ANON_TOKENS_ASSERT_OK_AND_ASSIGN(const Extension ext2, et.AsExtension());
  extensions2.extensions.push_back(ext2);
  ANON_TOKENS_ASSERT_OK_AND_ASSIGN(const std::string encoded_extensions2,
                                   EncodeExtensions(extensions2));
  EXPECT_EQ(absl::BytesToHexString(encoded_extensions2),
            absl::BytesToHexString(encoded_extensions));
}

TEST(AnonymousTokensPrivacyPassTokenEncodingsTest, DecodeTooShort) {
  std::string encoded_extensions;
  ASSERT_TRUE(absl::HexStringToBytes("00140001001000", &encoded_extensions));
  const absl::StatusOr<Extensions> extensions =
      DecodeExtensions(encoded_extensions);
  EXPECT_FALSE(extensions.ok()) << extensions.status();
}

TEST(AnonymousTokensPrivacyPassTokenEncodingsTest, DecodeTooLong) {
  std::string encoded_extensions;
  ASSERT_TRUE(absl::HexStringToBytes(
      "0014000100100000000000000E100000000064A5BDB0123456",
      &encoded_extensions));
  const absl::StatusOr<Extensions> extensions =
      DecodeExtensions(encoded_extensions);
  EXPECT_FALSE(extensions.ok());
}

TEST(AnonymousTokensPrivacyPassTokenEncodingsTest, GeoHintRoundTrip) {
  GeoHint gh{.geo_hint = "US,US-AL,ALABASTER"};
  ANON_TOKENS_ASSERT_OK_AND_ASSIGN(const Extension ext, gh.AsExtension());
  EXPECT_EQ(ext.extension_type, 0x0002);
  ANON_TOKENS_ASSERT_OK_AND_ASSIGN(const GeoHint gh2,
                                   GeoHint::FromExtension(ext));
  EXPECT_EQ(gh.geo_hint, gh2.geo_hint);
  EXPECT_EQ(gh2.country_code, "US");
  EXPECT_EQ(gh2.region, "US-AL");
  EXPECT_EQ(gh2.city, "ALABASTER");
}

TEST(AnonymousTokensPrivacyPassTokenEncodingsTest, GeoHintNoPostal) {
  GeoHint gh{.geo_hint = "US,US-AL,ALABASTER,FOO"};
  ANON_TOKENS_ASSERT_OK_AND_ASSIGN(const Extension ext, gh.AsExtension());
  EXPECT_FALSE(GeoHint::FromExtension(ext).ok());
}

TEST(AnonymousTokensPrivacyPassTokenEncodingsTest, GeoHintNoLowercase) {
  GeoHint gh{.geo_hint = "US,US-AL,Alabaster"};
  ANON_TOKENS_ASSERT_OK_AND_ASSIGN(const Extension ext, gh.AsExtension());
  EXPECT_FALSE(GeoHint::FromExtension(ext).ok());
}

TEST(AnonymousTokensPrivacyPassTokenEncodingsTest, GeoHintTooShort) {
  GeoHint gh{.geo_hint = "US,US-AL"};
  ANON_TOKENS_ASSERT_OK_AND_ASSIGN(const Extension ext, gh.AsExtension());
  EXPECT_FALSE(GeoHint::FromExtension(ext).ok());
}

TEST(AnonymousTokensPrivacyPassTokenEncodingsTest, GeoHintEmptyOk) {
  GeoHint gh{.geo_hint = "US,,"};
  ANON_TOKENS_ASSERT_OK_AND_ASSIGN(const Extension ext, gh.AsExtension());
  EXPECT_TRUE(GeoHint::FromExtension(ext).ok());
}

TEST(AnonymousTokensPrivacyPassTokenEncodingsTest, ChromeServiceTypeRoundTrip) {
  ServiceType st{.service_type_id = ServiceType::kChromeIpBlinding};
  ANON_TOKENS_ASSERT_OK_AND_ASSIGN(const Extension ext, st.AsExtension());
  EXPECT_EQ(ext.extension_type, 0xF001);
  ANON_TOKENS_ASSERT_OK_AND_ASSIGN(const ServiceType st2,
                                   ServiceType::FromExtension(ext));
  EXPECT_EQ(st.service_type_id, st2.service_type_id);
  EXPECT_EQ(st2.service_type, "chromeipblinding");
}

TEST(AnonymousTokensPrivacyPassTokenEncodingsTest, CronetServiceTypeRoundTrip) {
  ServiceType st{.service_type_id = ServiceType::kCronetIpBlinding};
  ANON_TOKENS_ASSERT_OK_AND_ASSIGN(const Extension ext, st.AsExtension());
  EXPECT_EQ(ext.extension_type, 0xF001);
  ANON_TOKENS_ASSERT_OK_AND_ASSIGN(const ServiceType st2,
                                   ServiceType::FromExtension(ext));
  EXPECT_EQ(st.service_type_id, st2.service_type_id);
  EXPECT_EQ(st2.service_type, "cronetipblinding");
}

TEST(AnonymousTokensPrivacyPassTokenEncodingsTest, WebviewServiceTypeRoundTrip)
{
  ServiceType st{.service_type_id = ServiceType::kWebviewIpBlinding};
  ANON_TOKENS_ASSERT_OK_AND_ASSIGN(const Extension ext, st.AsExtension());
  EXPECT_EQ(ext.extension_type, 0xF001);
  ANON_TOKENS_ASSERT_OK_AND_ASSIGN(const ServiceType st2,
                                   ServiceType::FromExtension(ext));
  EXPECT_EQ(st.service_type_id, st2.service_type_id);
  EXPECT_EQ(st2.service_type, "webviewipblinding");
}

TEST(AnonymousTokensPrivacyPassTokenEncodingsTest, WrongExtId) {
  ServiceType st{.service_type_id = ServiceType::kChromeIpBlinding};
  ANON_TOKENS_ASSERT_OK_AND_ASSIGN(Extension ext, st.AsExtension());
  ext.extension_type = 0xF002;
  EXPECT_FALSE(ServiceType::FromExtension(ext).ok());
}

TEST(AnonymousTokensPrivacyPassTokenEncodingsTest, WrongServiceTypeId) {
  ServiceType st{.service_type_id = 0x04};
  ANON_TOKENS_ASSERT_OK_AND_ASSIGN(Extension ext, st.AsExtension());
  EXPECT_EQ(ext.extension_type, 0xF001);
  EXPECT_FALSE(ServiceType::FromExtension(ext).ok());
}

TEST(AnonymousTokensDebugMode, RoundTrip) {
  DebugMode dm{.mode = DebugMode::kProd};
  ANON_TOKENS_ASSERT_OK_AND_ASSIGN(const Extension ext, dm.AsExtension());
  EXPECT_EQ(ext.extension_type, 0xF002);
  ANON_TOKENS_ASSERT_OK_AND_ASSIGN(const DebugMode dm2,
                                   DebugMode::FromExtension(ext));
  EXPECT_EQ(dm.mode, dm2.mode);
}

TEST(AnonymousTokensDebugMode, WrongExtId) {
  DebugMode dm{.mode = DebugMode::kProd};
  ANON_TOKENS_ASSERT_OK_AND_ASSIGN(Extension ext, dm.AsExtension());
  ext.extension_type = 0xF003;
  EXPECT_FALSE(ServiceType::FromExtension(ext).ok());
}

TEST(AnonymousTokensDebugMode, InvalidMode) {
  DebugMode dm{.mode = DebugMode::kProd};
  ANON_TOKENS_ASSERT_OK_AND_ASSIGN(Extension ext, dm.AsExtension());
  EXPECT_EQ(ext.extension_type, 0xF002);
  ext.extension_value = std::string("~");
  EXPECT_FALSE(DebugMode::FromExtension(ext).ok());
}

TEST(AnonymousTokensProxyLayer, RoundTripProxyA) {
  ProxyLayer pl{.layer = ProxyLayer::kProxyA};
  ANON_TOKENS_ASSERT_OK_AND_ASSIGN(const Extension ext, pl.AsExtension());
  EXPECT_EQ(ext.extension_type, 0xF003);
  ANON_TOKENS_ASSERT_OK_AND_ASSIGN(const ProxyLayer pl2,
                                   ProxyLayer::FromExtension(ext));
  EXPECT_EQ(pl.layer, pl2.layer);
}

TEST(AnonymousTokensProxyLayer, RoundTripProxyB) {
  ProxyLayer pl{.layer = ProxyLayer::kProxyB};
  ANON_TOKENS_ASSERT_OK_AND_ASSIGN(const Extension ext, pl.AsExtension());
  EXPECT_EQ(ext.extension_type, 0xF003);
  ANON_TOKENS_ASSERT_OK_AND_ASSIGN(const ProxyLayer pl2,
                                   ProxyLayer::FromExtension(ext));
  EXPECT_EQ(pl.layer, pl2.layer);
}

TEST(AnonymousTokensProxyLayer, WrongExtId) {
  ProxyLayer pl{.layer = ProxyLayer::kProxyA};
  ANON_TOKENS_ASSERT_OK_AND_ASSIGN(Extension ext, pl.AsExtension());
  ext.extension_type = 0xF004;
  EXPECT_FALSE(ProxyLayer::FromExtension(ext).ok());
}

TEST(AnonymousTokensProxyLayer, InvalidLayer) {
  ProxyLayer pl{.layer = ProxyLayer::kProxyA};
  ANON_TOKENS_ASSERT_OK_AND_ASSIGN(Extension ext, pl.AsExtension());
  EXPECT_EQ(ext.extension_type, 0xF003);
  ext.extension_value = std::string("~");
  EXPECT_FALSE(ProxyLayer::FromExtension(ext).ok());
}

TEST(AnonymousTokensPrivacyPassTokenEncodingsTest,
     EmptyMarshalTokenChallengeTest) {
  TokenChallenge token_challenge;
  ANON_TOKENS_ASSERT_OK_AND_ASSIGN(std::string encoded_token,
                                   MarshalTokenChallenge(token_challenge));

  std::string expected_token_encoding;
  ASSERT_TRUE(absl::HexStringToBytes("DA7A0000", &expected_token_encoding));

  EXPECT_EQ(encoded_token, expected_token_encoding);
}

TEST(AnonymousTokensPrivacyPassTokenEncodingsTest, MarshalTokenChallengeTest) {
  TokenChallenge token_challenge;
  token_challenge.issuer_name = "issuer.google.com";
  ANON_TOKENS_ASSERT_OK_AND_ASSIGN(std::string encoded_token,
                                   MarshalTokenChallenge(token_challenge));

  std::string expected_token_encoding;
  ASSERT_TRUE(absl::HexStringToBytes(
      "da7a00116973737565722e676f6f676c652e636f6d", &expected_token_encoding));

  EXPECT_EQ(encoded_token, expected_token_encoding);
}

TEST(AnonymousTokensPrivacyPassTokenEncodingsTest,
     UnMarshalTokenRequestWrongTokenType) {
  std::string token_request_encoding;
  ASSERT_TRUE(absl::HexStringToBytes(
      "1234124ed3f2a25ec528543d9a83c850d12b3036b518fafec080df3efcd9693b944d0560"
      "5686200d6500f249475737ea9246a70c3c2a1ff280663e46c792a8ae0d9a6877d1b427bb"
      "ae7129b88c92ad61c08a9fe41629a642263e4857e428a706ba87659361fed38087c0e881"
      "f5e15668e0701d7edd63be98fcc7415819d466c61341de03d7e2a24181d7b9321b0826d5"
      "9402a87e08514f36cc45b0f7aac0e9a6578ddb0534c8ebe528c693b6efb54e76a5a8056f"
      "5c27d01ad42119953c5987b05c9ae2ca04b12838e641b4b1aac21e36f18573f603735fac"
      "1f8f611029e4cb76c8a5cc6f2c4143474e458c8d2ca8e9a71f01d90e0d2d784874ff000a"
      "e105483941652e",
      &token_request_encoding));
  absl::StatusOr<TokenRequest> token_request =
      UnmarshalTokenRequest(token_request_encoding);

  EXPECT_EQ(token_request.status().code(), absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(token_request.status().message(),
              ::testing::HasSubstr("unsupported token type"));
}

TEST(AnonymousTokensPrivacyPassTokenEncodingsTest,
     UnMarshalTokenRequestBlindedRequestTooShort) {
  std::string token_request_encoding;
  ASSERT_TRUE(absl::HexStringToBytes(
      "DA7A124ed3f2a25ec528543d9a83c850d12b3036b518fafec080df3efcd9693b944d0560"
      "5686200d6500f249475737ea9246a70c3c2a1ff280663e46c792a8ae0d9a6877d1b427bb"
      "ae7129b88c92ad61c08a9fe41629a642263e4857e428a706ba87659361fed38087c0e881"
      "f5e15668e0701d7edd63be98fcc7415819d466c61341de03d7e2a24181d7b9321b0826d5"
      "9402a87e08514f36cc45b0f7aac0e9a6578ddb0534c8ebe528c693b6efb54e76a5a8056f"
      "5c27d01ad42119953c5987b05c9ae2ca04b12838e641b4b1aac21e36f18573f603735fac"
      "1f8f611029e4cb76c8a5cc6f2c4143474e458c8d2ca8e9a71f01d90e0d2d784874ff000"
      "a",
      &token_request_encoding));
  absl::StatusOr<TokenRequest> token_request =
      UnmarshalTokenRequest(token_request_encoding);

  EXPECT_EQ(token_request.status().code(), absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(token_request.status().message(),
              ::testing::HasSubstr("failed to read blinded_token_request"));
}

TEST(AnonymousTokensPrivacyPassTokenEncodingsTest,
     UnMarshalTokenRequestBlindedRequestTooLong) {
  std::string token_request_encoding;
  ASSERT_TRUE(absl::HexStringToBytes(
      "DA7A124ed3f2a25ec528543d9a83c850d12b3036b518fafec080df3efcd9693b944d0560"
      "5686200d6500f249475737ea9246a70c3c2a1ff280663e46c792a8ae0d9a6877d1b427bb"
      "ae7129b88c92ad61c08a9fe41629a642263e4857e428a706ba87659361fed38087c0e881"
      "f5e15668e0701d7edd63be98fcc7415819d466c61341de03d7e2a24181d7b9321b0826d5"
      "9402a87e08514f36cc45b0f7aac0e9a6578ddb0534c8ebe528c693b6efb54e76a5a8056f"
      "5c27d01ad42119953c5987b05c9ae2ca04b12838e641b4b1aac21e36f18573f603735fac"
      "1f8f611029e4cb76c8a5cc6f2c4143474e458c8d2ca8e9a71f01d90e0d2d784874ff000a"
      "e105483941652ea5cc6f2c4143474e458c8d2ca8e9aa",
      &token_request_encoding));
  absl::StatusOr<TokenRequest> token_request =
      UnmarshalTokenRequest(token_request_encoding);

  EXPECT_EQ(token_request.status().code(), absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(token_request.status().message(),
              ::testing::HasSubstr("token request had extra bytes"));
}

TEST(AnonymousTokensPrivacyPassTokenEncodingsTest,
     MarshalAndUnmarshalTokenRequest) {
  std::string blinded_token_request;
  ASSERT_TRUE(absl::HexStringToBytes(
      "4ed3f2a25ec528543d9a83c850d12b3036b518fafec080df3efcd9693b944d0560568620"
      "0d6500f249475737ea9246a70c3c2a1ff280663e46c792a8ae0d9a6877d1b427bbae7129"
      "b88c92ad61c08a9fe41629a642263e4857e428a706ba87659361fed38087c0e881f5e156"
      "68e0701d7edd63be98fcc7415819d466c61341de03d7e2a24181d7b9321b0826d59402a8"
      "7e08514f36cc45b0f7aac0e9a6578ddb0534c8ebe528c693b6efb54e76a5a8056f5c27d0"
      "1ad42119953c5987b05c9ae2ca04b12838e641b4b1aac21e36f18573f603735fac1f8f61"
      "1029e4cb76c8a5cc6f2c4143474e458c8d2ca8e9a71f01d90e0d2d784874ff000ae10548"
      "3941652e",
      &blinded_token_request));
  TokenRequest token_request{
      .token_type = 0xDA7A,
      .truncated_token_key_id = 0x12,
      .blinded_token_request = std::move(blinded_token_request)};
  ANON_TOKENS_ASSERT_OK_AND_ASSIGN(std::string encoded_token_request,
                                   MarshalTokenRequest(token_request));

  std::string expected_token_request_encoding;
  ASSERT_TRUE(absl::HexStringToBytes(
      "DA7A124ed3f2a25ec528543d9a83c850d12b3036b518fafec080df3efcd9693b944d0560"
      "5686200d6500f249475737ea9246a70c3c2a1ff280663e46c792a8ae0d9a6877d1b427bb"
      "ae7129b88c92ad61c08a9fe41629a642263e4857e428a706ba87659361fed38087c0e881"
      "f5e15668e0701d7edd63be98fcc7415819d466c61341de03d7e2a24181d7b9321b0826d5"
      "9402a87e08514f36cc45b0f7aac0e9a6578ddb0534c8ebe528c693b6efb54e76a5a8056f"
      "5c27d01ad42119953c5987b05c9ae2ca04b12838e641b4b1aac21e36f18573f603735fac"
      "1f8f611029e4cb76c8a5cc6f2c4143474e458c8d2ca8e9a71f01d90e0d2d784874ff000a"
      "e105483941652e",
      &expected_token_request_encoding));

  EXPECT_EQ(encoded_token_request, expected_token_request_encoding);

  ANON_TOKENS_ASSERT_OK_AND_ASSIGN(
      TokenRequest decoded_token_request,
      UnmarshalTokenRequest(encoded_token_request));

  EXPECT_EQ(decoded_token_request.token_type, token_request.token_type);
  EXPECT_EQ(decoded_token_request.truncated_token_key_id,
            token_request.truncated_token_key_id);
  EXPECT_EQ(decoded_token_request.blinded_token_request,
            token_request.blinded_token_request);
}

TEST(AnonymousTokensPrivacyPassTokenEncodingsTest,
     UnmarshalExtendedTokenRequestTooShort) {
  std::string extended_token_request_encoding_1;
  ASSERT_TRUE(absl::HexStringToBytes(
      "DA7A124ed3f2a25ec528543d9a83c850d12b3036b518fafec080df3efcd9693b944d0560"
      "5686200d6500f249475737ea9246a70c3c2a1ff280663e46c792a8ae0d9a6877d1b427bb"
      "ae7129b88c92ad61c08a9fe41629a642263e4857e428a706ba87659361fed38087c0e881"
      "f5e15668e0701d7edd63be98fcc7415819d466c61341de03d7e2a24181d7b9321b0826d5"
      "9402a87e08514f36cc45b0f7aac0e9a6578ddb0534c8ebe528c693b6efb54e76a5a8056f"
      "5c27d01ad42119953c5987b05c9ae2ca04b12838e641b4b1aac21e36f18573f603735fac"
      "1f8f611029e4cb76c8a5cc6f2c4143474e458c8d2ca8e9a71f01d90e0d2d784874ff000a"
      "e10548394165",
      &extended_token_request_encoding_1));
  std::string extended_token_request_encoding_2;
  ASSERT_TRUE(absl::HexStringToBytes(
      "DA7A124ed3f2a25ec528543d9a83c850d12b3036b518fafec080df3efcd9693b944d0560"
      "5686200d6500f249475737ea9246a70c3c2a1ff280663e46c792a8ae0d9a6877d1b427bb"
      "ae7129b88c92ad61c08a9fe41629a642263e4857e428a706ba87659361fed38087c0e881"
      "f5e15668e0701d7edd63be98fcc7415819d466c61341de03d7e2a24181d7b9321b0826d5"
      "9402a87e08514f36cc45b0f7aac0e9a6578ddb0534c8ebe528c693b6efb54e76a5a8056f"
      "5c27d01ad42119953c5987b05c9ae2ca04b12838e641b4b1aac21e36f18573f603735fac"
      "1f8f611029e4cb76c8a5cc6f2c4143474e458c8d2ca8e9a71f01d90e0d2d784874ff000a"
      "e105483941652e",
      &extended_token_request_encoding_2));

  absl::StatusOr<ExtendedTokenRequest> decoded_extended_token_request_1 =
      UnmarshalExtendedTokenRequest(extended_token_request_encoding_1);
  absl::StatusOr<ExtendedTokenRequest> decoded_extended_token_request_2 =
      UnmarshalExtendedTokenRequest(extended_token_request_encoding_2);

  EXPECT_EQ(decoded_extended_token_request_1.status().code(),
            absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(decoded_extended_token_request_1.status().message(),
              ::testing::HasSubstr("failed to read encoded_token_request"));
  EXPECT_EQ(decoded_extended_token_request_2.status().code(),
            absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(decoded_extended_token_request_2.status().message(),
              ::testing::HasSubstr("failed to read extensions."));
}

TEST(AnonymousTokensPrivacyPassTokenEncodingsTest,
     UnmarshalExtendedTokenRequestTooLong) {
  std::string extended_token_request_encoding;
  ASSERT_TRUE(absl::HexStringToBytes(
      "DA7A124ed3f2a25ec528543d9a83c850d12b3036b518fafec080df3efcd9693b944d0560"
      "5686200d6500f249475737ea9246a70c3c2a1ff280663e46c792a8ae0d9a6877d1b427bb"
      "ae7129b88c92ad61c08a9fe41629a642263e4857e428a706ba87659361fed38087c0e881"
      "f5e15668e0701d7edd63be98fcc7415819d466c61341de03d7e2a24181d7b9321b0826d5"
      "9402a87e08514f36cc45b0f7aac0e9a6578ddb0534c8ebe528c693b6efb54e76a5a8056f"
      "5c27d01ad42119953c5987b05c9ae2ca04b12838e641b4b1aac21e36f18573f603735fac"
      "1f8f611029e4cb76c8a5cc6f2c4143474e458c8d2ca8e9a71f01d90e0d2d784874ff000a"
      "e105483941652e000b0001000101000200020202DA",
      &extended_token_request_encoding));

  absl::StatusOr<ExtendedTokenRequest> decoded_extended_token_request =
      UnmarshalExtendedTokenRequest(extended_token_request_encoding);

  EXPECT_EQ(decoded_extended_token_request.status().code(),
            absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(decoded_extended_token_request.status().message(),
              ::testing::HasSubstr("no data after extensions is allowed"));
}

TEST(AnonymousTokensPrivacyPassTokenEncodingsTest,
     MarshalAndUnmarshalExtendedTokenRequest) {
  std::string blinded_token_request;
  ASSERT_TRUE(absl::HexStringToBytes(
      "4ed3f2a25ec528543d9a83c850d12b3036b518fafec080df3efcd9693b944d0560568620"
      "0d6500f249475737ea9246a70c3c2a1ff280663e46c792a8ae0d9a6877d1b427bbae7129"
      "b88c92ad61c08a9fe41629a642263e4857e428a706ba87659361fed38087c0e881f5e156"
      "68e0701d7edd63be98fcc7415819d466c61341de03d7e2a24181d7b9321b0826d59402a8"
      "7e08514f36cc45b0f7aac0e9a6578ddb0534c8ebe528c693b6efb54e76a5a8056f5c27d0"
      "1ad42119953c5987b05c9ae2ca04b12838e641b4b1aac21e36f18573f603735fac1f8f61"
      "1029e4cb76c8a5cc6f2c4143474e458c8d2ca8e9a71f01d90e0d2d784874ff000ae10548"
      "3941652e",
      &blinded_token_request));
  TokenRequest token_request{
      .token_type = 0xDA7A,
      .truncated_token_key_id = 0x12,
      .blinded_token_request = std::move(blinded_token_request)};
  std::string extension_value_1;
  ASSERT_TRUE(absl::HexStringToBytes("01", &extension_value_1));
  std::string extension_value_2;
  ASSERT_TRUE(absl::HexStringToBytes("0202", &extension_value_2));
  Extensions extensions;
  extensions.extensions.push_back(
      Extension{/*extension_type=*/0x0001, std::move(extension_value_1)});
  extensions.extensions.push_back(
      Extension{/*extension_type=*/0x0002, std::move(extension_value_2)});
  ExtendedTokenRequest extended_token_request{token_request, extensions};

  ANON_TOKENS_ASSERT_OK_AND_ASSIGN(
      std::string encoded_extended_token_request,
      MarshalExtendedTokenRequest(extended_token_request));

  std::string expected_extended_token_request_encoding;
  ASSERT_TRUE(absl::HexStringToBytes(
      "DA7A124ed3f2a25ec528543d9a83c850d12b3036b518fafec080df3efcd9693b944d0560"
      "5686200d6500f249475737ea9246a70c3c2a1ff280663e46c792a8ae0d9a6877d1b427bb"
      "ae7129b88c92ad61c08a9fe41629a642263e4857e428a706ba87659361fed38087c0e881"
      "f5e15668e0701d7edd63be98fcc7415819d466c61341de03d7e2a24181d7b9321b0826d5"
      "9402a87e08514f36cc45b0f7aac0e9a6578ddb0534c8ebe528c693b6efb54e76a5a8056f"
      "5c27d01ad42119953c5987b05c9ae2ca04b12838e641b4b1aac21e36f18573f603735fac"
      "1f8f611029e4cb76c8a5cc6f2c4143474e458c8d2ca8e9a71f01d90e0d2d784874ff000a"
      "e105483941652e000b0001000101000200020202",
      &expected_extended_token_request_encoding));

  EXPECT_EQ(encoded_extended_token_request,
            expected_extended_token_request_encoding);

  ANON_TOKENS_ASSERT_OK_AND_ASSIGN(
      ExtendedTokenRequest decoded_extended_token_request,
      UnmarshalExtendedTokenRequest(encoded_extended_token_request));

  EXPECT_EQ(decoded_extended_token_request.request.token_type,
            token_request.token_type);
  EXPECT_EQ(decoded_extended_token_request.request.truncated_token_key_id,
            token_request.truncated_token_key_id);
  EXPECT_EQ(decoded_extended_token_request.request.blinded_token_request,
            token_request.blinded_token_request);
  EXPECT_EQ(decoded_extended_token_request.extensions.extensions.size(),
            extensions.extensions.size());
  for (size_t i = 0; i < extensions.extensions.size(); ++i) {
    EXPECT_EQ(
        decoded_extended_token_request.extensions.extensions[i].extension_type,
        extensions.extensions[i].extension_type);
    EXPECT_EQ(
        decoded_extended_token_request.extensions.extensions[i].extension_value,
        extensions.extensions[i].extension_value);
  }
}

TEST(AnonymousTokensPrivacyPassTokenEncodingsTest,
     ValidateExtensionsValuesTest) {
  Extensions extensions;
  EXPECT_TRUE(ValidateExtensionsValues(extensions, absl::Now()).ok());

  ExpirationTimestamp et;
  absl::Time one_day_away = absl::Now() + absl::Hours(24);
  et.timestamp = absl::ToUnixSeconds(one_day_away);
  et.timestamp -= et.timestamp % 900;
  et.timestamp_precision = 900;
  ANON_TOKENS_ASSERT_OK_AND_ASSIGN(Extension ext, et.AsExtension());
  extensions.extensions.push_back(ext);
  EXPECT_TRUE(ValidateExtensionsValues(extensions, absl::Now()).ok());

  GeoHint gh;
  gh.geo_hint = "US,US-AL,ALABASTER";
  ANON_TOKENS_ASSERT_OK_AND_ASSIGN(ext, gh.AsExtension());
  extensions.extensions.push_back(ext);
  EXPECT_TRUE(ValidateExtensionsValues(extensions, absl::Now()).ok());

  ServiceType svc;
  svc.service_type_id = ServiceType::kChromeIpBlinding;
  ANON_TOKENS_ASSERT_OK_AND_ASSIGN(ext, svc.AsExtension());
  extensions.extensions.push_back(ext);
  EXPECT_TRUE(ValidateExtensionsValues(extensions, absl::Now()).ok());

  DebugMode debug;
  debug.mode = DebugMode::kDebug;
  ANON_TOKENS_ASSERT_OK_AND_ASSIGN(ext, debug.AsExtension());
  extensions.extensions.push_back(ext);
  EXPECT_TRUE(ValidateExtensionsValues(extensions, absl::Now()).ok());

  ProxyLayer proxy_layer;
  proxy_layer.layer = ProxyLayer::kProxyA;
  ANON_TOKENS_ASSERT_OK_AND_ASSIGN(ext, proxy_layer.AsExtension());
  extensions.extensions.push_back(ext);
  EXPECT_TRUE(ValidateExtensionsValues(extensions, absl::Now()).ok());
}

TEST(AnonymousTokensPrivacyPassTokenEncodingsTest,
     NotRoundedExpirationExtensionValidationTest) {
  Extensions extensions;
  EXPECT_TRUE(ValidateExtensionsValues(extensions, absl::Now()).ok());

  ExpirationTimestamp et;
  absl::Time one_day_away = absl::Now() + absl::Hours(24);
  et.timestamp = absl::ToUnixSeconds(one_day_away);
  et.timestamp -= et.timestamp % 900;
  et.timestamp += 17;
  et.timestamp_precision = 900;
  ANON_TOKENS_ASSERT_OK_AND_ASSIGN(Extension ext, et.AsExtension());
  extensions.extensions.push_back(ext);
  EXPECT_EQ(ValidateExtensionsValues(extensions, absl::Now()).code(),
            absl::StatusCode::kInvalidArgument);
}

TEST(AnonymousTokensPrivacyPassTokenEncodingsTest,
     BadPrecisionExpirationExtensionValidationTest) {
  Extensions extensions;
  EXPECT_TRUE(ValidateExtensionsValues(extensions, absl::Now()).ok());

  ExpirationTimestamp et;
  absl::Time one_day_away = absl::Now() + absl::Hours(24);
  et.timestamp = absl::ToUnixSeconds(one_day_away);
  et.timestamp -= et.timestamp % 2;
  et.timestamp_precision = 2;
  ANON_TOKENS_ASSERT_OK_AND_ASSIGN(Extension ext, et.AsExtension());
  extensions.extensions.push_back(ext);
  EXPECT_EQ(ValidateExtensionsValues(extensions, absl::Now()).code(),
            absl::StatusCode::kInvalidArgument);
}

TEST(AnonymousTokensPrivacyPassTokenEncodingsTest,
     MissingCountryGeoHintExtensionValidationTest) {
  Extensions extensions;
  GeoHint gh;
  gh.geo_hint = "US-AL,ALABASTER";
  ANON_TOKENS_ASSERT_OK_AND_ASSIGN(Extension ext, gh.AsExtension());
  extensions.extensions.push_back(ext);
  EXPECT_EQ(ValidateExtensionsValues(extensions, absl::Now()).code(),
            absl::StatusCode::kInvalidArgument);
}

TEST(AnonymousTokensPrivacyPassTokenEncodingsTest,
     CountryLowercaseGeoHintExtensionValidationTest) {
  Extensions extensions;
  GeoHint gh;
  gh.geo_hint = "us,US-AL,ALABASTER";
  ANON_TOKENS_ASSERT_OK_AND_ASSIGN(Extension ext, gh.AsExtension());
  extensions.extensions.push_back(ext);
  EXPECT_EQ(ValidateExtensionsValues(extensions, absl::Now()).code(),
            absl::StatusCode::kInvalidArgument);
}

TEST(AnonymousTokensPrivacyPassTokenEncodingsTest,
     NotCountryCodeGeoHintExtensionValidationTest) {
  Extensions extensions;
  GeoHint gh;
  gh.geo_hint = "USA,US-AL,ALABASTER";
  ANON_TOKENS_ASSERT_OK_AND_ASSIGN(Extension ext, gh.AsExtension());
  extensions.extensions.push_back(ext);
  EXPECT_EQ(ValidateExtensionsValues(extensions, absl::Now()).code(),
            absl::StatusCode::kInvalidArgument);
}

TEST(AnonymousTokensPrivacyPassTokenEncodingsTest,
     RegionLowercaseGeoHintExtensionValidationTest) {
  Extensions extensions;
  GeoHint gh;
  gh.geo_hint = "US,US-al,ALABASTER";
  ANON_TOKENS_ASSERT_OK_AND_ASSIGN(Extension ext, gh.AsExtension());
  extensions.extensions.push_back(ext);
  EXPECT_EQ(ValidateExtensionsValues(extensions, absl::Now()).code(),
            absl::StatusCode::kInvalidArgument);
}

TEST(AnonymousTokensPrivacyPassTokenEncodingsTest,
     ValidateExtensionsOrderAndValuesTest) {
  Extensions extensions;
  std::vector<uint16_t> expected_types;
  EXPECT_TRUE(ValidateExtensionsOrderAndValues(
                  extensions, absl::MakeSpan(expected_types), absl::Now())
                  .ok());

  ExpirationTimestamp et;
  absl::Time one_day_away = absl::Now() + absl::Hours(24);
  et.timestamp = absl::ToUnixSeconds(one_day_away);
  et.timestamp -= et.timestamp % 900;
  et.timestamp_precision = 900;
  ANON_TOKENS_ASSERT_OK_AND_ASSIGN(Extension ext, et.AsExtension());
  extensions.extensions.push_back(ext);
  expected_types.push_back(0x0001);
  EXPECT_TRUE(ValidateExtensionsOrderAndValues(
                  extensions, absl::MakeSpan(expected_types), absl::Now())
                  .ok());

  expected_types.push_back(0x0002);
  EXPECT_FALSE(ValidateExtensionsOrderAndValues(
                   extensions, absl::MakeSpan(expected_types), absl::Now())
                   .ok());

  GeoHint gh;
  gh.geo_hint = "US,US-AL,ALABASTER";
  ANON_TOKENS_ASSERT_OK_AND_ASSIGN(ext, gh.AsExtension());
  extensions.extensions.push_back(ext);
  EXPECT_TRUE(ValidateExtensionsOrderAndValues(
                  extensions, absl::MakeSpan(expected_types), absl::Now())
                  .ok());

  expected_types.clear();
  EXPECT_FALSE(ValidateExtensionsOrderAndValues(
                   extensions, absl::MakeSpan(expected_types), absl::Now())
                   .ok());

  expected_types.push_back(0x0002);
  expected_types.push_back(0x0001);
  EXPECT_FALSE(ValidateExtensionsOrderAndValues(
                   extensions, absl::MakeSpan(expected_types), absl::Now())
                   .ok());
}

}  // namespace
}  // namespace anonymous_tokens
