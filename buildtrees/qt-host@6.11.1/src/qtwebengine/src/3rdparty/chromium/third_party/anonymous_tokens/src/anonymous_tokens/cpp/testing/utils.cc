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
#include "anonymous_tokens/cpp/testing/utils.h"

#include <cstddef>
#include <cstdint>
#include <random>
#include <string>
#include <utility>
#include <vector>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/escaping.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"
#include "anonymous_tokens/cpp/crypto/constants.h"
#include "anonymous_tokens/cpp/crypto/crypto_utils.h"
#include "anonymous_tokens/cpp/shared/status_utils.h"
#include <openssl/base.h>
#include <openssl/bn.h>
#include <openssl/rsa.h>

namespace anonymous_tokens {

absl::StatusOr<std::string> TestSign(const absl::string_view blinded_data,
                                     RSA* rsa_key) {
  if (blinded_data.empty()) {
    return absl::InvalidArgumentError("blinded_data string is empty.");
  }
  const size_t mod_size = RSA_size(rsa_key);
  if (blinded_data.size() != mod_size) {
    return absl::InternalError(absl::StrCat(
        "Expected blind data size = ", mod_size,
        " actual blind data size = ", blinded_data.size(), " bytes."));
  }
  // Compute a raw RSA signature.
  std::string signature(mod_size, 0);
  size_t out_len;
  if (RSA_sign_raw(/*rsa=*/rsa_key, /*out_len=*/&out_len,
                   /*out=*/reinterpret_cast<uint8_t*>(&signature[0]),
                   /*max_out=*/mod_size,
                   /*in=*/reinterpret_cast<const uint8_t*>(&blinded_data[0]),
                   /*in_len=*/mod_size,
                   /*padding=*/RSA_NO_PADDING) != kBsslSuccess) {
    return absl::InternalError(
        "RSA_sign_raw failed when called from RsaBlindSigner::Sign");
  }
  if (out_len != mod_size || out_len != signature.size()) {
    return absl::InternalError(absl::StrCat(
        "Expected value of out_len and signature.size() = ", mod_size,
        " bytes, actual value of out_len and signature.size() = ", out_len,
        " and ", signature.size(), " bytes."));
  }
  return signature;
}

absl::StatusOr<std::string> TestSignWithPublicMetadata(
    const absl::string_view blinded_data, absl::string_view public_metadata,
    const RSA& rsa_key, const bool use_rsa_public_exponent) {
  if (blinded_data.empty()) {
    return absl::InvalidArgumentError("blinded_data string is empty.");
  } else if (blinded_data.size() != RSA_size(&rsa_key)) {
    return absl::InternalError(absl::StrCat(
        "Expected blind data size = ", RSA_size(&rsa_key),
        " actual blind data size = ", blinded_data.size(), " bytes."));
  }
  // Compute new public exponent using the public metadata.
  bssl::UniquePtr<BIGNUM> new_e;
  if (use_rsa_public_exponent) {
    ANON_TOKENS_ASSIGN_OR_RETURN(
        new_e,
        ComputeExponentWithPublicMetadataAndPublicExponent(
            *RSA_get0_n(&rsa_key), *RSA_get0_e(&rsa_key), public_metadata));
  } else {
    ANON_TOKENS_ASSIGN_OR_RETURN(
        new_e, ComputeExponentWithPublicMetadata(*RSA_get0_n(&rsa_key),
                                                 public_metadata));
  }

  // Compute phi(p) = p-1
  ANON_TOKENS_ASSIGN_OR_RETURN(bssl::UniquePtr<BIGNUM> phi_p, NewBigNum());
  if (BN_sub(phi_p.get(), RSA_get0_p(&rsa_key), BN_value_one()) != 1) {
    return absl::InternalError(
        absl::StrCat("Unable to compute phi(p): ", GetSslErrors()));
  }
  // Compute phi(q) = q-1
  ANON_TOKENS_ASSIGN_OR_RETURN(bssl::UniquePtr<BIGNUM> phi_q, NewBigNum());
  if (BN_sub(phi_q.get(), RSA_get0_q(&rsa_key), BN_value_one()) != 1) {
    return absl::InternalError(
        absl::StrCat("Unable to compute phi(q): ", GetSslErrors()));
  }
  // Compute phi(n) = phi(p)*phi(q)
  ANON_TOKENS_ASSIGN_OR_RETURN(auto ctx, GetAndStartBigNumCtx());
  ANON_TOKENS_ASSIGN_OR_RETURN(bssl::UniquePtr<BIGNUM> phi_n, NewBigNum());
  if (BN_mul(phi_n.get(), phi_p.get(), phi_q.get(), ctx.get()) != 1) {
    return absl::InternalError(
        absl::StrCat("Unable to compute phi(n): ", GetSslErrors()));
  }
  // Compute lcm(phi(p), phi(q)).
  ANON_TOKENS_ASSIGN_OR_RETURN(bssl::UniquePtr<BIGNUM> lcm, NewBigNum());
  if (BN_rshift1(lcm.get(), phi_n.get()) != 1) {
    return absl::InternalError(absl::StrCat(
        "Could not compute LCM(phi(p), phi(q)): ", GetSslErrors()));
  }

  // Compute the new private exponent new_d
  ANON_TOKENS_ASSIGN_OR_RETURN(bssl::UniquePtr<BIGNUM> new_d, NewBigNum());
  if (!BN_mod_inverse(new_d.get(), new_e.get(), lcm.get(), ctx.get())) {
    return absl::InternalError(
        absl::StrCat("Could not compute private exponent d: ", GetSslErrors()));
  }

  // Compute new_dpm1 = new_d mod p-1
  ANON_TOKENS_ASSIGN_OR_RETURN(bssl::UniquePtr<BIGNUM> new_dpm1, NewBigNum());
  BN_mod(new_dpm1.get(), new_d.get(), phi_p.get(), ctx.get());
  // Compute new_dqm1 = new_d mod q-1
  ANON_TOKENS_ASSIGN_OR_RETURN(bssl::UniquePtr<BIGNUM> new_dqm1, NewBigNum());
  BN_mod(new_dqm1.get(), new_d.get(), phi_q.get(), ctx.get());

  bssl::UniquePtr<RSA> derived_private_key(RSA_new_private_key_large_e(
      RSA_get0_n(&rsa_key), new_e.get(), new_d.get(), RSA_get0_p(&rsa_key),
      RSA_get0_q(&rsa_key), new_dpm1.get(), new_dqm1.get(),
      RSA_get0_iqmp(&rsa_key)));
  if (!derived_private_key.get()) {
    return absl::InternalError(
        absl::StrCat("RSA_new_private_key_large_e failed: ", GetSslErrors()));
  }

  return TestSign(blinded_data, derived_private_key.get());
}

IetfStandardRsaBlindSignatureTestVector
GetIetfStandardRsaBlindSignatureTestVector() {
  bool success = true;
  std::string n;
  success &= absl::HexStringToBytes(
      "aec4d69addc70b990ea66a5e70603b6fee27aafebd08f2d94cbe1250c556e047a928d635"
      "c3f45ee9b66d1bc628a03bac9b7c3f416fe20dabea8f3d7b4bbf7f963be335d2328d67e6"
      "c13ee4a8f955e05a3283720d3e1f139c38e43e0338ad058a9495c53377fc35be64d208f8"
      "9b4aa721bf7f7d3fef837be2a80e0f8adf0bcd1eec5bb040443a2b2792fdca522a7472ae"
      "d74f31a1ebe1eebc1f408660a0543dfe2a850f106a617ec6685573702eaaa21a5640a5dc"
      "af9b74e397fa3af18a2f1b7c03ba91a6336158de420d63188ee143866ee415735d155b7c"
      "2d854d795b7bc236cffd71542df34234221a0413e142d8c61355cc44d45bda9420497455"
      "7ac2704cd8b593f035a5724b1adf442e78c542cd4414fce6f1298182fb6d8e53cef1adfd"
      "2e90e1e4deec52999bdc6c29144e8d52a125232c8c6d75c706ea3cc06841c7bda33568c6"
      "3a6c03817f722b50fcf898237d788a4400869e44d90a3020923dc646388abcc914315215"
      "fcd1bae11b1c751fd52443aac8f601087d8d42737c18a3fa11ecd4131ecae017ae0a14ac"
      "fc4ef85b83c19fed33cfd1cd629da2c4c09e222b398e18d822f77bb378dea3cb360b605e"
      "5aa58b20edc29d000a66bd177c682a17e7eb12a63ef7c2e4183e0d898f3d6bf567ba8ae8"
      "4f84f1d23bf8b8e261c3729e2fa6d07b832e07cddd1d14f55325c6f924267957121902dc"
      "19b3b32948bdead5",
      &n);
  std::string e;
  success &= absl::HexStringToBytes("010001", &e);
  std::string d;
  success &= absl::HexStringToBytes(
      "0d43242aefe1fb2c13fbc66e20b678c4336d20b1808c558b6e62ad16a287077180b177e1"
      "f01b12f9c6cd6c52630257ccef26a45135a990928773f3bd2fc01a313f1dac97a51cec71"
      "cb1fd7efc7adffdeb05f1fb04812c924ed7f4a8269925dad88bd7dcfbc4ef01020ebfc60"
      "cb3e04c54f981fdbd273e69a8a58b8ceb7c2d83fbcbd6f784d052201b88a9848186f2a45"
      "c0d2826870733e6fd9aa46983e0a6e82e35ca20a439c5ee7b502a9062e1066493bdadf8b"
      "49eb30d9558ed85abc7afb29b3c9bc644199654a4676681af4babcea4e6f71fe4565c9c1"
      "b85d9985b84ec1abf1a820a9bbebee0df1398aae2c85ab580a9f13e7743afd3108eb3210"
      "0b870648fa6bc17e8abac4d3c99246b1f0ea9f7f93a5dd5458c56d9f3f81ff2216b3c368"
      "0a13591673c43194d8e6fc93fc1e37ce2986bd628ac48088bc723d8fbe293861ca7a9f4a"
      "73e9fa63b1b6d0074f5dea2a624c5249ff3ad811b6255b299d6bc5451ba7477f19c5a0db"
      "690c3e6476398b1483d10314afd38bbaf6e2fbdbcd62c3ca9797a420ca6034ec0a83360a"
      "3ee2adf4b9d4ba29731d131b099a38d6a23cc463db754603211260e99d19affc902c915d"
      "7854554aabf608e3ac52c19b8aa26ae042249b17b2d29669b5c859103ee53ef9bdc73ba3"
      "c6b537d5c34b6d8f034671d7f3a8a6966cc4543df223565343154140fd7391c7e7be03e2"
      "41f4ecfeb877a051",
      &d);
  std::string p;
  success &= absl::HexStringToBytes(
      "e1f4d7a34802e27c7392a3cea32a262a34dc3691bd87f3f310dc75673488930559c120fd"
      "0410194fb8a0da55bd0b81227e843fdca6692ae80e5a5d414116d4803fca7d8c30eaaae5"
      "7e44a1816ebb5c5b0606c536246c7f11985d731684150b63c9a3ad9e41b04c0b5b27cb18"
      "8a692c84696b742a80d3cd00ab891f2457443dadfeba6d6daf108602be26d7071803c671"
      "05a5426838e6889d77e8474b29244cefaf418e381b312048b457d73419213063c60ee7b0"
      "d81820165864fef93523c9635c22210956e53a8d96322493ffc58d845368e2416e078e5b"
      "cb5d2fd68ae6acfa54f9627c42e84a9d3f2774017e32ebca06308a12ecc290c7cd1156dc"
      "ccfb2311",
      &p);
  std::string q;
  success &= absl::HexStringToBytes(
      "c601a9caea66dc3835827b539db9df6f6f5ae77244692780cd334a006ab353c806426b60"
      "718c05245650821d39445d3ab591ed10a7339f15d83fe13f6a3dfb20b9452c6a9b42eaa6"
      "2a68c970df3cadb2139f804ad8223d56108dfde30ba7d367e9b0a7a80c4fdba2fd9dde66"
      "61fc73fc2947569d2029f2870fc02d8325acf28c9afa19ecf962daa7916e21afad09eb62"
      "fe9f1cf91b77dc879b7974b490d3ebd2e95426057f35d0a3c9f45f79ac727ab81a519a8b"
      "9285932d9b2e5ccd347e59f3f32ad9ca359115e7da008ab7406707bd0e8e185a5ed8758b"
      "5ba266e8828f8d863ae133846304a2936ad7bc7c9803879d2fc4a28e69291d73dbd799f8"
      "bc238385",
      &q);
  std::string message;
  success &= absl::HexStringToBytes(
      "8f3dc6fb8c4a02f4d6352edf0907822c1210a9b32f9bdda4c45a698c80023aa6b59f8cfe"
      "c5fdbb36331372ebefedae7d",
      &message);
  std::string salt;
  success &= absl::HexStringToBytes(
      "051722b35f458781397c3a671a7d3bd3096503940e4c4f1aaa269d60300ce449555cd734"
      "0100df9d46944c5356825abf",
      &salt);
  std::string inv;
  success &= absl::HexStringToBytes(
      "80682c48982407b489d53d1261b19ec8627d02b8cda5336750b8cee332ae260de57b02d7"
      "2609c1e0e9f28e2040fc65b6f02d56dbd6aa9af8fde656f70495dfb723ba01173d4707a1"
      "2fddac628ca29f3e32340bd8f7ddb557cf819f6b01e445ad96f874ba235584ee71f6581f"
      "62d4f43bf03f910f6510deb85e8ef06c7f09d9794a008be7ff2529f0ebb69decef646387"
      "dc767b74939265fec0223aa6d84d2a8a1cc912d5ca25b4e144ab8f6ba054b54910176d57"
      "37a2cff011da431bd5f2a0d2d66b9e70b39f4b050e45c0d9c16f02deda9ddf2d00f3e4b0"
      "1037d7029cd49c2d46a8e1fc2c0c17520af1f4b5e25ba396afc4cd60c494a4c426448b35"
      "b49635b337cfb08e7c22a39b256dd032c00adddafb51a627f99a0e1704170ac1f1912e49"
      "d9db10ec04c19c58f420212973e0cb329524223a6aa56c7937c5dffdb5d966b6cd4cbc26"
      "f3201dd25c80960a1a111b32947bb78973d269fac7f5186530930ed19f68507540eed9e1"
      "bab8b00f00d8ca09b3f099aae46180e04e3584bd7ca054df18a1504b89d1d1675d0966c4"
      "ae1407be325cdf623cf13ff13e4a28b594d59e3eadbadf6136eee7a59d6a444c9eb4e219"
      "8e8a974f27a39eb63af2c9af3870488b8adaad444674f512133ad80b9220e09158521614"
      "f1faadfe8505ef57b7df6813048603f0dd04f4280177a11380fbfc861dbcbd7418d62155"
      "248dad5fdec0991f",
      &inv);
  std::string encoded_message;
  success &= absl::HexStringToBytes(
      "6e0c464d9c2f9fbc147b43570fc4f238e0d0b38870b3addcf7a4217df912ccef17a7f629"
      "aa850f63a063925f312d61d6437be954b45025e8282f9c0b1131bc8ff19a8a928d859b37"
      "113db1064f92a27f64761c181c1e1f9b251ae5a2f8a4047573b67a270584e089beadcb13"
      "e7c82337797119712e9b849ff56e04385d144d3ca9d8d92bf78adb20b5bbeb3685f17038"
      "ec6afade3ef354429c51c687b45a7018ee3a6966b3af15c9ba8f40e6461ba0a17ef5a799"
      "672ad882bab02b518f9da7c1a962945c2e9b0f02f29b31b9cdf3e633f9d9d2a22e96e1de"
      "28e25241ca7dd04147112f578973403e0f4fd80865965475d22294f065e17a1c4a201de9"
      "3bd14223e6b1b999fd548f2f759f52db71964528b6f15b9c2d7811f2a0a35d534b821630"
      "1c47f4f04f412cae142b48c4cdff78bc54df690fd43142d750c671dd8e2e938e6a440b2f"
      "825b6dbb3e19f1d7a3c0150428a47948037c322365b7fe6fe57ac88d8f80889e9ff38177"
      "bad8c8d8d98db42908b389cb59692a58ce275aa15acb032ca951b3e0a3404b7f33f655b7"
      "c7d83a2f8d1b6bbff49d5fcedf2e030e80881aa436db27a5c0dea13f32e7d460dbf01240"
      "c2320c2bb5b3225b17145c72d61d47c8f84d1e19417ebd8ce3638a82d395cc6f7050b620"
      "9d9283dc7b93fecc04f3f9e7f566829ac41568ef799480c733c09759aa9734e2013d7640"
      "dc6151018ea902bc",
      &encoded_message);
  std::string blinded_message;
  success &= absl::HexStringToBytes(
      "10c166c6a711e81c46f45b18e5873cc4f494f003180dd7f115585d871a28930259654fe2"
      "8a54dab319cc5011204c8373b50a57b0fdc7a678bd74c523259dfe4fd5ea9f52f170e19d"
      "fa332930ad1609fc8a00902d725cfe50685c95e5b2968c9a2828a21207fcf393d15f8497"
      "69e2af34ac4259d91dfd98c3a707c509e1af55647efaa31290ddf48e0133b798562af5ea"
      "bd327270ac2fb6c594734ce339a14ea4fe1b9a2f81c0bc230ca523bda17ff42a377266bc"
      "2778a274c0ae5ec5a8cbbe364fcf0d2403f7ee178d77ff28b67a20c7ceec009182dbcaa9"
      "bc99b51ebbf13b7d542be337172c6474f2cd3561219fe0dfa3fb207cff89632091ab841c"
      "f38d8aa88af6891539f263adb8eac6402c41b6ebd72984e43666e537f5f5fe27b2b5aa11"
      "4957e9a580730308a5f5a9c63a1eb599f093ab401d0c6003a451931b6d12418030570584"
      "5060ebba6b0036154fcef3e5e9f9e4b87e8f084542fd1dd67e7782a5585150181c01eb6d"
      "90cb95883837384a5b91dbb606f266059ecc51b5acbaa280e45cfd2eec8cc1cdb1b7211c"
      "8e14805ba683f9b78824b2eb005bc8a7d7179a36c152cb87c8219e5569bba911bb32a1b9"
      "23ca83de0e03fb10fba75d85c55907dda5a2606bf918b056c3808ba496a4d95532212040"
      "a5f44f37e1097f26dc27b98a51837daa78f23e532156296b64352669c94a8a855acf3053"
      "3d8e0594ace7c442",
      &blinded_message);
  std::string blinded_signature;
  success &= absl::HexStringToBytes(
      "364f6a40dbfbc3bbb257943337eeff791a0f290898a6791283bba581d9eac90a6376a837"
      "241f5f73a78a5c6746e1306ba3adab6067c32ff69115734ce014d354e2f259d4cbfb8902"
      "44fd451a497fe6ecf9aa90d19a2d441162f7eaa7ce3fc4e89fd4e76b7ae585be2a2c0fd6"
      "fb246b8ac8d58bcb585634e30c9168a434786fe5e0b74bfe8187b47ac091aa571ffea0a8"
      "64cb906d0e28c77a00e8cd8f6aba4317a8cc7bf32ce566bd1ef80c64de041728abe087be"
      "e6cadd0b7062bde5ceef308a23bd1ccc154fd0c3a26110df6193464fc0d24ee189aea897"
      "9d722170ba945fdcce9b1b4b63349980f3a92dc2e5418c54d38a862916926b3f9ca270a8"
      "cf40dfb9772bfbdd9a3e0e0892369c18249211ba857f35963d0e05d8da98f1aa0c6bba58"
      "f47487b8f663e395091275f82941830b050b260e4767ce2fa903e75ff8970c98bfb3a08d"
      "6db91ab1746c86420ee2e909bf681cac173697135983c3594b2def673736220452fde4dd"
      "ec867d40ff42dd3da36c84e3e52508b891a00f50b4f62d112edb3b6b6cc3dbd546ba10f3"
      "6b03f06c0d82aeec3b25e127af545fac28e1613a0517a6095ad18a98ab79f68801e05c17"
      "5e15bae21f821e80c80ab4fdec6fb34ca315e194502b8f3dcf7892b511aee45060e3994c"
      "d15e003861bc7220a2babd7b40eda03382548a34a7110f9b1779bf3ef6011361611e6bc5"
      "c0dc851e1509de1a",
      &blinded_signature);
  std::string signature;
  success &= absl::HexStringToBytes(
      "6fef8bf9bc182cd8cf7ce45c7dcf0e6f3e518ae48f06f3c670c649ac737a8b8119a34d51"
      "641785be151a697ed7825fdfece82865123445eab03eb4bb91cecf4d6951738495f84811"
      "51b62de869658573df4e50a95c17c31b52e154ae26a04067d5ecdc1592c287550bb982a5"
      "bb9c30fd53a768cee6baabb3d483e9f1e2da954c7f4cf492fe3944d2fe456c1ecaf08403"
      "69e33fb4010e6b44bb1d721840513524d8e9a3519f40d1b81ae34fb7a31ee6b7ed641cb1"
      "6c2ac999004c2191de0201457523f5a4700dd649267d9286f5c1d193f1454c9f868a5781"
      "6bf5ff76c838a2eeb616a3fc9976f65d4371deecfbab29362caebdff69c635fe5a2113da"
      "4d4d8c24f0b16a0584fa05e80e607c5d9a2f765f1f069f8d4da21f27c2a3b5c984b4ab24"
      "899bef46c6d9323df4862fe51ce300fca40fb539c3bb7fe2dcc9409e425f2d3b95e70e9c"
      "49c5feb6ecc9d43442c33d50003ee936845892fb8be475647da9a080f5bc7f8a716590b3"
      "745c2209fe05b17992830ce15f32c7b22cde755c8a2fe50bd814a0434130b807dc1b7218"
      "d4e85342d70695a5d7f29306f25623ad1e8aa08ef71b54b8ee447b5f64e73d09bdd6c3b7"
      "ca224058d7c67cc7551e9241688ada12d859cb7646fbd3ed8b34312f3b49d69802f0eaa1"
      "1bc4211c2f7a29cd5c01ed01a39001c5856fab36228f5ee2f2e1110811872fe7c865c42e"
      "d59029c706195d52",
      &signature);
  if (!success) {
    return {};
  }
  IetfStandardRsaBlindSignatureTestVector test_vector = {
      std::move(n),
      std::move(e),
      std::move(d),
      std::move(p),
      std::move(q),
      std::move(message),
      std::move(salt),
      std::move(inv),
      std::move(encoded_message),
      std::move(blinded_message),
      std::move(blinded_signature),
      std::move(signature)};
  return test_vector;
}

std::vector<IetfRsaBlindSignatureWithPublicMetadataTestVector>
GetIetfRsaBlindSignatureWithPublicMetadataTestVectors() {
  bool success = true;
  std::string n;
  success &= absl::HexStringToBytes(
      "d6930820f71fe517bf3259d14d40209b02a5c0d3d61991c731dd7da39f8d69821552e231"
      "8d6c9ad897e603887a476ea3162c1205da9ac96f02edf31df049bd55f142134c17d4382a"
      "0e78e275345f165fbe8e49cdca6cf5c726c599dd39e09e75e0f330a33121e73976e4facb"
      "a9cfa001c28b7c96f8134f9981db6750b43a41710f51da4240fe03106c12acb1e7bb53d7"
      "5ec7256da3fddd0718b89c365410fce61bc7c99b115fb4c3c318081fa7e1b65a37774e8e"
      "50c96e8ce2b2cc6b3b367982366a2bf9924c4bafdb3ff5e722258ab705c76d43e5f1f121"
      "b984814e98ea2b2b8725cd9bc905c0bc3d75c2a8db70a7153213c39ae371b2b5dc1dafcb"
      "19d6fae9",
      &n);
  std::string e;
  success &= absl::HexStringToBytes("010001", &e);
  std::string d;
  success &= absl::HexStringToBytes(
      "4e21356983722aa1adedb084a483401c1127b781aac89eab103e1cfc52215494981d18dd"
      "8028566d9d499469c25476358de23821c78a6ae43005e26b394e3051b5ca206aa9968d68"
      "cae23b5affd9cbb4cb16d64ac7754b3cdba241b72ad6ddfc000facdb0f0dd03abd4efcfe"
      "e1730748fcc47b7621182ef8af2eeb7c985349f62ce96ab373d2689baeaea0e28ea7d45f"
      "2d605451920ca4ea1f0c08b0f1f6711eaa4b7cca66d58a6b916f9985480f90aca9721068"
      "5ac7b12d2ec3e30a1c7b97b65a18d38a93189258aa346bf2bc572cd7e7359605c20221b8"
      "909d599ed9d38164c9c4abf396f897b9993c1e805e574d704649985b600fa0ced8e54270"
      "71d7049d",
      &d);
  std::string p;
  success &= absl::HexStringToBytes(
      "dcd90af1be463632c0d5ea555256a20605af3db667475e190e3af12a34a3324c46a30940"
      "62c59fb4b249e0ee6afba8bee14e0276d126c99f4784b23009bf6168ff628ac1486e5ae8"
      "e23ce4d362889de4df63109cbd90ef93db5ae64372bfe1c55f832766f21e94ea3322eb21"
      "82f10a891546536ba907ad74b8d72469bea396f3",
      &p);
  std::string q;
  success &= absl::HexStringToBytes(
      "f8ba5c89bd068f57234a3cf54a1c89d5b4cd0194f2633ca7c60b91a795a56fa8c8686c0e"
      "37b1c4498b851e3420d08bea29f71d195cfbd3671c6ddc49cf4c1db5b478231ea9d91377"
      "ffa98fe95685fca20ba4623212b2f2def4da5b281ed0100b651f6db32112e4017d831c0d"
      "a668768afa7141d45bbc279f1e0f8735d74395b3",
      &q);
  std::string message_1;
  success &= absl::HexStringToBytes("68656c6c6f20776f726c64", &message_1);
  std::string public_metadata_1;
  success &= absl::HexStringToBytes("6d65746164617461", &public_metadata_1);
  std::string message_mask_1;
  success &= absl::HexStringToBytes(
      "64b5c5d2b2ca672690df59bab774a389606d85d56f92a18a57c42eb4cb164d43",
      &message_mask_1);
  std::string blinded_message_1;
  success &= absl::HexStringToBytes(
      "1b9e1057dd2d05a17ad2feba5f87a4083cc825fe06fc70f0b782062ea0043fa65ec8096c"
      "e5d403cfa2aa3b11195b2a655d694386058f6266450715a936b5764f42977c0a0933ff30"
      "54d456624734fd2c019def792f00d30b3ac2f27859ea56d835f80564a3ba59f3c876dc92"
      "6b2a785378ca83f177f7b378513b36a074e7db59448fd4007b54c64791a33b61721ab3b5"
      "476165193af30f25164d480684d045a8d0782a53dd73774563e8d29e48b175534f696763"
      "abaab49fa03a055ec9246c5e398a5563cc88d02eb57d725d3fc9231ae5139aa7fcb99410"
      "60b0bf0192b8c81944fa0c54568b0ab4ea9c4c4c9829d6dbcbf8b48006b322ee51d784ac"
      "93e4bf13",
      &blinded_message_1);
  std::string blinded_signature_1;
  success &= absl::HexStringToBytes(
      "7ef75d9887f29f2232602acab43263afaea70313a0c90374388df5a7a7440d2584c4b4e5"
      "b886accc065bf4824b4b22370ddde7fea99d4cd67f8ed2e4a6a2b7b5869e8d4d0c523183"
      "20c5bf7b9f02bb132af7365c471e799edd111ca9441934c7db76c164b0515afc5607b8ce"
      "b584f5b1d2177d5180e57218265c07aec9ebde982f3961e7ddaa432e47297884da8f4512"
      "fe3dc9ab820121262e6a73850920299999c293b017cd800c6ec994f76b6ace35ff4232f9"
      "502e6a52262e19c03de7cc27d95ccbf4c381d698fcfe1f200209814e04ae2d6279883015"
      "bbf36cabf3e2350be1e175020ee9f4bb861ba409b467e23d08027a699ac36b2e5ab98839"
      "0f3c0ee9",
      &blinded_signature_1);
  std::string signature_1;
  success &= absl::HexStringToBytes(
      "abd6813bb4bbe3bc8dc9f8978655b22305e5481b35c5bdc4869b60e2d5cc74b84356416a"
      "baaca0ca8602cd061248587f0d492fee3534b19a3fe089de18e4df9f3a6ad289afb5323d"
      "7934487b8fafd25943766072bab873fa9cd69ce7328a57344c2c529fe96983ca701483ca"
      "353a98a1a9610391b7d32b13e14e8ef87d04c0f56a724800655636cfff280d35d6b468f6"
      "8f09f56e1b3acdb46bc6634b7a1eab5c25766cec3b5d97c37bbca302286c17ff557bcf1a"
      "4a0e342ea9b2713ab7f935c8174377bace2e5926b39834079761d9121f5df1fad47a51b0"
      "3eab3d84d050c99cf1f68718101735267cca3213c0a46c0537887ffe92ca05371e26d587"
      "313cc3f4",
      &signature_1);

  std::vector<IetfRsaBlindSignatureWithPublicMetadataTestVector> test_vectors;
  // test_vector 1.
  test_vectors.push_back({
      n,
      e,
      d,
      p,
      q,
      std::move(message_1),
      std::move(public_metadata_1),
      std::move(message_mask_1),
      std::move(blinded_message_1),
      std::move(blinded_signature_1),
      std::move(signature_1),
  });

  std::string message_2;
  success &= absl::HexStringToBytes("68656c6c6f20776f726c64", &message_2);
  std::string message_mask_2;
  success &= absl::HexStringToBytes(
      "ebb56541b9a1758028033cfb085a4ffe048f072c6c82a71ce21d40842b5c0a89",
      &message_mask_2);
  std::string blinded_message_2;
  success &= absl::HexStringToBytes(
      "d1fc97f30efbf116fadd9895130cdd55f939211f7db19ce9a85287227a02b33fb698b523"
      "99f81be0e1f598482000202ec89968085753eae1810f14676b514e08238c8aa79d8b999a"
      "f54e9f4282c6220d4d760716e48e5413f3228cc59ce10b8252916640de7b9b5c7dc9c2bf"
      "f9f53b4fb5eb4a5f8bab49af3fd1b955d34312073d15030e7fdb44bdb23460d1c5662597"
      "f9947092def7fff955a5f3e63419ae9858c6405f9609b63c4331e0cf90d24c196bee554f"
      "2b78e0d8f6da3d4308c8d4ae9fbe18a8bb7fa4fc3b9cacd4263e5bd6e12ed891cfdfba8b"
      "50d0f37d7a9abe065238367907c685ed2c224924caf5d8fe41f5db898b09a0501d318d9f"
      "65d88cb8",
      &blinded_message_2);
  std::string blinded_signature_2;
  success &= absl::HexStringToBytes(
      "400c1bcdfa56624f15d04f6954908b5605dbeff4cd56f384d7531669970290d706529d44"
      "cde4c972a1399635525a2859ef1d914b4130068ed407cfda3bd9d1259790a30f6d8c07d1"
      "90aa98bf21ae9581e5d61801565d96e9eec134335958b3d0b905739e2fd9f39074da08f8"
      "69089fe34de2d218062afa16170c1505c67b65af4dcc2f1aeccd48275c3dacf96116557b"
      "7f8c7044d84e296a0501c511ba1e6201703e1dd834bf47a96e1ac4ec9b935233ed751239"
      "bd4b514b031522cd51615c1555e520312ed1fa43f55d4abeb222ee48b4746c7900696659"
      "0004714039bac7fd18cdd54761924d91a4648e871458937061ef6549dd12d76e37ed4176"
      "34d88914",
      &blinded_signature_2);
  std::string signature_2;
  success &= absl::HexStringToBytes(
      "4062960edb71cc071e7d101db4f595aae4a98e0bfe6843aca3e5f48c9dfb46d505e8c198"
      "06ffa07f040313d44d0996ef9f69a86fa5946cb818a32627fe2df2a0e80350288ae4fedf"
      "bee4193554cc1433d9d27639db8b4635265504d87dca7054c85e0c882d32887534405e6c"
      "c4e7eb4b174383e5ce4eebbfffb217f353102f6d1a0461ef89238de31b0a0c134dfac0d2"
      "a8c533c807ccdd557c6510637596a490d5258b77410421be4076ecdf2d7e9044327e36e3"
      "49751f3239681bba10fe633f1b246f5a9f694706316898c900af2294f47267f2e9ad1e61"
      "c7f56bf643280258875d29f3745dfdb74b9bbcd5fe3dea62d9be85e2c6f5aed68bc79f8b"
      "4a27b3de",
      &signature_2);
  // test_vector 2.
  test_vectors.push_back({n, e, d, p, q, std::move(message_2),
                          /*public_metadata=*/"", std::move(message_mask_2),
                          std::move(blinded_message_2),
                          std::move(blinded_signature_2),
                          std::move(signature_2)});

  std::string public_metadata_3;
  success &= absl::HexStringToBytes("6d65746164617461", &public_metadata_3);
  std::string message_mask_3;
  success &= absl::HexStringToBytes(
      "f2a4ed7c5aa338430c7026d7d92017f994ca1c8b123b236dae8666b1899059d0",
      &message_mask_3);
  std::string blinded_message_3;
  success &= absl::HexStringToBytes(
      "7756a1f89fa33cfc083567e02fd865d07d6e5cd4943f030a2f94b5c23f3fe79c83c49c59"
      "4247d02885e2cd161638cff60803184c9e802a659d76a1c53340972e62e728cc70cf684e"
      "f03ce2d05cefc729e6eee2ae46afa17b6b27a64f91e4c46cc12adc58d9cb61a4306dac73"
      "2c9789199cfe8bd28359d1911678e9709bc159dae34ac7aa59fd0c95962c9f4904bf04aa"
      "ba8a7e774735bd03be4a02fb0864a53354a2e2f3502506318a5b03961366005c7b120f0e"
      "6b87b44bc15658c3e8985d69f6adea38c24fe5e7b4bafa1ad6dc7d729281c26dffc88bd3"
      "4fcc5a5f9df9b9781f99ea47472ba8bd679aaada59525b978ebc8a3ea2161de84b7398e4"
      "878b751b",
      &blinded_message_3);
  std::string blinded_signature_3;
  success &= absl::HexStringToBytes(
      "2a13f73e4e255a9d5bc6f76cf48dfbf189581c2b170600fd3ab1a3def148846213239b9d"
      "0a981537541cb4f481a602aeebca9ef28c9fcdc63d15d4296f85d864f799edf08e904518"
      "0571ce1f1d3beff293b18aae9d8845068cc0d9a05b822295042dc56a1a2b604c51aa65fd"
      "89e6d163fe1eac63cf603774797b7936a8b7494d43fa37039d3777b8e57cf0d95227ab29"
      "d0bd9c01b3eae9dde5fca7141919bd83a17f9b1a3b401507f3e3a8e8a2c8eb6c5c1921a7"
      "81000fee65b6dd851d53c89cba2c3375f0900001c04855949b7fa499f2a78089a6f0c9b4"
      "d36fdfcac2d846076736c5eaedaf0ae70860633e51b0de21d96c8b43c600afa2e4cc64cd"
      "66d77a8f",
      &blinded_signature_3);
  std::string signature_3;
  success &= absl::HexStringToBytes(
      "67985949f4e7c91edd5647223170d2a9b6611a191ca48ceadb6c568828b4c415b6270b03"
      "7cd8a68b5bca1992eb769aaef04549422889c8b156b9378c50e8a31c07dc1fe0a80d25b8"
      "70fadbcc1435197f0a31723740f3084ecb4e762c623546f6bd7d072aa565bc2105b95424"
      "4a2b03946c7d4093ba1216ec6bb65b8ca8d2f3f3c43468e80b257c54a2c2ea15f640a081"
      "83a00488c7772b10df87232ee7879bee93d17e194d6b703aeceb348c1b02ec7ce202086b"
      "6494f96a0f2d800f12e855f9c33dcd3abf6bd8044efd69d4594a974d6297365479fe6c11"
      "f6ecc5ea333031c57deb6e14509777963a25cdf8db62d6c8c68aa038555e4e3ae4411b28"
      "e43c8f57",
      &signature_3);
  // test_vector 3.
  test_vectors.push_back({
      n,
      e,
      d,
      p,
      q,
      /*message=*/"",
      std::move(public_metadata_3),
      std::move(message_mask_3),
      std::move(blinded_message_3),
      std::move(blinded_signature_3),
      std::move(signature_3),
  });

  std::string message_mask_4;
  success &= absl::HexStringToBytes(
      "ba3ea4b1e475eebe11d4bfe3a48521d3ba8cd62f3baed9ec29fbbf7ff0478bc0",
      &message_mask_4);
  std::string blinded_message_4;
  success &= absl::HexStringToBytes(
      "99d725c5613ff87d16464b0375b0976bf4d47319d6946e85f0d0c2ca79eb02a4c0c28264"
      "2e090a910b80fee288f0b3b6777e517b757fc6c96ea44ac570216c8fcd868e15da4b389b"
      "0c70898c5a2ed25c1d13451e4d407fe1301c231b4dc76826b1d4cc5e64b0e28fb9c71f92"
      "8ba48c87e308d851dd07fb5a7e0aa5d0dce61d1348afb4233355374e5898f63adbd5ba21"
      "5332d3329786fb7c30ef04c181b267562828d8cf1295f2ef4a05ef1e03ed8fee65efb772"
      "5d8c8ae476f61a35987e40efc481bcb4b89cb363addfb2adacf690aff5425107d29b2a75"
      "b4665d49f255c5caa856cdc0c5667de93dbf3f500db8fcce246a70a159526729d82c34df"
      "69c926a8",
      &blinded_message_4);
  std::string blinded_signature_4;
  success &= absl::HexStringToBytes(
      "a9678acee80b528a836e4784f0690fdddce147e5d4ac506e9ec51c11b16ee2fd5a32e382"
      "a3c3d276a681bb638b63040388d53894afab79249e159835cd6bd65849e5d1397666f03d"
      "1351aaec3eae8d3e7cba3135e7ec4e7b478ef84d79d81039693adc6b130b0771e3d6f087"
      "9723a20b7f72b476fe6fef6f21e00b9e3763a364ed918180f939c3510bb5f46b35c06a00"
      "e51f049ade9e47a8e1c3d5689bd5a43df20b73d70dcacfeed9fa23cabfbe750779997da6"
      "bc7269d08b2620acaa3daa0d9e9d4b87ef841ebcc06a4c0af13f1d13f0808f512c508985"
      "86b4fc76d2b32858a7ddf715a095b7989d8df50654e3e05120a83cec275709cf79571d8f"
      "46af2b8e",
      &blinded_signature_4);
  std::string signature_4;
  success &= absl::HexStringToBytes(
      "ba57951810dbea7652209eb73e3b8edafc56ca7061475a048751cbfb995aeb4ccda2e9eb"
      "309698b7c61012accc4c0414adeeb4b89cd29ba2b49b1cc661d5e7f30caee7a12ab36d6b"
      "52b5e4d487dbff98eb2d27d552ecd09ca022352c9480ae27e10c3a49a1fd4912699cc01f"
      "ba9dbbfd18d1adcec76ca4bc44100ea67b9f1e00748d80255a03371a7b8f2c160cf63249"
      "9cea48f99a6c2322978bd29107d0dffdd2e4934bb7dc81c90dd63ae744fd8e57bff5e83f"
      "98014ca502b6ace876b455d1e3673525ba01687dce998406e89100f55316147ad510e854"
      "a064d99835554de8949d3662708d5f1e43bca473c14a8b1729846c6092f18fc0e08520e9"
      "309a32de",
      &signature_4);
  // test_vector 4.
  test_vectors.push_back({
      std::move(n),
      std::move(e),
      std::move(d),
      std::move(p),
      std::move(q),
      /*message=*/"",
      /*public_metadata=*/"",
      std::move(message_mask_4),
      std::move(blinded_message_4),
      std::move(blinded_signature_4),
      std::move(signature_4),
  });
  if (!success) {
    return {};
  }
  return test_vectors;
}

std::vector<IetfRsaBlindSignatureWithPublicMetadataTestVector>
GetIetfPartiallyBlindRSASignatureNoPublicExponentTestVectors() {
  bool success = true;
  std::string n;
  success &= absl::HexStringToBytes(
      "d6930820f71fe517bf3259d14d40209b02a5c0d3d61991c731dd7da39f8d69821552e231"
      "8d6c9ad897e603887a476ea3162c1205da9ac96f02edf31df049bd55f142134c17d4382a"
      "0e78e275345f165fbe8e49cdca6cf5c726c599dd39e09e75e0f330a33121e73976e4facb"
      "a9cfa001c28b7c96f8134f9981db6750b43a41710f51da4240fe03106c12acb1e7bb53d7"
      "5ec7256da3fddd0718b89c365410fce61bc7c99b115fb4c3c318081fa7e1b65a37774e8e"
      "50c96e8ce2b2cc6b3b367982366a2bf9924c4bafdb3ff5e722258ab705c76d43e5f1f121"
      "b984814e98ea2b2b8725cd9bc905c0bc3d75c2a8db70a7153213c39ae371b2b5dc1dafcb"
      "19d6fae9",
      &n);
  std::string e;
  success &= absl::HexStringToBytes("010001", &e);
  std::string d;
  success &= absl::HexStringToBytes(
      "4e21356983722aa1adedb084a483401c1127b781aac89eab103e1cfc52215494981d18dd"
      "8028566d9d499469c25476358de23821c78a6ae43005e26b394e3051b5ca206aa9968d68"
      "cae23b5affd9cbb4cb16d64ac7754b3cdba241b72ad6ddfc000facdb0f0dd03abd4efcfe"
      "e1730748fcc47b7621182ef8af2eeb7c985349f62ce96ab373d2689baeaea0e28ea7d45f"
      "2d605451920ca4ea1f0c08b0f1f6711eaa4b7cca66d58a6b916f9985480f90aca9721068"
      "5ac7b12d2ec3e30a1c7b97b65a18d38a93189258aa346bf2bc572cd7e7359605c20221b8"
      "909d599ed9d38164c9c4abf396f897b9993c1e805e574d704649985b600fa0ced8e54270"
      "71d7049d",
      &d);
  std::string p;
  success &= absl::HexStringToBytes(
      "dcd90af1be463632c0d5ea555256a20605af3db667475e190e3af12a34a3324c46a30940"
      "62c59fb4b249e0ee6afba8bee14e0276d126c99f4784b23009bf6168ff628ac1486e5ae8"
      "e23ce4d362889de4df63109cbd90ef93db5ae64372bfe1c55f832766f21e94ea3322eb21"
      "82f10a891546536ba907ad74b8d72469bea396f3",
      &p);
  std::string q;
  success &= absl::HexStringToBytes(
      "f8ba5c89bd068f57234a3cf54a1c89d5b4cd0194f2633ca7c60b91a795a56fa8c8686c0e"
      "37b1c4498b851e3420d08bea29f71d195cfbd3671c6ddc49cf4c1db5b478231ea9d91377"
      "ffa98fe95685fca20ba4623212b2f2def4da5b281ed0100b651f6db32112e4017d831c0d"
      "a668768afa7141d45bbc279f1e0f8735d74395b3",
      &q);

  std::string message_1;
  success &= absl::HexStringToBytes("68656c6c6f20776f726c64", &message_1);
  std::string public_metadata_1;
  success &= absl::HexStringToBytes("6d65746164617461", &public_metadata_1);
  std::string blinded_message_1;
  success &= absl::HexStringToBytes(
      "cfd613e27b8eb15ee0b1df0e1bdda7809a61a29e9b6e9f3ec7c345353437638e85593a73"
      "09467e36396b0515686fe87330b312b6f89df26dc1cc88dd222186ca0bfd4ffa0fd16a97"
      "49175f3255425eb299e1807b76235befa57b28f50db02f5df76cf2f8bcb55c3e2d39d8c4"
      "b9a0439e71c5362f35f3db768a5865b864fdf979bc48d4a29ae9e7c2ea259dc557503e29"
      "38b9c3080974bd86ad8b0daaf1d103c31549dcf767798079f88833b579424ed5b3d70016"
      "2136459dc29733256f18ceb74ccf0bc542db8829ca5e0346ad3fe36654715a3686ceb69f"
      "73540efd20530a59062c13880827607c68d00993b47ad6ba017b95dfc52e567c4bf65135"
      "072b12a4",
      &blinded_message_1);
  std::string blinded_signature_1;
  success &= absl::HexStringToBytes(
      "ca7d4fd21085de92b514fbe423c5745680cace6ddfa864a9bd97d29f3454d5d475c6c1c7"
      "d45f5da2b7b6c3b3bc68978bb83929317da25f491fee86ef7e051e7195f3558679b18d6c"
      "d3788ac989a3960429ad0b7086945e8c4d38a1b3b52a3903381d9b1bf9f3d48f75d9bb7a"
      "808d37c7ecebfd2fea5e89df59d4014a1a149d5faecfe287a3e9557ef153299d49a4918a"
      "6dbdef3e086eeb264c0c3621bcd73367195ae9b14e67597eaa9e3796616e30e264dc8c86"
      "897ae8a6336ed2cd93416c589a058211688cf35edbd22d16e31c28ff4a5c20f1627d09a7"
      "1c71af372edc18d2d7a6e39df9365fe58a34605fa1d9dc53efd5a262de849fb083429e20"
      "586e210e",
      &blinded_signature_1);
  std::string signature_1;
  success &= absl::HexStringToBytes(
      "cdc6243cd9092a8db6175b346912f3cc55e0cf3e842b4582802358dddf6f61decc37b7a9"
      "ded0a108e0c857c12a8541985a6efad3d17f7f6cce3b5ee20016e5c36c7d552c8e8ff6b5"
      "f3f7b4ed60d62eaec7fc11e4077d7e67fc6618ee092e2005964b8cf394e3e409f331dca2"
      "0683f5a631b91cae0e5e2aa89eeef4504d24b45127abdb3a79f9c71d2f95e4d16c9db0e7"
      "571a7f524d2f64438dfb32001c00965ff7a7429ce7d26136a36ebe14644559d3cefc4778"
      "59dcd6908053907b325a34aaf654b376fade40df4016ecb3f5e1c89fe3ec500a04dfe5c8"
      "a56cad5b086047d2f963ca73848e74cf24bb8bf1720cc9de4c78c64449e8af3e7cddb0da"
      "b1821998",
      &signature_1);
  std::vector<IetfRsaBlindSignatureWithPublicMetadataTestVector> test_vectors;
  // test_vector 1.
  test_vectors.push_back(
      {n, e, d, p, q, std::move(message_1), std::move(public_metadata_1),
       /*message_mask=*/"", std::move(blinded_message_1),
       std::move(blinded_signature_1), std::move(signature_1)});

  std::string message_2;
  success &= absl::HexStringToBytes("68656c6c6f20776f726c64", &message_2);
  std::string blinded_message_2;
  success &= absl::HexStringToBytes(
      "5e6568cd0bf7ea71ad91e0a9708abb5e97661c41812eb994b672f10aa8983151113aeaab"
      "cf1306fa5a493e3dbdd58fc8bdb61aac934fae832676bcab7abacdcc1b9c1f2af3586ae0"
      "09042293b6945fee0aeffb2d2b8a24f82614b8be39bab71a535f6d65f1631e927dbd471b"
      "0753e7a63a201c7ecd26e7fbbb5e21e02f865b64e20731004c395b0e059a92fffa4c636a"
      "c4c00db9aa086b5dd1a3dd101bb04970b12ca3f4936f246e32d394f328cea2510554060e"
      "8d291acdbee04b8bc91e967241ba45f3509d63ded5f9b358f4216f37a885e563b7baa93a"
      "717ca7cdbe10e398d14bb2d5a1376b4a5f83226ce2c575087bc28d743caeff9c1b11cc8b"
      "d02f5f14",
      &blinded_message_2);
  std::string blinded_signature_2;
  success &= absl::HexStringToBytes(
      "72c4e0f4f677aa1dbb686e23b5944b3afdc7f824711a1f7486d1ed6fa20aad255a141288"
      "5aee04c64359964e694a713da2a1684325c1c31401cac1ea39a9e454675b55f743ff144a"
      "c605d0ed254b12d9bdd43b0e8a17c0d4711239732e45e4166261d0b16d2f29403c5f2584"
      "a29b225daa7530ba15fc9af15ed2ce8fcb126ad0b0758fd522fbf99a83e4cfe0539aa264"
      "d06a1633deee0053f45fc8a944f1468a0c0c449155139779a3230c8fa41a81858418151f"
      "a195f57ea645699f550d3cb37c549542d436071d1af74e629f938fa4717ca9def382fc35"
      "089e4caec9e5d740c38ecb2aa88c90176d2f322866acfd50e2b92313161e81327f889aca"
      "0c94bcb8",
      &blinded_signature_2);
  std::string signature_2;
  success &= absl::HexStringToBytes(
      "a7ace477c1f416a40e93ddf8a454f9c626b33c5a20067d81bdfef7b88bc15de2b0462447"
      "8b2134b4b23d91285d72ca4eb9c6c911cd7be2437f4e3b24426bce1a1cb52e2c8a4d13f7"
      "fd5c9b0f943b92b8bbcba805b847a0ea549dbc249f2e812bf03dd6b2588c8af22bf8b6bb"
      "a56ffd8d2872b2f0ebd42ac8bd8339e5e63806199deec3cf392c078f66e72d9be817787d"
      "4832c45c1f192465d87f6f6c333ce1e8c5641c7069280443d2227f6f28ff2045acdc368f"
      "2f94c38a3c909591a27c93e1778630aeeeb623805f37c575213091f096be14ffa739ee55"
      "b3f264450210a4b2e61a9b12141ca36dd45e3b81116fc286e469b707864b017634b8a409"
      "ae99c9f1",
      &signature_2);
  // test_vector 2.
  test_vectors.push_back({n, e, d, p, q, std::move(message_2),
                          /*public_metadata=*/"", /*message_mask=*/"",
                          std::move(blinded_message_2),
                          std::move(blinded_signature_2),
                          std::move(signature_2)});

  std::string public_metadata_3;
  success &= absl::HexStringToBytes("6d65746164617461", &public_metadata_3);
  std::string blinded_message_3;
  success &= absl::HexStringToBytes(
      "92d5456738e0cfe0fa770b51e6a72d633d7cec3a945459f1db96dbc500a5d1bca34a8390"
      "59579759301c098231b102fb1e114bf9f892f42f902a336f4a3585b23efa906dfcb94213"
      "f4d3b39951551cedecbf51efa213ad030cf821ee3fa46a57d67429f838ff728f47111f7f"
      "1b22000a979c0f56cc581396935780d76173410d2a8a5688cd59622903008fe50af1fcc5"
      "e7cf96affad7e60fbed67996c7a377effa0f08d9273cd33536b2625c9575d10636cc9646"
      "36a1500f4fcb22aabbef77fe415cbc7245c1032d34bd480ee338f55be0a79c0076d9cf9c"
      "94c0db3003a33b23c62dbd1a85f2b15db5d153b318cca53c6d68e1e63bafa39c9a43be72"
      "f36d2569",
      &blinded_message_3);
  std::string blinded_signature_3;
  success &= absl::HexStringToBytes(
      "a76a1c53566a9781de04d87e8c3a0bc902b47819e7b900580654215b0a710cb563b085b5"
      "e9fff150791f759da03a139dfc9159c21410f1e3d345b8c5dcca35211772900f85c5eec0"
      "65987cbdbf303e9651196223263a713e4135d6b20bfa8fb8212341665647a9a7e07a831c"
      "cbf9e62d9366ec9ac0bbe96228e6fbb848f8f6f474cce68e3556dc882847e9e61b5b5e02"
      "bbfd6152aeca74e8782a54ffe6552d63fb837738a05044b38f7e908c4989b202bd858695"
      "c61e12cf9d47ef276a17917e39f942871defd9747541957b1e2f8950da43c9a05ba4835b"
      "ded23c24cf64edfee10dd0c70b071427cfcbb8b5eb225daf149a6b4d42bebcc536380a9d"
      "753a8b1e",
      &blinded_signature_3);
  std::string signature_3;
  success &= absl::HexStringToBytes(
      "02bc0f2728e2b8cd1c1b9873d4b7f5a62017430398165a6f8964842eaa19c1de292207b7"
      "4dc25ee0aa90493216d3fbf8e1b2947fd64335277b34767f987c482c69262967c8a8aaf1"
      "80a4006f456c804cdc7b92d956a351ad89703cc76f69ed45f24d68e1ae0361479e0f6faf"
      "10c3b1582de2dcd2af432d57c0c89c8efb1cf3ac5f991fe9c4f0ad24473939b053674a25"
      "82518b4bd57da109f4f37bc91a2f806e82bb2b80d486d0694e663992c9517c946607b978"
      "f557bbb769d4cd836d693c77da480cd89b916e5e4190f317711d9c7e64528a314a14bf0b"
      "9256f4c60e9ddb550583c21755ab882bdfdf22dc840249389b1e0a2189f58e19b41c5f31"
      "3cddce29",
      &signature_3);
  // test_vector 3.
  test_vectors.push_back({n, e, d, p, q,
                          /*message=*/"", std::move(public_metadata_3),
                          /*message_mask=*/"", std::move(blinded_message_3),
                          std::move(blinded_signature_3),
                          std::move(signature_3)});

  std::string blinded_message_4;
  success &= absl::HexStringToBytes(
      "ba562cba0e69070dc50384456391defa410d36fa853fd235902ff5d015d688a44def6b6a"
      "7e71a69bff8ee510f5a9aa44e9afddd3e766f2423b3fc783fd1a9ab618586110987c1b3d"
      "dce62d25cae500aa92a6b886cb609829d06e67fbf28fbbf3ee7d5cc125481dd002b90809"
      "7732e0df06f288cc6eb54565f8153d480085b56ab6cb5801b482d12f50558eb3cb0eb7a4"
      "ff8fcc54d4d7fcc2f8913a401ae1d1303ead7964f2746e4804e2848bba87f53cf1412afe"
      "dc82d9c383dd095e0eb6f90cc74bc4bb5ea7529ded9cde2d489575d549b884379abe6d7b"
      "71969e6a9c09f1963d2719eefccd5f2a407845961ccc1fa580a93c72902b2499d96f89e6"
      "c53fc888",
      &blinded_message_4);
  std::string blinded_signature_4;
  success &= absl::HexStringToBytes(
      "280c5934022fd17f7f810d4f7adf1d29ced47d098834411d672163cc793bcaad239d07c4"
      "c45048a682995950ce84703064cd8c16d6f2579f7a65b66c274faccc6c73c9d299dcf35c"
      "96338c9b81af2f93554a78528551e04be931c8502ee6a21ef65d1fa3cd049a993e261f85"
      "c841b75857d6bf02dd4532e14702f8f5e1261f7543535cdf9379243b5b8ca5cd69d25762"
      "76a6c25b78ab7c69d2b0c568eb57cf1731983016dece5b59e75301ca1a148154f2592c84"
      "06fee83a434f7b3192649c5be06000866ff40bf09b558c7af4bbb9a79d5d13151e7b6e60"
      "2e30c4ab70bbbce9c098c386e51b98aefab67b8efc03f048210a785fd538ee6b75ecd484"
      "c1340d91",
      &blinded_signature_4);
  std::string signature_4;
  success &= absl::HexStringToBytes(
      "b7d45ec4db11f9b74a6b33806e486f7ee5f87c4fa7c57d08caf0ca6d3ba55e66bf0769c8"
      "4b9187b9a86e49ba0cb58348f01156ac5bc2e9570fe0a8c33d0ad049a965aeb2a8d8a3cb"
      "b30f89a3da6732a9bb3d9415141be4e9052f49d422301a9cfce49947db7d52a1c620b710"
      "6ae43afbcb7cb29b9c215e0c2b4cf8d62db67224dd3de9f448f7b6607977c608595d2938"
      "0b591a2bff2dff57ea2c77e9cdf69c1821ff183a7626d45bbe1197767ac577715473d185"
      "71790b1cf59ee35e64362c826246ae83923d749117b7ec1b4478ee15f990dc200745a45f"
      "175d23c8a13d2dbe58b1f9d10db71917708b19eeeab230fe6026c249342216ee785d9422"
      "c3a8dc89",
      &signature_4);
  // test_vector 4.
  test_vectors.push_back(
      {std::move(n), std::move(e), std::move(d), std::move(p), std::move(q),
       /*message=*/"", /*public_metadata=*/"",
       /*message_mask=*/"", std::move(blinded_message_4),
       std::move(blinded_signature_4), std::move(signature_4)});
  if (!success) {
    return {};
  }
  return test_vectors;
}

std::string RandomString(int n, std::uniform_int_distribution<int>* distr_u8,
                         std::mt19937_64* generator) {
  std::string rand(n, 0);
  for (int i = 0; i < n; ++i) {
    rand[i] = static_cast<uint8_t>((*distr_u8)(*generator));
  }
  return rand;
}

std::pair<anonymous_tokens::TestRsaPublicKey,
          anonymous_tokens::TestRsaPrivateKey>
GetStrongTestRsaKeyPair2048() {
  bool success = true;
  std::string n;
  success &= absl::HexStringToBytes(
      "b31928fd04c205d364cab9f7a5620dd8db9992dfaa41c1d29b11df91204ddc0d28a5869c"
      "fc4c8ee2fca229c487b0f529c7d782303d4f5b9d85019031b159e4a7ad7d172ccd73915f"
      "10550a7f19d63bfe438d6801a226dedc054bee2958c599cfd8513ed26ae29a5521f6ab7a"
      "e4991404b6888d60a76eadec189492a988e4c941d3ffd8feb7bdf715ec0ceaf53707d83e"
      "3cc743ec3b7d88d5dc46b615a63d4fee9a0a391546069b811e29095d5a1319fbb70248c3"
      "5711a46d3c16f1444be285aeddb33256ca775562e755ac9449bfec12cdd099c8dac96b34"
      "69764c474a88bc7e1dd19db68e9275606a81428616554a918a951bde14ee093dbdbdbbd0"
      "892486f9",
      &n);
  std::string e;
  success &= absl::HexStringToBytes("010001", &e);

  anonymous_tokens::TestRsaPublicKey public_key = {std::move(n), std::move(e)};
  std::string d;
  success &= absl::HexStringToBytes(
      "1bcda61d5165c57dc1c1ef08d0f5ddec727aeee026103b44b4aa1ba8edf8e8566a9ef7bc"
      "db360f609193a3244d645d4af529319ec785d0552dd6c649d09c81f0bdf0136ef31e23cd"
      "3c3dd7794fcb8058c2a7eb2385c6bf062d14528ebca7406f91c75b17535c8654fd06cc2c"
      "31dcc9ccc9817d6129dcf6c71631ca6ae3439132921a9c18111b4b11b421868feac7c9ed"
      "6c73c437a24dbc5b364790cf4e7ac1573e72bab1b1e456b55e2ea0a673986f2305c50122"
      "ba924db6d281a5e3efc6c03d0fdc690d4d8e4fb5f45a1c4ce5c4595fde5563e8be01170e"
      "6e7ef6396bd8d435a14028748d4ef182fbffcc4aa1b99f86a6155cd26da9bb218a1e3b2c"
      "dce38e19",
      &d);
  std::string p;
  success &= absl::HexStringToBytes(
      "edec7eb7cca858e3fc1c0f4eef5f4574216c96614d3bdee1830930a0036f85f277eb6ff3"
      "3a757fcf6323325b1967eae0f802dffd2a79c2c222f17c6378bc8d08e3d6ba975e13c62e"
      "5b93e2bb561fb1587dfeb14b20cf5cce9f4518b8eb052c8e48c0b891dd94fa2fef904d45"
      "ffe00f7a1a8e77c3c34e337612eba4b40a16078f",
      &p);
  std::string q;
  success &= absl::HexStringToBytes(
      "c0b4895d14c4e4aca5eee0bf0e58b0da5a210a2793ca06ba8f6b8a6b70202cabc545c220"
      "922f02ca849f4ee79313e3fbdfdbdb85367b307f8fe663e108d3bdac4399836e225f1956"
      "c3d112167f24db0e429a71d2ad191465f3b99cd3370bfd7b3e8d1a5e5e788dcfab21ddb0"
      "0f1aaa73d7cb62f0228449a51d032c9f636b04f7",
      &q);
  std::string dp;
  success &= absl::HexStringToBytes(
      "2707b7d5f105e0e72d9170d573213ee48923261c3a2e4b26d5772979e6766213dfa648cc"
      "2ed7ddaaa8c9ba560579eda710287094386697137fe5fb90d9da9c8c4bcc0afa0fddd092"
      "0445e358f60ce6ebec675eb04366a103e84ece7a6f5b7eeeac72a9148cb406c2dc5ae0c2"
      "4df274b78429c0ede5592bc9ffda963f4eb44473",
      &dp);
  std::string dq;
  success &= absl::HexStringToBytes(
      "22c0ac41201cbe0cb0c41abdf9ed5ebf921f81404ff3d030d6ea9304fb2ca241bc0aef8e"
      "862e7a3761a1854e5804ef499e3e7d215208f75f19e977bbbea6c8ff0715e950f45be82a"
      "f09784c68fd96ab3f0a8ffbbf9c19b1f23cc268f24cf41c07730653ffd938a27987a3c0b"
      "a33db0ddc15e0992baf6d67d33753e17e48b0953",
      &dq);
  std::string crt;
  success &= absl::HexStringToBytes(
      "29402f48481599b7e44c7ab9f0cfc673266dfd9ff0e2d7f9f40b6b1d8061808eb631935f"
      "d5c1e536dd99266d79c04842b121adf361e8c7a8bc04fdb7c5ad053a8b9117cf2068142e"
      "117bdda6d2a5a01ff8f0ba28d42287612c35e5ff267a20b5da454205cdf6d24d22d49685"
      "11c16b0f1a1e55865d0b5ace0beaae0ba3bbd68e",
      &crt);
  anonymous_tokens::TestRsaPrivateKey private_key = {
      public_key.n, public_key.e,  std::move(d),  std::move(p),
      std::move(q), std::move(dp), std::move(dq), std::move(crt)};
  if (!success) {
    return {};
  }
  return std::make_pair(public_key, private_key);
}

std::pair<anonymous_tokens::TestRsaPublicKey,
          anonymous_tokens::TestRsaPrivateKey>
GetAnotherStrongTestRsaKeyPair2048() {
  bool success = true;
  std::string n;
  success &= absl::HexStringToBytes(
      "cfe2049a15de49dd75e828eb8f5321b44f3d4169f53f9b58b37f1aba52f87ea749b83028"
      "4857eab7f0ea3bac6b866e5f485be31cea03a7ff2c0ba7cfdbe6f070fc49e37e28f2afe9"
      "0b61e12a877febb1d4ba6fc0932df332afe51e8fa702a762b944a3f80a5fea2612cc75c5"
      "9400e00df62ba4be83cc50198c39b6ac4bc6a5b4f6edaf0bdeac025d6dd03d9f0f7c2127"
      "cf3c046a7e4e7cc7bc33f246f52408df49b29696d994e190076a78142cd353c4d5fe38d9"
      "708466e49131efa416d35218cde5c380d548599b8ce39a9efcfe04df6aa317e122ac9813"
      "46dbde6af84544d8f04e1c19749e6a8fb1efff4b3c3dc3d7d2c95eefc1acd2dd80b5ff58"
      "5eabfb09",
      &n);
  std::string e;
  success &= absl::HexStringToBytes("010001", &e);
  anonymous_tokens::TestRsaPublicKey public_key = {std::move(n), std::move(e)};

  std::string d;
  success &= absl::HexStringToBytes(
      "1176a1bf55fdf603922f9e1c67ce6a82ecb32f271910ae5aadbd8c3fc1cf99483163b53b"
      "f513d9a679291c393851333d72e53137911b1c864dab6efe01b1ad5a387f768a7723280e"
      "f24357388ce87ca2d4459334c0c877e936a88f402f1e0474c12e987db2556b64a668a1ae"
      "26e849ea325769400def607d3cefee3e1c218472ffea639163dd7e802b20e35b3d0cd7c1"
      "1229cde6ad4d73cb097c1b348f5586585d2727ff62789385665d11b16eceffd85582b58a"
      "858ca356d7011bb5e4777bf3b67fef77cc528c56a147d6d7229398bb7bb057139a9b9e7d"
      "33e5ac6f302c538b4c81901ef28adb6c530cd549d61ec78e9402fb0deaab176027fda9b0"
      "801403e7",
      &d);
  std::string p;
  success &= absl::HexStringToBytes(
      "fda22fbc727c67fa8b5c72c54bf5136a564de2f46697f1953f751da1cc5bde428f5a5f70"
      "07c775a14ab25d1b6996b374bfc1df6665b8e9d2914754ad1a3cebd8bf6da17e9ea0a98d"
      "289e609681fd295500d0803522696662a1564eb6d4f1422db8d8da48826df937cd19176e"
      "41889481d1309086aee3968c2692dd893f59288b",
      &p);
  std::string q;
  success &= absl::HexStringToBytes(
      "d1d28de5df823cea723f6979d73d44d86c202328cd4914abffd7b2e11245c075d4e501dc"
      "a7b90249bdb273fe6f78dbc4fdf0229dcb333b9fc24ec6ffd02fcda1a8fa788e3b49f037"
      "6be5ce222ccdf92e17e651a5a53507d9687f62835b08825f53f7e3d760e98e83533e7172"
      "1b10cd8832dc1c471875655d66cb19e58bb0493b",
      &q);
  std::string dp;
  success &= absl::HexStringToBytes(
      "8d8e547827a9795ae326e0c36ec64464c7f04667c34eb03d6d224f3c7b5316b42d4ff20e"
      "13b965d4745d220be79d7d60fe9914b710b4e8836623da85962c44313f7dcf715cd52c6c"
      "252c6799f8c8b3a5c68397da8fef257e8caf1fd578f981c704f0babb57584b8cb2427bca"
      "447716f3712e5aab60b692d27bc0e235f48e2d4b",
      &dp);
  std::string dq;
  success &= absl::HexStringToBytes(
      "72c12850379ca03a4cffb76d26b5e0a849028e982b26340319eadb6f533208dfa8ef12c4"
      "9e8a85e0d4b9fbcc8524e1756cb8e005d2f393417de0dddf5cfa380999445b98d67e4abd"
      "d4ea1b81ff652b49f55247074442aba7510a92536aff4d665ba330de43a79904e40b3bba"
      "7f69022fe23915d220635c6be7e35ea7776d93af",
      &dq);
  std::string crt;
  success &= absl::HexStringToBytes(
      "6b7f1d159c6be9a9c4d6d4171f6e90b3c9d40abee51b891f538a653c06da423ece647713"
      "a6192babbdc8580cfa941f4cc88952f982fe197fd2fcd29d0b6b01960361419a74182cc9"
      "4acaac94ad88b000677bba8f97f4ba362019a0fe1ffeb64691ca17039ebd6ad5fec82690"
      "90d2163b54ca25f4840f46f0395fdfec83cac4eb",
      &crt);
  anonymous_tokens::TestRsaPrivateKey private_key = {
      public_key.n, public_key.e,  std::move(d),  std::move(p),
      std::move(q), std::move(dp), std::move(dq), std::move(crt)};
  if (!success) {
    return {};
  }
  return std::make_pair(public_key, private_key);
}

std::pair<anonymous_tokens::TestRsaPublicKey,
          anonymous_tokens::TestRsaPrivateKey>
GetStrongTestRsaKeyPair3072() {
  bool success = true;
  std::string n;
  success &= absl::HexStringToBytes(
      "bd8be57544c2b43220d80b377fa22d69226e968b9f04e321e7c9e82ec4a4849386d2c437"
      "7cf2b8ec93145fbebb6f4508266169e4a83b37671f28285fe91c75a4b721804e71a7eaea"
      "97d42cd3055e4e46e78ed10898472f92c61d981d1df20d55f89e0558eb95a13f5f8ae04a"
      "a2cbfbf99c4599702b1498ab337fe36396a39a073c5d5dbedf557e6d245f807c28a4c2f4"
      "4197ae256190d9a410392ede4fdf9d337fc201bb26447fabc442b19c79c531e12922a90b"
      "ada53615b12e9a54ecb033f9a22be859984e296d632c9eb287825bb4bfb7f3d16c4f2ba3"
      "0b2ca5a04512e62c993351c7039a64d865ba49eb960b176dbe7c4853db37911f7bae7827"
      "32441e428992422754ca3d78a20e9cedbafa8ec2460403997c381772be64b72133c1585b"
      "0d1fe5e96a3f7e2388228826989766da37f9949d1040230cb78f88005e5e92796a285b3a"
      "cdd90733ed4a111d35f4632eda15dc669e595380331acab1e98cf872126dac05c2d7a7be"
      "ff889ff39ea60cf7ac69f62bd35e6c2ff193c9037d0f500d",
      &n);
  std::string e;
  success &= absl::HexStringToBytes("010001", &e);
  anonymous_tokens::TestRsaPublicKey public_key = {std::move(n), std::move(e)};

  std::string d;
  success &= absl::HexStringToBytes(
      "2de57b093b3e1e1de94006ef48537fc56e55f2d41a0c37e754d5da07c10bc92263ca1343"
      "10594197df4156b1bb7704f3253fff4123cf3aea186c43e27d72abb5d7b61ff85ea2f74a"
      "18bb82a31230b4a98c96535d4e6a2645d6fd0181436801fca837b339c5c9b482c0e2c2ce"
      "afbecee3b108555008ce72ed398a25084f488c1a666e812d9fac76f17c96376958fa144e"
      "cab72caed68219811580932db78f80e420725cb2f16032bde7c6f274de3376917bc16dc7"
      "6b238f060fa226329c214a642417795cc3efa5337b1b89d6b14ac31e681c2e2a8962c086"
      "feaf590eb54769d05d5eaa2b96113ab27fd8ecca8e5ac717604af7c9e2572f05859d22b5"
      "658ba76206ca3f5a8c780bc664f5448927348427ac08e5713ebe160d2a4968093fad4015"
      "47669487775baf5c5605cff96e8170e5cde4eab215ee05d3a8a3416426573f2026157aae"
      "a1b8626102e969cb7fdfa67d4585d4970dd708308a6bd7f1cad1bc916ae3e8be82f2a944"
      "4a43cd171ad636f62b5c5b76d9709c39ae36f03ec6bbceed",
      &d);
  std::string p;
  success &= absl::HexStringToBytes(
      "e9ca59fe1ddb5c5050192692145220e04623867aff99f70a0224c11144c167dc79f21df6"
      "1b64c378c82940b78dd5608ff07a00bb83261e6f328ddea1f53a40a7b9a6bc9702e05afd"
      "1717456416f26b199cdb704d0d5b555deaf4d1d6e738b86db8096fc57c4d3c8cd3b510a6"
      "d5fa90c05135aec2dc161fd9e38771b7f4d26ff0e8a1d0ec0dd4d832128df1adbdf33125"
      "f723717efe947c65539ddeadc95e8960b79f0c77ec8761c38bced50a76f145176c0b5dac"
      "e6b7e3aa0b2ba16646357ec3",
      &p);
  std::string q;
  success &= absl::HexStringToBytes(
      "cf8d8e9c9102b69b76e28aa06431c177320e8ae147db85432507d51a68c587ac548197cc"
      "73666ae65ba4de3c5a974a4344f1f804969431537ff31e3f23f3cc50f90d69b4f994b410"
      "40aef3072b2cf2679094860924a6404b7196386463a074a6fd1b0b4bfcbcab82f81549f4"
      "4a65ff33a6ce5788fc1a7710759ca59c2040c21f1c97d66ee0f110c4f37da1c07508b0e6"
      "0ea1878ea6133ddf8ba4b29fc1761e5b43b7830ab87768058eec47c22a3ff8bbde4f6b10"
      "849b78daa6a072c30f7aa8ef",
      &q);
  std::string dp;
  success &= absl::HexStringToBytes(
      "65203e25094d257527f0791a9ee79788eb4dda91c1961ba19ca3c14f72ea25bedc90ba1d"
      "066463990f1ba8febcbf1b71a7975e51bdbcf3552e0ce7cc2e82f00c9ce55e96038c804f"
      "1179e36e13eef01cb818c34ed1043cbccf30eec38268aa7deb2949cba6a4d218284b1dd4"
      "cca20192ee8dc5f64bb4d63a2d8d1cc77182c520f3bf6adb70702cc41bfa821ba11a5c9c"
      "0b76ad553d51852d5f29de7455b22ac2472ae8fdc6b618b7b8f5d2792051e48ce9135185"
      "c496ae4793655fff19477279",
      &dp);
  std::string dq;
  success &= absl::HexStringToBytes(
      "1d4340102301d6ed245ddc5db0c2b31c331a89ca296f71e27d9e15159c1ffd78f6912eed"
      "cc776c2afe50c8648a013a9f31614c2e996c5b68026a2ca18a581d3e6d5ecec08d4fc1f3"
      "68ab41e888d5d5777492fc32ddcff2d0b03b15c851a395ced570b2af0bfb2dd35156ef0e"
      "5a4ef72439286e7f09cc516d28a7e55195da8b84076c00f7b10f4be5f8ce85b7b4c87ce8"
      "72b7a37d213d25441754293b0cf3b263fbb02bf19f0076d211cc8e7179b37b464199c0e6"
      "9b4bb04663a7cb8664f04e51",
      &dq);
  std::string crt;
  success &= absl::HexStringToBytes(
      "b5b84f7c4868e4de55d37efe7ada9865b0cc73b4b08e111cb8502b39210b17d81a542ea7"
      "93b970d03557c30b5243e066c7ff46e3abfcf3972a9a6199927d05f64fefb7efbb336d71"
      "6599e7cf507e87f274541ef5216235fdedfba524879fecedf4455a60071af52d36a0df37"
      "b3f4c64b75e564fbdaadc36356e2382efd783ab4e82f4f708fb1addd288658dfd4afc14c"
      "427e2699d8ed178fb343ebef2afd343d0f3aeb30a96dcac9f6a136d54347a42e318daf23"
      "d1d57b1cd964bd665a3f2a67",
      &crt);
  anonymous_tokens::TestRsaPrivateKey private_key = {
      public_key.n, public_key.e,  std::move(d),  std::move(p),
      std::move(q), std::move(dp), std::move(dq), std::move(crt)};
  if (!success) {
    return {};
  }
  return std::make_pair(public_key, private_key);
}

std::pair<anonymous_tokens::TestRsaPublicKey,
          anonymous_tokens::TestRsaPrivateKey>
GetStrongTestRsaKeyPair4096() {
  bool success = true;
  std::string n;
  success &= absl::HexStringToBytes(
      "cd7d928f252a882c2ba68c1705970f61b7f63c5e907ea5f34e650e3c35edd74678734d62"
      "6fca38a1230c52147cb8b16e2db9adbfe7ce4647ef2eb49b4ade458c80ef0e29ac410923"
      "3d0f512643106fb2e42308fbc2db13c1db24c672a3bfc32acfb429ae5104507f2b342473"
      "a9aa5eab8a9c24d7fe08fb59bea4049d14fea781484591460e5eef62bd67d3c28aa8e360"
      "c50b936998565ca12fbc647d32c446f3f326fe0a36388bfb3ed7a4c1e8c900a299c88bda"
      "f6dc9ebb032f810f682ddfc2d5fa46e8fa28b8bdfa32131f259615f85bde8a4eb8258ccb"
      "da83e62cf12795c0cae1498c2b435e27c31b9ef8a1efbf9552bc6f929a76d9d3a997bfe6"
      "fe11c155a571446decdb5032b80482d0bcb8ab0a23ab82451049a1af692764b691876200"
      "05a9d3b5d530d38bfc41938066f505a6e2484795ce70a69e5df5a551b5179ff1ed3a34ec"
      "eb09834317de137d9c2d6b35c745c67b05a1412fc0f616581a051f41bf14c48dcc8b558f"
      "92cdee22f5d0f4a75c232e4acf45d3d2491a2eda3d7ed40fcef81058b8b3b019ef749245"
      "3dd3220d5a1ee706abcf4da44a572376eee594dca796f8be05ba104ea08881e68c091326"
      "22f233574bd0c3f9dfaa9ae7c6579b90312851aeec02b2678c5e530cc8fbc30e389799df"
      "92a2898c3420836763e199488adc8e5464ff4a67debf35ac2011d4723c3cf1ea1326ce55"
      "5f80611b20944a31",
      &n);
  std::string e;
  success &= absl::HexStringToBytes("010001", &e);
  anonymous_tokens::TestRsaPublicKey public_key = {std::move(n), std::move(e)};

  std::string d;
  success &= absl::HexStringToBytes(
      "1d618f83851a64370094c322058c18486e0fb88902db00ea5d72a88ae66117ef3d08ab6f"
      "603187504edd139d5749e720ac4c08b2503817a77064fab0db8f155da60fc834202b7a5d"
      "7dfd032ad7daf145a045fc22573590c91e86cf131423b689980218159302ed6989695eae"
      "e4faf5a74c5dd00ccc0747bd08bb95e749d9b164944b521eb4ae51470a72de7dc9eaa4fc"
      "30a05b96f50fa015f1e7db6c65465828c842f27ece4ade84f172cedd64e5dc7fe3421ff1"
      "126bf00c2843f20d9c6536c1ba6b9b18f3afbfde75f813f0d7a47286bcc8007989ede088"
      "4339a9bf124a0928f4392b156e18274dc3215f65086e69b3b58d38dcbad6348605912b80"
      "a12233c4c418ab6cedeb313207c2567e0754a9f0b4ac5365cfbc699ccc3a967a668e9ee9"
      "c272c4dfac1a7024bd98ccb7e6de98fe5a3a43fcb01e0d354ca7b31c266253a35f7ee110"
      "9c59f2523bd03fa6d8c6f03c5b347fc597c3d0011a0d984105b74a2a406a7ab815657da8"
      "8c8ee56d78925409df32f8698a75af8fb2b3576b5676c1ffc8026421b73e72698b3d1069"
      "5f369874fa681df1b4f1e78155ff7238b23a1f1b73541fd4a60831a5d78c6a8b2b86d9a5"
      "d24f36c9437f5b8e5e522d078c9f23c6bbd24e0b261b575b4d31b3d05434afb3b45442f9"
      "81d33954d0b433808aa0cacaba9530f3f6083dd059a0ad36ade853997c575a0036a69185"
      "1f34c391be7e6f43",
      &d);
  std::string p;
  success &= absl::HexStringToBytes(
      "e6503c05c40a5db99f52ae1ae7ae3a313802821e2d93a431f71c21206e7cf683603de565"
      "b0788038841f761025f4f50b090a2a828240460d5eba1fc49cec36d93cb7ee2abda6dade"
      "da381b83c3e6f18c1ddea7651a7fe87ee65ce089817baa7998c6db994132850d6b47f9af"
      "bf6c6fbf7d813173d2d2f904892288dc603f4b11c96d67228b0591f49311f227f81cad39"
      "161039028b009155a703ea581d3f10b4b668e59d07f0ca90bc26970b854ac17abdd86789"
      "ee0d61db5942226f498099076ce05aaa72a52cf6006216a8f7d1afbd64e9449b068c65fa"
      "eec6cdb3b02a2d0f9320d85d963067c38093ad6a3483a3db7e5964ba29634540de9ed60b"
      "8e1423ab",
      &p);
  std::string q;
  success &= absl::HexStringToBytes(
      "e4689c2d46a1e63dc955942bc34a948b50cc1047cd61b67aec389f7315aac62d9d249715"
      "25a1d925a93d4da005280298587b3559aba6c2329c63baaa37ab7fabb88c349ad34f7cfd"
      "3a57d5c4dc2c9a623fdb5724af0e808a00ec3a02d503b02905fa8dbb97d47d588dd9dab4"
      "6cc03709f54fff79d0c5941372faa9f9b6ff7524b4cb1740b6af34ced5c39b47ce490238"
      "7dffffdb7ab6c38a54e55d42b47359cef31e1d993abdaf15fab917a15db3a558660ad5fe"
      "3bcd298c2625481bc61b3aecfc960c6c7d732c560fcd99cf1d6d56da6c0ed876b2b957d0"
      "c2d7e86a1cd57a08380f526f18e4d3ca9000271cbf8e87f66e4f908834df312c6a6d62b9"
      "137c6d93",
      &q);
  std::string dp;
  success &= absl::HexStringToBytes(
      "8d017a7e1d342b85c5e19ceea401ab670edf9a4257ad18cdee78ae5f68c5e13735e92f55"
      "3ee1c7bed24560c72a35fb00b29c22c29c74356f621b99ef8a13a4d103b7a87d4a77a970"
      "df3192c6ed5dab6d19ac83d8068d610eb08314859b5cd756730eeccbbb7aeeb2f487b07a"
      "c53be27ede9c0666df20838d1f58a16a2b131526e2a7b489158c677bd1bf1eff118c9d11"
      "624cb45ab637b6c335e9d3c3f6c3f1ba72236ed0e157aeed46046a5d8751e97af85851ab"
      "c4af34c652b386d993aac40623c6883beaccede5fefe0ed98c4038d43fc0015cd87984c6"
      "4902365658f8b975dba23455b7ea12dd430f2710eaeddd9838970a705f7e839bdfb06763"
      "d3acc8d9",
      &dp);
  std::string dq;
  success &= absl::HexStringToBytes(
      "469418a658ec103449715b4ec692d621d27eac0d33e69cb79124d20882ca796080ed5c8e"
      "1949d0cab5680f0382746190e7ce72a6d9c6b6bd62dbe24354de769dfe71bc9396f639fe"
      "19b828832331d926c0eaab1bd7c8186a0c6cf2640ba48f1bae104519918a048d878fa8e8"
      "15aeb3932d2d6219272cd65bc82cb2b74a17d7ffd6a9e6ee8544d0819546534635f5136d"
      "9769b28b04795324fca4bf53ac64f47c615d8df1da57e0b15eff30d1191e38da7ef59c38"
      "6a0c34696d241a0b130539091fe7d1c0f866cd6d6e86ae9f744d64082c59ce03a7a863fd"
      "4b27e2565fc08b6bdcbec74f33170a66ce666daf91759e87c4806b7ddb3098864c00aeff"
      "d7889c67",
      &dq);
  std::string crt;
  success &= absl::HexStringToBytes(
      "a4e8c9443c2619b6c92c9dd9941422274431e80503dc8a143ce8d97cde3e331fca29e1de"
      "60ea50f7520d19192e39d0e106b37e20cc3a084afab1ab09c3205e1d7e59050ab76101ea"
      "7bf014dcccc7f948ff5fb14ddd83ee804de5c659672142b4b7e661e0be8e95eddee3b815"
      "f1f26741639fd04e5015153375ee1dfaa87ebf5b4340948538d3bfa1b4cdc7e81b68c7c0"
      "c85879bd5026ea66735e4c3b56294f6c63ac1ba0709edeefc252c90723039f1fe227086a"
      "2b57299d7f7bcd1f09b82985c7710bb43d342167142629a23094981f3908d0a1be38a5e3"
      "f823fad1ef96aa643fb5811cbafe8b134725075d4b664409de70b2571ea6ef53a44615db"
      "16b7bda5",
      &crt);
  anonymous_tokens::TestRsaPrivateKey private_key = {
      public_key.n, public_key.e,  std::move(d),  std::move(p),
      std::move(q), std::move(dp), std::move(dq), std::move(crt)};
  if (!success) {
    return {};
  }
  return std::make_pair(public_key, private_key);
}

}  // namespace anonymous_tokens
